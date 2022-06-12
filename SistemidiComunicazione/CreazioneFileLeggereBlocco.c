#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>

#define PERM 0644

typedef int pipe_t[2];

int main(int argc, char **argv) {
	
	int B;					/* numero intero strettamente positivo, che rappresenta il numero dei figli da creare */
	int L;					/* numero intero strettamente positivo, che rappresenta la dimensione (in caratteri) di F ed è multiplo di B (entrambe queste caratteristiche non importa siano controllate) */
	char *FCreato;				/* variabile per nome file da creare */
	int fdw;				/* file descriptor per il file .Chiara creato dal padre */
	int pidFiglio, status, ritorno;	/* per valore di ritorno figli */
	int *pid;				/* array di pid per fork */
	char *buff;				/* array dinamico per il blocco corrente */
	pipe_t *pipes;				/* pipe di comunicazione tra padre e figlio */
	int q, j;				/* indici per i cicli */
	int fd;				/* file descriptor del file passato come primo parametro che i figli devono leggere */
	char c;				/* carattere per la lettura da pipe del padre */
	
	/* Controllo sul numero di parametri: main F L B*/
	if(argc != 4) {
		printf("Errore nel numero di parametri\n");
		exit(1);
	}
	
	/* Controllo sul terzo parametro: B numero intero strettamente positivo */
	if(atoi(argv[3]) <= 0) {
		printf("Errore: %s non numerico o positivo\n", argv[3]);
		exit(2);
	}
	B = atoi(argv[3]);	//assegno a B il terzo parametro
	
	/* assegno a L il secondo parametro */
	L = atoi(argv[2]);
	
	/* Allocazione spazio per file da creare: bisogna allocare una stringa lunga come il nome del file + il carattere '.' + i caratteri della paraola Chiara (6) + il terminatore di stringa */
	FCreato=(char *)malloc(strlen(argv[1])+8);
	if(FCreato == NULL) {
		printf("Errore nelle malloc\n");
		exit(3);
	}
	
	sprintf(FCreato, "%s.Chiara", argv[1]);
	fdw = creat(FCreato, PERM);
	if(fdw < 0) {
		printf("Impossibile creare il file %s\n", FCreato);
		exit(4);
	}
	
	/* Allocazione array pid */
	pid=(int *)malloc(B*sizeof(int));
	if(pid == NULL) {
		printf("Errore allocazione array pid\n");
		exit(5);
	}
	
	/* Allocazione pipe */
	pipes = (pipe_t *)malloc(B*sizeof(pipe_t));
	if(pipes == NULL) {
		printf("Errore allocazione pipe\n");
		exit(6);
	}
	
	/* Creazione pipe */
	for(q=0;q<B;q++) {
		if((pipe(pipes[q])) != 0) {
			printf("Errore creazione delle pipe\n");
			exit(7);
		}
	}
	
	/* Creazione dei processi figli */
	for(q=0;q<B;q++) {
		if((pid[q]=fork()) < 0) {
			printf("Errore creazione figlio\n");
			exit(8);
		}
		
		if(pid[q] == 0) {
			/* codice del figlio */
			
			/* Chiudo lati pipe inutilizzati dal figlio */
			for(j=0;j<B;j++) {
				close(pipes[j][0]);
				if(q != j)
					close(pipes[j][1]);
			}
			
			/* ogni figlio deve aprire il file in sola lettura */
			fd = open(argv[1], O_RDONLY);
			if(fd < 0) {
				printf("Impossibile aprire il file %s\n", argv[1]);
				exit(-1);
			}
			
			/* allocazione dinamica buff */
			buff = (char *)malloc(L/B*sizeof(char));
			if(buff == NULL) {
				printf("Errore allocazione buff\n");
				exit(-1);
			}
			
			/* ci posizioniamo nella posizione giusta in cui leggere */
			lseek(fd, (long)q*L/B, SEEK_SET);
			/* leggo dal file il blocco di dimensione L/B */
			read(fd, buff, L/B);
			/* invio al padre l'ultimo carattere del blocco letto */
			write(pipes[q][1], &(buff[(L/B)-1]), 1);
			
			/* ritorno al padre la dimensione del blocco esaminato */
			exit(L/B);
		}
	}
	
	/* codice del padre */
	
	/* chiusura della pipe inutilizzate dal padre */
	for(j=0;j<B;j++) {
		close(pipes[j][1]);
	}
	
	/* leggo dalle pipe tra padre e figli il carattere inviato */
	for(q=0;q<B;q++) {
		/* leggo dalla pipe */
		read(pipes[q][0], &c, 1);
		
		/* scrivo sul file creato .Chiara il carattere letto dalla pipe */
		write(fdw, &c, 1);
	}
	
	/* il padre aspetta i figli */
	for(q=0;q<B; q++) {
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
