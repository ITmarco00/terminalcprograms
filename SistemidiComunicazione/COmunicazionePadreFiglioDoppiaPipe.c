#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <ctype.h>
#include <time.h>

#define PERM 0644

typedef int pipe_t[2];

int mia_random(int n) {
	int casuale;
	casuale = rand() % n;
	return casuale;
}

int main(int argc, char **argv) {

	//char str[]="/tmp/creato";
	char str[]="fileprovacreato";
	int pid;			//process identifier per le fork()
	char c;			//carattere per lettura da file
	int fd, Fcreato;
	int H;
	int caratteri_scritti = 0;		//caratteri scritti per figlio
	int pidFiglio, ritorno, status;	//variabili per attesa figli
	
	//controllo sul numero di parametri: N>=4 -> N+1>=5
	/*if(argc < 6){
		puts("Errore: numero di parametri incorretto");
		exit(1);
	}*/
	
	int M = argc - 1;	//numero di parametri inseriti
	int N = M - 1;		//numero di file passati come parametri
	
	
	//controllo sull'ultimo parametro
	if(atoi(argv[M]) >= 0) {
		H = atoi(argv[M]);
		if(H <= 0 || H >= 255) {
			printf("Errore: ultimo parametro non positivo o non minore di 255\n");
			exit(2);
		}
	}
	else {
		printf("Errore: ultimo parametro non numerico\n");
		exit(2);
	}
	
	//inizializzo il seme
	srand(time(NULL));
	
	//creo il file
	if((Fcreato = creat(str, PERM)) < 0) {
		puts("Errore nella creazione del file");
		exit(3);
	}

	
	/* allocazione memoria per pipe fra padre-figlio e figlio padre */
	pipe_t *pipe_fp = malloc(N*sizeof(pipe_t));
	pipe_t *pipe_pf = malloc(N*sizeof(pipe_t));
	if(pipe_pf == NULL || pipe_fp == NULL) {
		puts("Errore nella malloc");
		exit(4);
	}
	
	//creo le pipe
	for(int i=0; i<N;i++) {
		if(pipe(pipe_fp[i]) != 0) {
			printf("Errore creazione delle pipe figlio-padre\n");
			free(pipe_fp);
			exit(5);
		}
		if(pipe(pipe_pf[i]) != 0) {
			printf("Errore creazione delle pipe padre-figlio\n");
			free(pipe_pf);
			exit(6);
		}
	}
	
	//il padre crea N processi figli
	for (int j = 0; j < N; j++) {
		if((pid = fork()) < 0){
			puts("Errore nella fork");
			exit(7);
		}
		
		if (pid == 0) {
			//codice del figlio
			printf("Sono il processo figlio di indice %d e pid %d associato al file %s\n", j, getpid(), argv[j+1]);
			
			//chiudo lati della pipe che il figlio non usa
			for(int k=0;k<N;k++) {
				close(pipe_fp[k][0]);
				close(pipe_pf[k][1]);
				if(k != j) {
					close(pipe_fp[k][1]);
					close(pipe_pf[k][0]);	
				}	
			}
			
			if((fd = open(argv[j+1], O_RDONLY)) < 0) {
				printf("Errore apertura file %s\n", argv[j+1]);
				exit(-1);	
			}
			
			
			
			//calcolo lunghezza dela linea
			int lung = 0;
			while(read(fd, &c, 1)) {
			
				lung++;
				//puts("OK");
				if(c == '\n') {
					//invio al padre la lunghezza della linea
					//printf("File %s Invio al padre %d\n", argv[j+1], lung);
					int nw = write(pipe_fp[j][1], &lung, sizeof(int));
					if(nw < 0) {
						puts("Errore invio lunghezza linea");
						exit(-2);
					}
					
					int indice;
					if(read(pipe_pf[j][0], &indice, sizeof(int)) <= 0) {
						printf("Errore nel ritorno della variabile indice\n");
						exit(-3);
					}
					//printf("Ricevo indice %d\n", indice);
					if(indice < lung) {
						int posizione = lung - indice;
						
						//sposto il file point indietro nella nuova posizione
						for(int z=0; z <= posizione; z++) {
							lseek(fd, -1L,SEEK_CUR);
						}
						
						int carattere_da_inserire;
						read(fd, &carattere_da_inserire, 1);
						if(write(Fcreato, &carattere_da_inserire,1) < 0) {
							puts("Errore nella write su file creato");
							exit(-4);
						}
						
						//printf("Inserisco nel file carattere numero %d\n", indice);
						for(int z=0; z < posizione; z++) {
							lseek(fd, +1L,SEEK_CUR);
						}
						
						//aumento il contatore di caratteri scritti su file
						caratteri_scritti++;
					}
					
					
					//azzero la lunghezza della linea corrente
					lung=0;
				}
				
				
				
				
			}
			
			exit(caratteri_scritti);
		}
	}
	
	/* Codice del padre */
	
	//chiudo lati pipe che padre non usa
	for(int k=0;k<N;k++) {
		close(pipe_fp[k][1]);
		close(pipe_pf[k][0]);		
	}
	
	//allocazione di una array per le lunghezze
	int *lunghezze = malloc(N*sizeof(int));
	if(lunghezze == NULL) {
		printf("Errore nella malloc delle lunghezze\n");
		exit(8);
	}
	
	//leggo la pipe figlio-padre
	for(int i=0; i < H; i++) {
		
		for (int j = 0; j < N; j++) {
			read(pipe_fp[j][0], &lunghezze[j], sizeof(int));
			//printf("%d\n",nread);
			//printf("Padre riceve: %d\n", lunghezze[j]);
		}
		
		//prendo in modo random una lunghezza
		int lunghezza_random = mia_random(N);
		int lunghezza_scelta = lunghezze[lunghezza_random];
		//printf("%d", lunghezza_scelta);
		int indice_random = mia_random(lunghezza_scelta);
		
		for(int k=0; k<N; k++) {
			//comunico a tutti i figli l'indice scelto
			write(pipe_pf[k][1], &indice_random, sizeof(int));
		}
	}
	
	//Attessa terminazione figli
	for(int j=0; j< N; j++) {
		pidFiglio = wait(&status);
		if(pidFiglio < 0) {
			puts("Errore wait");
			exit(9);
		}
		if((status & 0xFF) != 0) {
			printf("Figlio con pid %d terminato in modo anomalo\n", pidFiglio);
		}
		else {
			ritorno=(int)((status >> 8) & 0xFF);
			printf("Il figlio con pid=%d ha ritornato %d\n", pidFiglio, ritorno);
		}
	}
	
	exit(0);
}
