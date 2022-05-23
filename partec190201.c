#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/wait.h>


int main(int argc, char**argv){

	int piped[2][2]; //array di due pipe, gli elementi di posto 0 sono per i processi dispari mentre gli altri per quelli pari 
	int N; //N è il numero dei processi da creare 
	int pid; //variabile usata per il valore di ritorno della fork()
	int fdr;	//file descriptor utilizzato per ogni figlio per aprire il file in lettura
	int i;		//indice
	int tot=0;	//variabile usata dal padre per contenere i caratteri scritti su stdout 
	int nr0, nr1;	//nr0 numero di caratteri letti dalla pipe pari e nr0 letti dalla pipe dispari
	char ch, ch0, ch1;
	int pidFiglio, status, ritorno;

	
	//controllo dei parametri 
	if(argc < 3){
		printf("Errore nel numero di parametri: %d \n",argc);
		exit(1);
	}
	
	N=argc-1;
	
	if(pipe(piped[0]) < 0){
		printf("Errore nella creazione della prima pipe \n");
		exit(2);
	}
		
	if(pipe(piped[1]) < 0){
		printf("Errore nella creazione della seconda pipe \n");
		exit(3);
	}

	for(i =0; i<N; i++){
		if((pid = fork()) <0){
			printf("Errore nella creazione del processo figlio %d con pid  %d \n",i,pid);
			exit(-1);
		}
		if(pid == 0){
			printf("Sono il processo figlio con pid %d e sono associato al file %s \n",getpid(),argv[i+1]);

			//chiususra dei lati della pipe non utilizzati
			close(piped[0][0]);
			close(piped[0][1]);
			close(piped[i%2][1]); //lato di scrittura dell'altra pipe 


			if((fdr = open(argv[i+1],O_RDONLY))<0){
				printf("Errore in apertura del file %s \n",argv[i+1]);
				exit(-1);
			}

			while(read(fdr, &ch,1)>0){
				if(i%2 == 0){
					if(isalpha(ch)){
						write(piped[0][1],&ch,1);
					}
				}
				else{
					if(isdigit(ch)){
						write(piped[0][0],&ch,1);
					}
				}	
			}
			exit(0);
		}
	}


	//processo padre 
	close(piped[0][1]);
	close(piped[1][1]);
	
	nr0 = read(piped[0][0],&ch0,1);
	nr1 = read(piped[1][0],&ch1,1);
	
	while((nr0 != 0) || (nr1 != 0)){
		tot = tot + nr0 + nr1;
		write(1,&ch1,nr1);
		write(1,&ch0,nr0);
		nr0 = read(piped[0][0],&ch0,1);
		nr1 = read(piped[0][1],&ch1,1);
	}
	printf("Numero di caratteri letti e scritti %d\n",tot);


	for(i=0; i<N; i++){

		if((pidFiglio = wait(&status)) < 0){
			printf("Errore nella wait \n");
			exit(5);
		}

		if((status & 0xFF) != 0){
			printf("Figlio con pid %d terminato in modo anomalo \n",pidFiglio);
		}
		else{
			ritorno=(int)((status >> 8) & 0xFF);
			printf("Il figlio con pid %d ha ritornato %d (se 255 problemi) \n",pidFiglio,ritorno);
		}
	
	}
	exit(0);
}
