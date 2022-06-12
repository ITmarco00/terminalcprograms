#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>

typedef int pipe_t[2];

int main(int argc, char **argv) {
	
	int N;				/* Numero file per i figli*/
	int *pid;			/* array di pid per fork */
	pipe_t *pipeFiglioPadre;	/* array di pipe di comunicazione fra figli e padre */
	pipe_t *pipePadreFiglio;	/* array di pipe di comunicazione/sincronizzazione fra padre e figli. */
	int pidFiglio, status, ritorno;/* per valore di ritorno figli */
	int fd, fdp;			/* file descriptor */
	int *confronto;		/* array per il controllo sui caratteri */
	char token='x';		/* carattere il cui valore che serve solo per sincronizzare i figli */
	char c, cF, cP;		/* carattere per la lettura */
	
	/* Controllo sul numero di parametri: N>=2 --> N+1>=3 */
	if(argc < 4) {
		puts("Errore numero di parametri incorretto");
		exit(1);
	}
	
	N = argc - 2;
	printf("NUmero di processi da creare: %d\n", N);
	
	/* allocazione array per il controllo sui caratteri e riempimento */
	confronto=(int *)malloc(N*sizeof(int));
	if(confronto == NULL){
		printf("Errore allocazione array di confronto\n");
		exit(2);
	}
	for(int i=0;i<N;i++)
		confronto[i]=1;
	
	
	/* allocazione array per i pid */
	pid=(int *)malloc(N * sizeof(int));
	if(pid == NULL) {
		printf("Errore allocazione array pid\n");
		exit(3);
	}
	
	/* allocazione array di pipe figli-padre */
	pipeFiglioPadre=(pipe_t *)malloc(N*sizeof(pipe_t));
	if(pipeFiglioPadre == NULL) {
		printf("Errore allocazione array pipe figli-padre\n");
		exit(4);
	}
	
	/* allocazione array di pipe padre-figli */
	pipePadreFiglio=(pipe_t *)malloc(N*sizeof(pipe_t));
	if(pipePadreFiglio == NULL) {
		printf("Errore allocazione array pipe padre-figli\n");
		exit(5);
	}
	
	/* creazione pipe figlio-padre*/
	for(int i=0;i<N;i++) {
		if(pipe(pipeFiglioPadre[i])<0) {
			printf("Errore creazione pipe\n");
			exit(6);
		}
	}
	
	/* creazione di altre N pipe di comunicazione/sincronizzazione con il padre */
	for(int i=0;i<N;i++) {
		if(pipe(pipePadreFiglio[i]) < 0) {
			printf("Errore creazione pipe\n");
			exit(7);
		}
	}
	
	/* creazione processi figli con salvataggio pid */
	for(int i=0; i< N; i++) {
		if((pid[i]=fork()) < 0) {
			printf("Errore creazione figli\n");
			exit(8);
		}
		
		if(pid[i] == 0) {
			/* codice figlio */
			printf("Creato figlio di indice %d con pid %d associato al file %s\n", i, getpid(), argv[i+1]);
			
			/* chiusura pipes inutilizzate */
			for(int j=0;j<N;j++) {
				close(pipeFiglioPadre[j][0]);
				close(pipePadreFiglio[j][1]);
				if(j != i) {
					close(pipeFiglioPadre[j][1]);
					close(pipePadreFiglio[j][0]);
				}
			}
			
			/* apertura file */
			if((fd = open(argv[i+1], O_RDONLY)) < 0) {
				printf("Impossibile aprire il file %s\n", argv[i+1]);
				exit(-1);
			}
			
			/* ciclo di lettura da file solo dopo aver ricevuto l'indicazione dal padre */
			while(read(pipePadreFiglio[i][0], &token, 1)) {
				
				if(token == 't')
					break;
				
				read(fd,&c,1);
				//printf("HO LETTO IL TOKEN per il carattere %c\n", c);
				/* il carattere letto va mandato al padre per il confronto */
				
				write(pipeFiglioPadre[i][1], &c, 1);
				
				
			}
			
			/* il file e' terminato e quindi si deve ritornare al padre il carattere richiesto */
			exit(0);
		}
	}
	
	
	/* codice del padre */
	
	/* chiusura pipes inutilizzate */
	for(int i=0;i<N;i++) {
		close(pipeFiglioPadre[i][1]);
		close(pipePadreFiglio[i][0]);
	}
	
	/* apertura file */
	if((fdp = open(argv[N+1], O_RDONLY)) < 0 ) {
		printf("Errore apertura file %s\n",argv[N+1]);
		exit(9);
	}
	
	/* il padre deve leggere del file fino a che ce ne sono */
	while(read(fdp, &cP, 1)) {
		
		for(int i=0;i<N;i++) {
			/* il padre manda l'indicazione di leggere ad ogni figlio per ogni carattere */
			if(confronto[i] == 1) {
				write(pipePadreFiglio[i][1], &token, 1);
				
				/* il padre riceve il carattere letto dal figlio */
				read(pipeFiglioPadre[i][0], &cF, 1);
				
				if(cF != cP)
					confronto[i] = 0;
			
			}
			
			
			
		}
		
	}	
		/* il padre termina forzatamente gli altri figli a parte quello gia' terminato, mandando il segnale SIGKILL che non puo' essere ignorato o intercettato */
		for(int i=0;i<N;i++) {
			if(confronto[i] == 0) {
				if(kill(pid[i], SIGKILL) == -1) {
					/* controlliamo che la kill non fallisca a causa della terminazione di uno dei figli */
					printf("Figlio con pid %d non esiste e quindi e' gia' terminato\n", pid[i]);
				}
			}
			else {
				//per i figli per cui il confronto non e' fallito mandiamo una indicazione di uscire dal ciclo, altrimneti rimarebbero bloccati sulla read e avremo un deadclock: dato che il figlio aspetterebbe un token dal padre e il padre aspetterebbe che il figlio finisse
				token='t';
				write(pipePadreFiglio[i][1], &token, 1);
			}
		}
	
	
	
	/* il padre aspetta i figli */
	for(int i=0;i<N;i++) {
		pidFiglio = wait(&status);
		if(pidFiglio < 0) {
			printf("Errore in wait\n");
			exit(10);
		}
		
		if((status & 0xFF) != 0 )
			printf("Figlio con pid %d terminato in modo anomalo\n", pidFiglio);
		else {
			ritorno=(int)((status >> 8) & 0xFF);
			
			printf("Il figlio con pid=%d ha ritornato il valore %d (se 255 problemi)\n", pidFiglio, ritorno);
			
			for(int j=0;j<N;j++) {
				//se un figlio termina normalmente vuol dire che non e' stato ucciso dal SIGKILL
				if(pid[j] == pidFiglio) {
					printf("Questo significa che il figlio di indice %d ha verificato che il file %s e' uguale al file %s\n", j, argv[j+1], argv[N+1]);
				}
			}
		}
		
		
	}
	
	exit(0);
}
