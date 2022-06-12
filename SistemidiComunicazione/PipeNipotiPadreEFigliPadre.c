#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>

typedef int pipe_t[2];

int main(int argc, char **argv) {
	
	int N;					/* numero di file */
	pipe_t *pipeFiglio;			/* array pipe tra Figlio e Padre */
	pipe_t *pipeNipote;			/* array pipe tra Nipote e Padre */
	int ritorno, status, pidFiglio;	/* variabili per wait */
	int i, j;				/* indici */
	int fd;				/* file descriptor */
	int pid;				/* pid per fork */
	long int n_tras;			/* contatore di trasformazioni */
	char ch;				/* carattere per la lettura */
	char spazio = ' ';			/* carattere spazio */
	long int trasF, trasN;			/* variabili per la lettura da pipe del padre del numero di trasformazioni */
	
	/* Controllo sul numero di parametri: N>=2 */
	if(argc < 3) {
		printf("Errore numero di parametri\n");
		exit(1);
	}
	
	N = argc - 1;
	printf("Creazione %d figli e nipoti\n", N);
	
	/* allocazione array di pipe tra figlio-padre */
	pipeFiglio = (pipe_t *)malloc(N * sizeof(pipe_t));
	if(pipeFiglio == NULL) {
		printf("Errore allocazione pipe Figlio-Padre\n");
		exit(2);
	}
	
	/* allocazione array di pipe tra nipote-padre */
	pipeNipote = (pipe_t *)malloc(N * sizeof(pipe_t));
	if(pipeNipote == NULL) {
		printf("Errore allocazione pipe NIpote-Padre\n");
		exit(3);
	}
	
	/* creazione pipe figlio-padre */
	for (i=0;i<N;i++) {
		if(pipe(pipeFiglio[i])<0)
		{
			printf("Errore creazione pipe figlio-padre\n");
			exit(4);
		}
	}
	
	/* creazione pipe nipote-padre */
	for (i=0;i<N;i++) {
		if(pipe(pipeNipote[i])<0)
		{
			printf("Errore creazione pipe nipote-padre\n");
			exit(5);
		}
	}
	
	/* creazione figli */
	for(i=0;i<N;i++) {
		if((pid = fork()) < 0) {
			printf("Errore creazione figli\n");
			exit(6);
		}
		
		if(pid == 0) {
			/* codice del figlio */
			
			/* inizializzo il contatore di trasformazioni */
			n_tras = 0;
			
			/* chiudo le pipe che non utilizza il figlio */
			for(j=0;j<N;j++) {
				//close(pipeNipote[j][1]);
				//close(pipeNipote[j][0]);
				close(pipeFiglio[j][0]);
				if(j != i)
					close(pipeFiglio[j][1]);
			}
			
			/* creazione processo nipote */
			for(j=0; j < N; j++) {
				if((pid = fork()) < 0) {
					printf("Errore creazione nipote");
					exit(-1);
				}
				if(pid == 0) {
					/* codice del nipote */
					
					//inizializzo variabile contatore trasformazioni
					n_tras= 0;
					/* chiudo le pipe che non utilizza il figlio */
					for(j=0;j<N;j++) {
						//close(pipeFiglio[j][1]);
						//close(pipeFiglio[j][0]);
						close(pipeNipote[j][0]);
						if(j != i)
							close(pipeNipote[j][1]);
					}
					
					/* apertura del file associato corrispondente */
					if ((fd=open(argv[i+1],O_RDWR))<0)
					{	
						//printf("Impossibile aprire il file %s\n", argv[i+1]);
						exit(-1);
					}
					
					while(read(fd, &ch, 1)) {
						//controllo se e' un carattere alfabetico minuscolo
						if(ch >= 'a' && ch <= 'z'){
							int sost = ch - 32;
							lseek(fd, -1L, SEEK_CUR);
							
							//sostituisco con il carattere in maiuscolo
							write(fd, &sost, 1);
							n_tras++;
						}
						
					}
					
					/* invio al padre tramite pipe il numero di trasformazioni effettuate */
					write(pipeNipote[j][1], &n_tras, sizeof(long int));
					
					/* restituisco al figlio l'opportuno codice */
					exit(n_tras/256);
				}
			}
			
			//codice figlio
			
			/* chiudo le pipe utilizzate dal nipote */
			for(j=0;j<N;j++) {
				close(pipeNipote[j][1]);
				close(pipeNipote[j][0]);
				
			}
			
			/* apertura del file associato corrispondente */
			if ((fd=open(argv[i+1],O_RDWR))<0)
			{	
				printf("Impossibile aprire il file %s\n", argv[i+1]);
				exit(-1);
			}
			
			while(read(fd, &ch, 1)) {
				//controllo se e' un carattere numerico
				if(isdigit(ch) != 0) {
					lseek(fd, -1L, SEEK_CUR);
					//sostituisco con uno spazio
					write(fd, &spazio, 1);
					n_tras++;
					
				}
			}
			
			/* invio al padre tramite pipe il numero di trasformazioni effettuate */
			write(pipeFiglio[j][1], &n_tras, sizeof(long int));
			
			/*il figlio deve aspettare il nipote */
			pidFiglio=wait(&status);
			if(pidFiglio < 0) {
				printf("Errore in wait\n");
				exit(-1);
			}
			if((status & 0xFF) != 0) {
				printf("Nipote con pid %d terminato in modo anomalo\n", pid);
				exit(-1);
			}
			else {
				ritorno=(int)((status >> 8) & 0xFF);
				printf("Il nipote con pid=%d del figlio con pid=%d ha ritornato il codice %d (se -1 problemi)\n", pidFiglio, getpid(), ritorno);
			}
			
			/* restituisco al padre l'opportuno codice */
			exit(n_tras/256);
			
		}
	}
	
	/* codice del padre */
	
	/* chiudo le pipe inutilizzate */
	for(j=0;j<N;j++) {
		close(pipeFiglio[j][1]);
		close(pipeNipote[j][1]);
	}
	
	for(i=0;i<N;i++) {
		/* lettura dei dati dalle pipe e stampa dei risultati */
		read(pipeFiglio[i][0], &trasF, sizeof(long int));
		read(pipeNipote[i][0], &trasN, sizeof(long int));
		printf("Sul file %s effettuate %ld modifiche dal figlio e %ld modifiche dal nipote\n", argv[i+1], trasF, trasN);
	}
	
	/* Il padre aspetta i figli */
	for (i=0; i < N; i++)
	{
		pidFiglio = wait(&status);
		if (pidFiglio < 0)
		{
		        printf("Errore in wait\n");
		        exit(7);
		}
		if ((status & 0xFF) != 0)
		        printf("Figlio con pid %d terminato in modo anomalo\n", pidFiglio);
		else
		{ 
			ritorno=(char)((status >> 8) & 0xFF);
			printf("Il figlio con pid=%d ha ritornato il codice %d(se -1 problemi)\n", pidFiglio, ritorno);
		} 
	}
	
	exit(0);
}
