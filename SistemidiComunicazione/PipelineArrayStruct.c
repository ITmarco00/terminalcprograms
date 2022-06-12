#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <stdbool.h>

#define N 26			/* numero di processi da creare (uno per ogni carattere) */

typedef int pipe_t[2];
typedef struct {
	char car;		/* campo v1 del testo */
	long int num_occ;	/* campo v2 del testo */
} s_occ;

void scambia(s_occ *a, s_occ *b) {
	s_occ temp = *a;
	*a=*b;
	*b=temp;
}

void bubbleSort (s_occ v[], int dim) {
	int i;
	bool ordinato = false;
	
	while (dim > 1 && !ordinato) {
		ordinato = true;
		
		for(i=0;i<dim-1;i++) {
			if(v[i].num_occ > v[i+1].num_occ) {
				scambia(&v[i], &v[i+1]);
				ordinato = false;
			}
		}
		
		dim--;
	}	
}

int main(int argc, char **argv) {
	
	int *pid;				/* array di pid di fork */
	int i,j; 				/* indici */
	int fd;				/* file descriptor */
	char ritorno;
	int pidFiglio, status;			/* per valore di ritorno figli */
	char carattere = 'a';			/* primo carattere alfabetico */
	s_occ *letta; 				/* struttura usata dai figli e dal padre */
	pipe_t *pipes;				/* array di pipe usate a pipeline da primo figlio, a secondo figlio .... ultimo figlio e poi a padre: ogni processo (a parte il primo) legge dalla pipe i-1 e scrive sulla pipe i */
	long int n_occ;			/* conteggio delle occorrenze calcolate da ogni figlio */
	char ch;				/* carattere dalla lettura del file */
	int nr,nw;              		/* variabili per salvare valori di ritorno di read/write da/su pipe */
	
	/* Controllo sul numero di parametri: 1 parametro */
	if (argc != 2) {
		printf("Errore numero di parametri (inserire un parametro)\n");
		exit(1);
	}
	
	/* allocazione array di pid */
	pid=(int *) malloc(N * sizeof(int));
	if(pid == NULL) {
		printf("Errore allocazione array pid\n");
		exit(2);
	}
	
	/* allocazione array di struct */
	letta = (s_occ *)malloc(N*sizeof(s_occ));
	if(letta == NULL) {
		printf("Errore allocazione array struct\n");
		exit(3);
	}
	
	/* allocazione pipe */
	pipes=(pipe_t *)malloc(N*sizeof(pipe_t));
	if(pipes == NULL) {
		printf("Errore allocazione pipe\n");
		exit(4);
	}
	
	/* creazione pipe */
	for (i=0;i<N;i++) {
		if(pipe(pipes[i])<0)
		{
			printf("Errore creazione pipe\n");
			exit(5);
		}
	}
	
	/* creazione figli */
	for(i=0;i<N;i++) {
		if((pid[i]=fork()) < 0) {
			printf("Errore creazione figlio\n");
			exit(6);
		}
		
		if(pid[i] == 0) {
			//codice del figlio
			
			//prendo il carattere associato al figlio
			carattere = carattere + i;
			
			/* chiusura pipes inutilizzate */
			for (j=0;j<N;j++)
			{
				if (j!=i)
					close (pipes[j][1]);
				if ((i == 0) || (j != i-1))
					close (pipes[j][0]);
			}
			
			
			/* inizializziamo il contatore delle occorrenze */
			n_occ = 0;
			
			/* apertura file */
			if ((fd=open(argv[1],O_RDONLY))<0)
			{	
				printf("Impossibile aprire il file %s\n", argv[1]);
				exit(-1);
			}
			
			while(read(fd, &ch, 1)) {
				/* cerco il carattere */
				if(ch == carattere) {
					n_occ++;
				}
			}
			
			if(i==0) {
				// il primo figlio deve preparare la struttura da inviare
				letta[i].car=carattere;
				letta[i].num_occ=n_occ;
			}
			else {
				/* lettura da pipe della struttura per tutti i figli a parte il primo */
				nr=read(pipes[i-1][0],letta,N * sizeof(s_occ));
				if (nr != N * sizeof(s_occ))
				{	
					printf("Figlio %d ha letto un numero di byte sbagliati %d\n", i, nr);
					exit(-1);
				}
				
				letta[i].car=carattere;
				letta[i].num_occ=n_occ;
			}
			
			/* scrivo sulla pipe  */
			nw=write(pipes[i][1],letta,N * sizeof(s_occ));
			if (nw != N * sizeof(s_occ))
			{
		      		printf("Figlio %d ha scritto un numero di byte sbagliati %d\n", i, nw);
		        	exit(-1);
			}
			
			/* ogni figlio deve tornare l'ultimo carattere letto */
			exit(ch);
		}
	}
	
	
	/* codice del padre */
	
	/* chiusura pipe: tutte meno l'ultima in lettura */
	for(i=0;i<N;i++)
	{
		close (pipes[i][1]);
		if (i != N-1) close (pipes[i][0]);
	}
	
	
	/* il padre deve leggere l'array di strutture che gli arriva dall'ultimo figlio */
	nr=read(pipes[N-1][0],letta, N * sizeof(s_occ));
	if (nr != N * sizeof(s_occ))
	{
		printf("Padre ha letto un numero di byte sbagliati %d\n", nr);
		exit(7);
	}
	
	bubbleSort(letta, N);
	
	//stampo i risultati
	for(i=0;i<N;i++) {
		int indice_car=letta[i].car - carattere;
		printf("Il figlio di indice %d e pid=%d associato al carattere %c: occorrenze=%ld\n", indice_car, pid[indice_car], letta[i].car, letta[i].num_occ);
	}
	
	/* Il padre aspetta i figli */
	for (i=0; i < N; i++)
	{
		pidFiglio = wait(&status);
		if (pidFiglio < 0)
		{
		        printf("Errore in wait\n");
		        exit(8);
		}
		if ((status & 0xFF) != 0)
		        printf("Figlio con pid %d terminato in modo anomalo\n", pidFiglio);
		else
		{ 
			ritorno=(char)((status >> 8) & 0xFF);
			printf("Il figlio con pid=%d ha ritornato carattere %c di valore ASCII=%d(se 255 problemi)\n", pidFiglio, ritorno, (int)ritorno);
		} 
	}
	
	exit(0);
}
