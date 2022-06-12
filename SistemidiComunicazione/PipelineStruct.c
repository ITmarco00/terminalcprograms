/* Soluzione della parte C del compito della II prova in itinere del 26 Maggio 2017 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h> 
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>

typedef int pipe_t[2];
typedef struct{
	long int occ; 		/* numero massimo di occorrenze (campo c1 del testo) */
        int id; 		/* indice figlio (campo c2 del testo) */
	long int somma; 	/* somma del numero di occorrenze (campo c3 del testo) */
	} s_occ;



int main (int argc, char **argv)
{
int N; 			/* numero di file */
int *pid;		/* array di pid per fork */
pipe_t *pipes;		/* array di pipe usate a pipeline da primo figlio, a secondo figlio .... ultimo figlio e poi a padre: ogni processo (a parte il primo) legge dalla pipe i-1 e scrive sulla pipe i */
int i,j; 		/* indici */
int fd; 		/* file descriptor */
int pidFiglio, status, ritorno;	/* per valore di ritorno figli */
char Cx, ch;  		/* carattere da cercare e carattere letto da file */
long int cur_occ; 	/* conteggio delle occorrenze calcolate da ogni figlio */
s_occ letta; 		/* struttura usata dai figli e dal padre */
int nr,nw;              /* variabili per salvare valori di ritorno di read/write da/su pipe */

/* controllo sul numero di parametri almeno 2 file e un carattere */
if (argc < 4)
{
	printf("Errore numero di parametri\n");
	exit(1);
}

/* controlliamo che l'ultimo parametro sia un singolo carattere */
if (strlen(argv[argc-1]) != 1)
{
	printf("Errore ultimo parametro non singolo carattere\n");
	exit(2);
}
 
/* individuiamo il carattere da cercare */
Cx = argv[argc-1][0];
printf("Carattere da cercare %c\n", Cx);

N = argc-2;
printf("Numero di processi da creare %d\n", N);

/* allocazione pid: il padre deve salvare i pid dei figli perche' deve risalire al pid del figlio dal suo indice */
if ((pid=(int *)malloc(N*sizeof(int))) == NULL)
{
	printf("Errore allocazione pid\n");
	exit(3); 
}

/* allocazione pipe */
if ((pipes=(pipe_t *)malloc(N*sizeof(pipe_t))) == NULL)
{
	printf("Errore allocazione pipe\n");
	exit(4); 
}

/* creazione pipe */
for (i=0;i<N;i++)
	if(pipe(pipes[i])<0)
	{
		printf("Errore creazione pipe\n");
		exit(5);
	}

/* creazione figli */
for (i=0;i<N;i++)
{
	if ((pid[i]=fork())<0)
	{
		printf("Errore creazione figlio\n");
		exit(6);
	}
	else if (pid[i]==0)
	{ 	/* codice figlio */
		printf("Sono il figlio %d e sono associato al file %s\n", getpid(), argv[i+1]);
		/* nel caso di errore in un figlio decidiamo di ritornare un valore via via crescente rispetto al massimo valore di i (che e' N-1) */
		/* chiusura pipes inutilizzate */
		for (j=0;j<N;j++)
		{
			if (j!=i)
				close (pipes[j][1]);
			if ((i == 0) || (j != i-1))
				close (pipes[j][0]);
		}
 
		/* inizializziamo il contatore di occorrenze */
		cur_occ= 0;
	
		/* apertura file */
		if ((fd=open(argv[i+1],O_RDONLY))<0)
		{	
			printf("Impossibile aprire il file %s\n", argv[i+1]);
			exit(N);
		}
		while(read(fd,&ch,1)>0)
		{
			/* cerco il carattere */
			if (ch == Cx)
	 		{
				cur_occ++;
		 		/*
				printf("Sono il figlio di indice %d e pid %d e ho trovato %ld occorrenze del carattere %c\n", i, getpid(), cur_occ, Cx);
				*/
   			}	
		}
		if (i == 0)
		{	/* il figlio di indice 0 deve preparare la struttura da mandare al figlio seguente */
			letta.id = 0;
			letta.occ = cur_occ;
			letta.somma = cur_occ; 
    		}
		else
 		{	
			/* lettura da pipe della struttura per tutti i figli a parte il primo */
			nr=read(pipes[i-1][0],&letta,sizeof(s_occ));
			if (nr != sizeof(s_occ))
        		{	
        			printf("Figlio %d ha letto un numero di byte sbagliati %d\n", i, nr);
        			exit(N+1);
        		}
 			/*
			printf("HO ricevuto da figlio di indice %d che ha trovato %ld occorrenze del carattere %c\n", letta.id, letta.occ, Cx);
  			*/
			if (letta.occ < cur_occ)
	        	{  	/* il figlio di indice i ha calcolato un numero di occorrenze maggiore e quindi bisogna aggiornare i valori di letta */
        			letta.id = i;
        			letta.occ = cur_occ;
        		}	
        		/* il valore della somma, va aggiornato comunque */
			letta.somma += cur_occ;
		}	
		/* tutti i figli mandano in avanti, l'ultimo figlio manda al padre una struttura */
		nw=write(pipes[i][1],&letta,sizeof(s_occ));
		if (nw != sizeof(s_occ))
        	{
              		printf("Figlio %d ha scritto un numero di byte sbagliati %d\n", i, nw);
                	exit(N+2);
        	}
		/* ogni figlio deve tornare il proprio indice d'ordine */
		exit(i);
	}
} /* fine for */

/* codice del padre */
/* chiusura pipe: tutte meno l'ultima in lettura */
for(i=0;i<N;i++)
{
	close (pipes[i][1]);
	if (i != N-1) close (pipes[i][0]);
}

/* il padre deve leggere la struttura che gli arriva dall'ultimo figlio */
nr=read(pipes[N-1][0],&letta,sizeof(s_occ));
if (nr != sizeof(s_occ))
{
        printf("Padre ha letto un numero di byte sbagliati %d\n", nr);
        exit(8);
}
/* il padre deve stampare i campi della struttura ricevuta */
printf("Il figlio di indice %d e pid %d ha trovato il numero massimo di occorrenze %ld del carattere %c nel file %s\n", letta.id, pid[letta.id], letta.occ, Cx, argv[letta.id+1]);
printf("I figli hanno trovato in totale %ld occorrenze del carattere %c nei file\n", letta.somma, Cx);

/* Il padre aspetta i figli */
for (i=0; i < N; i++)
{
        pidFiglio = wait(&status);
        if (pidFiglio < 0)
        {
                printf("Errore in wait\n");
                exit(9);
        }
        if ((status & 0xFF) != 0)
                printf("Figlio con pid %d terminato in modo anomalo\n", pidFiglio);
        else
        { 
		ritorno=(int)((status >> 8) & 0xFF);
        	printf("Il figlio con pid=%d ha ritornato %d (se > di %d problemi)\n", pidFiglio, ritorno, N-1);
        } 
}
exit(0);
}
