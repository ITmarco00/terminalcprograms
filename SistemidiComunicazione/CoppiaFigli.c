#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

#define PERM 0644

typedef int pipe_t[2];		/* tipo per contenere i file descriptors di una pipe */

int main(int argc, char **argv) {
	int N;			/* numero di file: i processi figli saranno il doppio */
	int H;			/* numero intero positivo dispari */
	int pid;		/* variabile per fork */
	pipe_t *pipe_sp;	/* array di pipe per la comunicazione dai figli secondi della coppia ai figli primi della coppia */
	int fd;		/* variabile per open */
	char *FCreato;		/* variabile per nome file da creare da parte dei processi figli primi della coppia */
	int fdw;		/* variabile per creat */
	char *buffer;		/* variabile per leggere dai figli */
	int lung;		/* variabile per tenere traccia del numero di blocchi presenti nel file */
	int status, pidFiglio, ritorno; /* per wait */
	int i, j;		/* indici per cicli */
	int nr, nw;		/* per controllo su read/write */
	
	/* Controllo sul numero di parametri: anche se non indicato supponiamo che N sia maggiore o uguale a 2 */
	if(argc < 4) {
		printf("Errore numero parametri %d\n", argc);
		exit(1);
	}
	
	/* Controllo sul primo parametro: numero dispari H */
	H = atoi(argv[1]);
	if( (H <= 0) || (H % 2 == 0) ) {
		printf("Errore numero H %s\n", argv[1]);
		exit(2);
	}
	
	/* Calcoliamo il numero dei file */
	N = argc - 2;
	
	printf("Numero processi da creare %d con H=%d\n", 2*N, H);
	
	/* allocazione memoria dinamica per buffer */
	buffer=(char *)malloc(H*sizeof(char));
	if(buffer == NULL) {
		printf("Errore nella malloc per buffer\n");
		exit(3);
	}
	
	/* allocazione memoria dinamica per pipe_sp. NOTA BENE: servono un numero di pipe che e' la meta' del numero di figli */
	pipe_sp=(pipe_t *)malloc(N*sizeof(pipe_t));
	if(pipe_sp == NULL) {
		printf("Errore nelle malloc per le pipe\n");
		exit(4);
	}
	
	/* creazione delle pipe: ATTENZIONE VANNO CREATE N pipe */
	for(i=0; i<N; i++) {
		if(pipe(pipe_sp[i]) != 0) {
			printf("Errore creazione delle pipe\n");
			exit(5);
		}
	}
	
	/* creazione dei processi figli: ne devono essere creati 2*N */
	for(i=0;i<2*N;i++) {
		pid=fork();
		if(pid < 0) { /* errore */
			printf("Errore nella fork con indice %d\n", i);
			exit(6);
		}
		
		if(pid == 0) {
			// codice del figlio: in caso di errore torniamo 0 che non e' un valore accettabile (per quanto risulta dalla specifica della parte shell)
			if(i < N) {
				//siamo nel codice dei figli primi della coppia
				
				/* stampa di debugging */
				printf("PRIMO DELLA COPPIA-Figlio di indice %d e pid %d associato al file %s\n", i, getpid(), argv[i+2]);
				
				/* i figli primi della coppia devono creare il file specificato */
				FCreato=(char *)malloc(strlen(argv[i+2])+11); /* bisogna allocare una stringa lunga come il nome del file + il carattere '.' + i caratteri della paraola mescolato (9) + il terminatore di stringa */
				if(FCreato == NULL) {
					printf("Errore nelle malloc\n");
					exit(0);
				}
				/* copiamo il nome del file associato al figlio primo della coppia */
				strcpy(FCreato, argv[i+2]);
				/* concateniamo la stringa specificata dal testo */
				strcat(FCreato, ".mescolato");
				fdw = creat(FCreato, PERM);
				if(fdw < 0) {
					printf("Impossibile creare il file%s\n", FCreato);
					exit(0);
				}
				
				/* chiudiamo le pipe che non servono */
				/*  ogni figlio PRIMO della coppia legge solo da pipe_sp[i] */
				for(j=0;j<N;j++) {
					close(pipe_sp[j][1]);
					if(j != i)
						close(pipe_sp[j][0]);
				}
				
				/* ogni figlio deve aprire il suo file associato */
				fd = open(argv[i+2], O_RDONLY);
				if(fd < 0) {
					printf("Impossibile aprire il file %s\n", argv[i+2]);
					exit(0);
				}
				
				/* calcoliamo la lunghezza in blocchi del file */
				lung = lseek(fd, 0L, 2) / H;
				/* bisogna riportare l'I/O pointer all'inizio del file */
				lseek(fd, 0L, 0);
				for(j=0; j<lung/2;j++) {
					read(fd, buffer, H);
					/* ogni blocco letto dal processo PRIMO della coppia, bisogna scriverlo sul file */
					write(fdw, buffer, H);
					/* dobbiamo a questo punto aspettare il blocco dal processo SECONDO della coppia */
					nr=read(pipe_sp[i][0], buffer, H);
					if(nr != H) {
						printf("Errore in lettura da pipe %d\n", i);
						exit(0);
					}
					/* ogni blocco ricevuto dal processo SECONDO della coppia, bisogna scriverlo sul file */
					write(fdw, buffer, H);
				}
			}
			else {
				/* siamo nel codice dei figli secondi della coppia */
				
				/* stampa di debugging */
				printf("SECONDO DELLA COPPIA-Figlio di indice %d e pid %d associato al file %s\n", i, getpid(), argv[i-N+2]);
				
				/* chiudiamo le pipe che non servono */
				/* ogni figlio SECONDO della coppia scrive solo su pipe_sp[i-N] */
				for(j=0;j<N;j++) {
					close(pipe_sp[j][0]);
					if(j != i-N) { /* ATTENZIONE ALL'INDICE CHE DEVE ESSERE USATO */
						close(pipe_sp[j][1]);
					}
				}
				
				/* ogni figlio deve aprire il suo file associato: siamo nei figli secondi della coppia e quindi attenzione all'indice */
				fd=open(argv[i-N+2], O_RDONLY);
				if (fd < 0) {
					printf("Impossibile aprire il file %s\n", argv[i-N+2]);
					exit(0);
				}
				
				/* Calcoliamo la lunghezza in blocchi del file */
				lung = lseek(fd, 0L, 2) / H;
				/* bisogna posizionare l'I/O pointer a meta' del file */
				lseek(fd, (long)lung/2 * H, 0);
				for(j=0;j<lung/2;j++) {
					read(fd, buffer, H);
					/* ad ogni blocco letto dal SECONDO processo della coppia, bisogna inviare al processo PRIMO della coppia: attenzione all'indice */
					nw=write(pipe_sp[i-N][1], buffer, H);
					if(nw != H){
						printf("Errore in scrittura su pipe %d\n", i-N);
						exit(0);
					}
				}
			}
			
			exit(lung/2); /* torniamo il numero di blocchi letti (supposto <= di 255) */
		}
	}
	
	// codice del padre
	
	/* chiudiamo tutte le pipe, dato che le usano solo i figli */
	for(i=0;i<N;i++) {
		close(pipe_sp[i][0]);
		close(pipe_sp[i][1]);
	}
	
	/* attesa della terminazione dei figli */
	for(i=0;i<2*N;i++) {
		pidFiglio=wait(&status);
		if(pidFiglio < 0) {
			printf("Errore wait\n");
			exit(7);
		}
		if((status & 0xFF) != 0)
			printf("Figlio con pid %d terminato in modo anomalo\n", pidFiglio);
		else {
			ritorno=(int)((status >> 8) & 0xFF);
			printf("IL figlio con pid=%d ha ritornato %d (se 0 problemi)\n", pidFiglio, ritorno);
		}
	}
	
	exit(0);
}
