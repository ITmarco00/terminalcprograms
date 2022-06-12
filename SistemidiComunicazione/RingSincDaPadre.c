#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>

typedef int pipe_t[2];

int main(int argc, char **argv) {
	
	int Q;				/* numero di file/processi */
	int pid;			/* pid per fork */
	pipe_t *pipes;			/* array di pipe usate a ring da primo figlio, a secondo figlio... ultimo figlio e poi primo figlio: ogni processo legge dalla pipe q e scrive sulla pipe (q+1)%Q */
	int q, j;			/* indici */
	int fd;			/* file descriptor */
	int pidFiglio, status, ritorno;	/* per valore di ritorno figli */
	char segnaleSinc = 'o';	/* segnale di sincronizzazione */
	int k;				/* variabile per iterazione  */
	char c;			/* carattere per la lettura da file */
	int posizione;			/* posizione all'interno del file */
	int nr, nw;			/* variabili per il controllo sui caratteri letti e scritti */
	
	
	/* Controllo sul numero di parametri */
	if(argc < 3) {
		printf("Errore: numero di parametri incorretto\n");
		exit(1);
	}
	
	//assegno a Q il numero di file passati come parametri
	Q = argc - 1;
	printf("Numero di processi da creare: %d\n", Q);
	
	/* allocazione pipe */
	pipes = (pipe_t *)malloc(Q*sizeof(pipe_t));
	if(pipes == NULL) {
		printf("Errore allocazione pipe\n");
		exit(2);
	}
	
	/* Creazione pipe */
	for(q=0;q<Q;q++) {
		if(pipe(pipes[q]) < 0) {
			printf("Errore creazione pipe\n");
			exit(3);
		}
	}
	
	/* creazione figli */
	for(q=0;q<Q;q++) {
		if((pid = fork()) < 0) {
			printf("Errore creazione figlio\n");
			exit(4);
		}
		
		if(pid == 0) {
			/* codice figlio */
			
			/* chiusura pipes inutilizzate */
			for(j=0;j<Q;j++){
				if(j != q)
					close(pipes[j][0]);
				if(j != (q+1)%Q)
					close(pipes[j][1]);
			}
			
			/* apertura del file associato */
			if((fd=open(argv[q+1],O_RDONLY)) < 0) {
				printf("Impossibile aprire il file %s\n", argv[q+1]);
				exit(-1);
			}
			
			
			/* inizializzo la posizione nel file e la variabile k per l'iterazione*/
			posizione = 0;
			k = 0;
			
			while(read(fd, &c, 1)) {
			
				if(posizione == (q + (k*Q)) ) {
					
					/* dobbiamo aspettare di 'ok' dal figlio precedente */
					nr = read(pipes[q][0], &segnaleSinc, sizeof(char));
					if(nr != sizeof(char)) {
						printf("Figlio %d ha letto un numero di byte sbagliato %d\n", q, nr);
						exit(-1);
					}
					
					printf("Figlio con indice %d e pid %d ha letto il carattere %c\n", q, getpid(), c);
					
					/* invio il segnale 'ok' al figlio successivo */
					nw = write(pipes[(q+1)%Q][1], &segnaleSinc, sizeof(char));
					if(nw != sizeof(char)) {
						printf("Figlio %d ha scritto un numero di byte sbagliati %d\n", q, nw);
						exit(-1);
					}
					
					/* aumento k */
					k++;
					
				}
				
				posizione++;
			}
			
			/* il figlio restituisce al padre il numero d'ordine */
			exit(q);
		}
	}
	
	/* codice del padre */
	
	/* chiusura di tutte le pipe che non usa */
	for(q=1; q < Q; q++) {
		close(pipes[q][0]);
		close(pipes[q][1]);
	}
	
	/* ora si deve mandare il segnale di 'ok' al primo figlio */
	nw = write(pipes[0][1], &segnaleSinc, sizeof(char));
	if(nw != sizeof(char)) {
		printf("Padre ha scritto un  numero di byte sbagliato %d\n", nw);
		exit(5);
	}
	
	/* ora possiamo chiudere anche il lato di scrittura, ma ATTENZIONE NON QUELLO DI LETTURA */
	close(pipes[0][1]);
	
	/* il padre aspetta i figli */
	for(q=0;q<Q; q++) {
		pidFiglio=wait(&status);
		if(pidFiglio < 0) {
			printf("Errore in wait\n");
			exit(6);
		}
		
		if((status & 0xFF) != 0)
			printf("Figlio con pid %d terminato in modo anomalo\n", pidFiglio);
		else {
			ritorno=(int)((status >> 8) & 0xFF);
			printf("Il figlio con pid=%d ha ritornato %d (se 255 problemi!)\n", pidFiglio, ritorno);
		}
	}
	
}
