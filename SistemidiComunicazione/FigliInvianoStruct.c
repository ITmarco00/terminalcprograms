#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>

typedef int pipe_t[2];
typedef struct {
	int pid;		/* campo c1 del testo: pid del processo figlio */
	char secondo;		/* campo c2 del testo: secondo carattere */
	char penultimo;	/* campo c3 del testo: penultimo carattere della linea(considerando il terminatore di linea come ultimo carattere) */
} str;

int main(int argc, char **argv) {
	
	int L;					/* numero di linee del file / numero di processi figli */
	int pid;				/* pid per fork() */
	int fd; 				/* file descriptor */
	int pidFiglio, status, ritorno;	/* per valore di ritorno figli */
	str pip;				/* struttura usata dal figlio e per la pipe */
	pipe_t *piped;				/* pipe di comunicazione tra figlio e padre */
	int q, i, j;				/* indici usati per i cicli */
	char linea[250];			/* linea letta */
	int nr;				/* controllo sulla read del padre */

	/* Controllo sul numero di parametri */
	if(argc != 3) {
		printf("Errore: numero di parametri incorretto\n");
		exit(1);
	}
	
	/* Controllo sul tipo del secondo parametro: numero strettamente positivo e <=255 */
	if(atoi(argv[2]) < 0) {
		printf("Errore: secondo parametro non numerico\n");
		exit(2);
	}
	/* assegno alla variabile L il valore del secondo parametro */
	L=atoi(argv[2]);
	if(L <= 0 || L > 255) {
		printf("Errore: secondo parametro non positivo e minore o uguale a 255\n");
		exit(2);
	}
	
	/* Allocazione dell'array di L pipe descriptors */
	piped = (pipe_t *) malloc (L*sizeof(pipe_t));
	if(piped == NULL) {
		printf("Errore nella allocazione della memoria\n");
		exit(3);
	}
	
	/* Creazione delle L pipe figli-padre */
	for(q=0; q < L; q++) {
		if(pipe(piped[q]) < 0) {
			printf("Errore nella creazione della pipe\n");
			exit(4);
		}
	}
	
	/* Creazione dei processi figli */
	for(q=0;q<L;q++) {
		if((pid=fork()) < 0) {
			printf("Errore creazione figlio\n");
			exit(5);
		}
		
		if (pid == 0) {
			/* codice del figlio */
			
			/* chiusura pipe inutilizzate */
			for(j=0;j<L;j++) {
				close(piped[j][0]);
				if(q != j)
					close(piped[j][1]);
			}
			
			/* apertura del file */
			fd = open(argv[1], O_RDONLY);
			if(fd < 0) {
				printf("IMpossibile aprire il file %s\n", argv[1]);
				exit(-1);
			}
			
			/* inizializzo i come contatore della linea 
			e j come indice di linea*/
			i = 1;
			j = 0;
			
			/* ciclo di lettura del file */
			while(read(fd, &(linea[j]), 1)) {
				
				if(linea[j] == '\n') {
					
					/* trovata la linea da guardare */
					if(q+1 == i) {
						
						/* inserisco nella struct i dati ricercati */
						pip.pid=getpid();
						pip.secondo=linea[1];
						pip.penultimo=linea[j-1];
						write(piped[q][1], &pip, sizeof(pip));
						break;	/* usciamo dal ciclo di lettura */
						
					}
					else {
						j = 0; /* azzero l'indice di linea per la prossima linea */
						i++; /* se troviamo un terminatore di linea incrementiamo il conteggio delle linee */
					}
					
				}
				else j++;
				
			}
			
			/* il figlio ritorno il numero della linea analizzata */
			exit(q+1);
		}
	}
	
	/* codice del padre */
	
	/* chiusura delle pipe inutilizzate dal padre */
	for(j=0;j<L;j++) {
		close(piped[j][1]);
	}
	
	/* il padre recupera le informazioni dalle pipe */
	for(q=0;q<L;q++) {
		
		/* legge la struttura */
		nr = read(piped[q][0], &pip, sizeof(pip));
		if(nr != 0) {
			
			/* controllo se c2 e c3 sono uguali */
			if(pip.secondo == pip.penultimo) {
				/* stampo le info richieste */
				printf("Il figlio di indice %d e pid %d ha trovato che il secondo carattere (%c) e il penutlimo carattere (%c) della linea %d del file %s sono uguali\n", q, pip.pid, pip.secondo, pip.penultimo, q+1, argv[1]);
			}
			
		}
		
	}
	
	/* il padre aspetta i figli */
	for(q=0;q<L;q++) {
		pidFiglio=wait(&status);
		
		if( pidFiglio < 0 ) {
			printf("Errore in wait\n");
			exit(6);
		}
		
		
		if((status & 0xFF) != 0)
			printf("Figlio con pid %d terminato il nodo anomalo\n", pidFiglio);
		else {
			ritorno=(int)((status >> 8) & 0xFF);
			printf("IL figlio con pid=%d ha ritornato %d (se 255 problemi)\n", pidFiglio, ritorno);
		}
	}
	
	exit(0);
}
