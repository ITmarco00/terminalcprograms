#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define PERM 0644

typedef int pipe_t[2];

int main(int argc, char **argv) {
	
	int Q;					/* numero di parametri passati */
	int fdC;				/* file descriptor Camilla */
	int fd;				/* file descriptors file associati ai processi */
	pipe_t *pipes;				/* pipes */
	int i, j;				/* indici per for */
	int q;					/* indice dei processi figli */
	char linea[250];			/* variabile per memorizzare la linea */
	int pid;				/* pid per fork */
	int pidFiglio, status, ritorno;	/* per valore di ritorno figli */
	bool carNum = false;			/* variabile booleana per indicare se il primo carattere della linea è numerico */
	char c;				/* carattere per la lettura */
	int lunghezza;				/* contatore della lunghezza della stringa */
	int totLinee;				/* totale linee inviate al padre */
	
	/* Controllo sul numero di parametri */
	if(argc < 3) {
		printf("Errore: numero di parametri erratto\n");
		exit(1);
	}
	
	Q = argc - 1;
	
	/* creo il file di nome Camilla */
	fdC = creat("Camilla", PERM);
	if(fdC < 0) {
		printf("Errore creazione file Camilla\n");
		exit(2);
	}
	close(fdC);
	
	/* Allocazione array pipes */
	pipes = (pipe_t *) malloc(Q*sizeof(pipe_t));
	if(pipes == NULL) {
		printf("Errore allocazione array pipe\n");
		exit(3);
	}
	
	/* Creazione pipe */
	for (i=0;i<Q;i++) {
		if(pipe(pipes[i])<0)
		{
			printf("Errore creazione pipe\n");
			exit(4);
		}
	}
	
	/* stampa di debugging */
	printf("Il padre con pid=%d deve creare %d figli\n", getpid(), Q);
	
	/* Creazione processi figli */
	for(q=0;q<Q;q++) {
		if((pid = fork()) < 0) {
			printf("Errore creazione figlio\n");
			exit(5);
		}
		
		if(pid == 0) {
			/* codice del figlio */
			
			/* Chiudo lati pipe inutilizzati dal figlio */
			for(j=0;j<Q;j++) {
				close(pipes[j][0]);
				if(q != j)
					close(pipes[j][1]);
			}
			
			/* Apertura file associato al processo */
			fd = open(argv[q+1],O_RDONLY);
			if(fd < 0) {
				printf("Impossibile aprire il file %s\n", argv[q+1]);
				exit(-1);
			}
			
			/* inizializzo il contatore della lunghezza della linea e il contatore delle linee inviate al padre */
			lunghezza = 0;
			totLinee = 0;
			
			/* lettura del file */
			while(read(fd, &c, 1)) {
				if(lunghezza == 0) {
					/* controllo se il primo carattere della linea è numerico */
					if(isdigit(c)) {
						carNum = true;;
					}
				}
				
				/* aggiungo il carattere alla stringa e aumento il contatore della lunghezza */
				linea[lunghezza] = c;
				lunghezza++;
				
				if(c == '\n') {
					/* ho raggiunto la fine della linea */
					
					if(lunghezza < 10 && carNum == true) {
						/* se il primo carattere e' numerico e la lunghezza della stringa e' minore di 10, invio la linea al padre */
						
						write(pipes[q][1], linea, lunghezza);
						totLinee++;
					}
					
					/* azzero le variabili per la prossima linea */
					lunghezza=0;
					carNum=false;
					
				}
				
			}
			
			/* il figlio restituisce il numero di linee che ha inviato al padre */
			exit(totLinee);
		}
	}
	
	
	/* codice del padre */
	
	/* chiusura della pipe inutilizzate dal padre */
	for(j=0;j<Q;j++) {
		close(pipes[j][1]);
	}
	
	/* scorro tutti i processi figli */
	for(q=0;q<Q;q++) {
		
		/* inizializzo lunghezza della linea */
		lunghezza = 0;
		while(read(pipes[q][0], &c, 1)) {
			
			if(c == '\n') {
				//aggiungo il terminatore di linea
				linea[lunghezza+1] = '\0';
				lunghezza++;
				
				printf("Il figlio di indice %d ha inviato la linea: \"%s\" del file %s\n ", q, linea, argv[q+1]);
				
				//azzero la lunghezza della linea
				lunghezza = 0;
			}
			else {
				linea[lunghezza] = c;
				lunghezza++;
			}
			
		}
		
	}
	
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
	
	exit(0);
		
}
