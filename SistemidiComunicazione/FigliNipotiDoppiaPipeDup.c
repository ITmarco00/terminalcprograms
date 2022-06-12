#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>

typedef int pipe_t[2];

int main (int argc, char **argv) {
	
	int status, pidNipote, ritorno;
	int pid;		//process identifier per le fork() 
	int j,k;		//indice dei cicli
	pipe_t *piped;
	pipe_t p;		//una sola pipe per comunicazione fra nipote e figlio
	/* controllo sul numero di parametri */
	if(argc <= 2) {
		puts("Errore: numero incorretto di parametri");
		exit(1);
	}
	
	int N = argc - 1;	//numero di file passati
	
	/* Allocazione dell'array di N pipe descriptors */
	piped = (pipe_t *) malloc(N * sizeof(pipe_t));
	if(piped == NULL) {
		puts("Errore nella allocazione della memoria");
		exit(2);
	}
	
	/* Creazione delle N pipe figli-padre */
	for(j = 0; j < N; j++) {
		if(pipe(piped[j]) < 0) {
			printf("Errore nella creazione della pipe\n");
			exit(3);
		}
	}
	
	printf("Sono il processo padre pid %d e sto per generare %d figli\n", getpid(), N);
	
	/* generazione N processi figli */
	for(j=0; j<N; j++) {
		if ((pid = fork()) < 0) {
			puts("Errore nella fork");
			exit(4);
		}
		
		if(pid == 0) {
			//codice dei figli
			printf("Sono il processo figlio di indice %d e pid %d e sto per creare il nipote che recuperera' la lunghezza del file %s\n", j, getpid(), argv[j+1]);
			
			/* Chiusura delle pipe non usate nella comunicazione con il padre */
			for(k = 0; k < N; k++) {
				close(piped[k][0]); //chiudo lettura
				if (k != j) close(piped[k][1]);
			}
			
			/* creiamo la pipe di comunicazione fra nipote e figlio */
			if(pipe(p) < 0) {
				printf("Errore nella creazione della pipe fra figlio e nipote\n");
				exit(-2);
			}
			
			//creo nipote
			if((pid = fork()) < 0) {
				puts("Errore nella fork di creazione del nipote");
				exit(-3);
			}
			if(pid == 0) {
				//codice del nipote
				printf("Sono il processo nipote del figlio di indice %d e pid %d e sto per recuperare la lunghezza in linee del file %s\n", j, getpid(), argv[j+1]);
				
				//chiusura pipe comunicazione fra figlio e padre rimasta aperta
				close(piped[j][1]);
				
				close(0);
				//apro il file
				if(open(argv[j+1], O_RDONLY) < 0) {
					printf("Errore nella open del file %s\n", argv[j+1]);
					exit(-4);
				}
				
				//chiudo lo standard output e uso dup sul lato di scrittura della pipe
				close(1);
				dup(p[1]);
				//chiudo entrambi i lati della pipe
				close(p[0]);
				close(p[1]);
				
				/*Ridirezione standard error su /dev/null
				close(2);
				open("/dev/null", O_WRONLY);*/
				
				//eseguo il comando wc
				execlp("wc", "wc", "-l", (char *)0);
				
				//non si dovrebbe mai arrivare qui!
				exit(-1);
			}
			else {
				//codice del figlio
				
				/*ogni figlio deve chiudere il lato che non usa della pipe di comunicazione con il nipote*/
				close(p[1]);
				
				pidNipote=wait(&status);
				if (pidNipote < 0) {
					printf("Errore nella wait\n");
					exit(-1);
				}
				if((status & 0xFF) != 0) {
					printf("Nipote con pid %d terminato in modo anomalo\n", pidNipote);
					exit(-1);
				}
				
				char c;
				char str[5];
				int cont=0;
				while(read(p[0], &c, 1) > 0) {
					if(isdigit(c)) {
						str[cont]=c;
						cont++;
					}
				}
				str[cont]='\0';
				int numero_linee = atoi(str);
				write(piped[j][1], &numero_linee, sizeof(int));
				
				ritorno = (int) ((status >> 8) & 0xFF);
				exit(ritorno);
			}
		}
	}
	
	
	/* codice del padre */
	
	//padre chiude lato pipe che non usa
	for(j = 0; j< N; j++) {
		close(piped[j][1]);
	}
	
	long int somma = 0;
	int lunghezza;
	
	//il padre recupera le informazioni dai figli rispettando l'ordine dei file
	for(j=0; j < N; j++) {
		/*il padre recuperare valori interi dei figli*/
		read(piped[j][0], &lunghezza, sizeof(lunghezza));
		somma = somma + lunghezza;
		printf("Il figlio di indice %d ha comunicato il valore %d per il file %s\n", j, lunghezza, argv[j+1]);
	}
	
	puts("* * * * * * * * * * * *");
	printf("La somma delle lunghezza dei file e' %ld\n", somma);
	puts("* * * * * * * * * * * *");
	
	/*Il padre aspetta i figli*/
	for (j = 0; j < N; j++) {
		pid = wait(&status);
		if (pid < 0) {
			puts("Errore in wait\n");
			exit(5);
		}
		
		if((status & 0xFF) != 0)
			printf("Figlio con pid %d terminato in modo anomalo\n", pid);
		else {
			ritorno = (int)((status >> 8) & 0xFF);
			if(ritorno != 0) {
				printf("Il figlio con pid=%d ha ritornato %d e quindi vuol dire che il nipote non e' riuscito ad eseguire il wc oppure il figlio o nipote sono incorsi in errori\n", pid, ritorno);
			}
			else
				printf("Il figlio con pid=%d ha ritornato %d\n", pid, ritorno);
		}
	}
	
	exit(0);
	
}
