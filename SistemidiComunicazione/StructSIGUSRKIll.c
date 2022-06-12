#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>

typedef int pipe_t[2];
typedef struct{
	int id;			//indice d'ordine di un processo (c1 nel testo)
	int num;			//numero di caratteri (compresso terminatore) (c2 nel testo)
	} s;
	
/* Variabili globali */
char linea[256];			// buffer linea: si può supporre che una linea sia lunga massimo 255 caratteri (256 con il terminatore)
int stampate;				// numero linee stampate (come variabile globale e' automaticamente inizializzata a 0)
s cur;					// struttura usata dal figlio corrente

//funzioni per trattare i segnali SIGUSR1 e SIGUSR2: la prima fara' la scrittura su standard output, mentre la seconda non fara' nulla
void scrivi(int sig) {
	//disabilita il segnale SIGUSR1
	signal(SIGUSR1, SIG_IGN);
	
	/* se il padre ha detto che devo scrivere allora si scrive la linea su standard output usando la write su 1 */
	write(1, linea, cur.num);
	stampate++;
	
	//resetta il segnale, per poterlo riutilizzare
	signal(SIGUSR1, scrivi);
}

void salta(int sig) {
	//disabilita il segnale SIGUSR1
	signal(SIGUSR2, SIG_IGN);
	
	//non fa nulla
	
	//resetta il segnale, per poterlo riutilizzare
	signal(SIGUSR2, salta);
}

int main(int argc, char **argv) {
	
	int N = argc - 2;			//numero di file
	int H;					//lunghezza in linee del file
	int *pid;				//array contenente i pid per fork
	pipe_t *pipes;				/* array di pipe usate a pipeline da primo figlio, a secondo figlio .... ultimo figlio e poi a padre: ogni processo (a parte il primo) legge dalla pipe i-1 e scrive sulla pipe i */
	int fd; 				/* file descriptor */
	int pidFiglio, status, ritorno;	/* per valore di ritorno figli */
	s pip;					/* struttura usata dal figlio e per la pipe */
	
	/* controllo sul numero di parametri: il testo non specifica nessun vincolo, ma per avere senso l'esecuzione ci deve essere almeno un file e il numero di linee */
	if(argc < 3) {
		printf("Errore: numero di parametri incorretto\n");
		exit(1);
	}
	
	//controllo sull'ultimo parametro: numero intero strettamente positivo e minore di 255
	if(atoi(argv[N+1]) < 0) {
		printf("Errore: %s non numerico\n", argv[N+1]);
		exit(2);
	}
	H = atoi(argv[N+1]);
	if(H >= 255 || H <= 0) {
		printf("Errore: %d non positivo o non minore di 255\n", H);
		exit(2);
	}
	
	/* allocazione pid: il padre deve salvare i pid dei figli perche' deve risalire al pid del figlio dal suo indice */
	if ((pid=(int *)malloc(N*sizeof(int))) == NULL)
	{
		printf("Errore allocazione array pid\n");
		exit(3); 
	}
	
	/* allocazione pipe */
	if ((pipes=(pipe_t *)malloc(N*sizeof(pipe_t))) == NULL)
	{
		printf("Errore allocazione array pipe\n");
		exit(4); 
	}
	
	/* creazione pipe */
	for (int i=0;i<N;i++)
		if(pipe(pipes[i])<0)
		{
			printf("Errore creazione pipe\n");
			exit(5);
		}
		
	/* padre aggancia le due funzioni (scrivi e salta) che useranno i figli alla ricezione dei segnali inviati al padre */
	signal(SIGUSR1, scrivi);
	signal(SIGUSR2, salta);
		
	/* creazione figli */
	for(int i=0;i<N;i++) {
		if((pid[i]=fork()) < 0) {
			printf("Errore creazione figlio\n");
			exit(6);
		}
		
		if(pid[i] == 0) {
			/* codice figlio */
			printf("Sono il figlio %d e sono associato al file %s\n", getpid(), argv[i+1]);
			
			/* chiusura pipes inutilizzate */
			for (int j=0;j<N;j++)
			{
				if (j!=i)
					close(pipes[j][1]);
				if ((i == 0) || (j != i-1))
					close(pipes[j][0]);
			}
			
			/* inizializzazione struttura */
			cur.id = i;
			cur.num = 0;
			
			/* apertura del file */
			if((fd = open(argv[i+1], O_RDONLY)) < 0) {
				printf("Errore apertura file %s\n", argv[i+1]);
				exit(-1);
			}
			
			//finche' leggo da file
			while(read(fd, &linea[cur.num], 1) > 0) {
				/* aggiorno conteggio lunghezza linea */
				cur.num++;
				
				if(linea[cur.num-1] == '\n') {
					
					if(i != 0) {
						/* se non sono il primo figlio */
						read(pipes[i-1][0], &pip,sizeof(s));
						if(cur.num < pip.num) {
							/* se la lunghezza calcolata e' minore di quella ricevuta, si deve mandare avanti quella ricevuta */
							cur.id = pip.id;
							cur.num = pip.num;
						}
					}
					
					/* comunicazione lunghezza linea */
					write(pipes[i][1], &cur, sizeof(s));
					
					/* aspetto segnale dal padre */
					pause();
					
					/* dopo l'arrivo del segnale azzero il conteggio linea precedente */
					cur.num = 0;
					cur.id = i;	
					
				}
			}
			
			exit(stampate);
		}
	}
	
	/* codice del padre */
	
	/* chiusura pipe: tutte meno l'ultima in lettura */
	for(int i=0;i<N;i++)
	{
		close(pipes[i][1]);
		if (i != N-1) close (pipes[i][0]);
	}
	
	/* il padre deve leggere H strutture, tante quante sono le linee lette dai figli */
	for(int j=0; j<H; j++) {
		read(pipes[N-1][0], &pip, sizeof(s));
		
		/* il padre deve mandare il segnale che corrisponde a scrivi solo al processo di cui gli e' arrivato l'indice, mentre agli altri deve mandare il segnale che corrisponde a salta */
		for(int i=0;i<N;i++) {
			sleep(1); //per sicurezza
			
			if(i == pip.id){
				kill(pid[i], SIGUSR1);
			}	
			else {
				kill(pid[i], SIGUSR2);
			}
				
		}
	}
	
	/* il padre aspetta i figli */
	for(int i=0;i<N; i++) {
		pidFiglio=wait(&status);
		if(pidFiglio < 0) {
			printf("Errore in wait\n");
			exit(7);
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
