#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

typedef int pipe_t[2];

int main(int argc, char ** argv){
	int pid, pidFiglio, status, ritorno;
	int N; //numero di processi figli
	pipe_t *piped; //array dinamico delle n pipe
	if(argc != 2){
		printf("Passare il numero di processi da generare come parametro: \n");
		exit(1);
	}

	N= atoi(argv[1]);

	printf("DEBUG -Sono il processo padre e sto per allocare %d pipe \n",N);
	piped = (pipe_t *)malloc(N*sizeof(pipe_t));
	for(int i=0; i<N; i++){
		printf("Sto per creare la pipe %d \n",i);
		if(pipe(piped[i])<0){
			printf("Errore nella creazione della pipe %d \n",i);
		}
	}

	printf("DEBUG-Sono il processo PADRE con PID %d e sto per generare %d processi figli \n",getpid(),N);

	for(int i =0; i<N; i++){
		if((pid = fork())<0){
			printf("Errore nella generazione del %d processo figlio \n",i);
			exit(-1);
		}
		if(pid == 0){
			printf("Sono il processo FIGLIO numero %d con PID %d \n",i,getpid());
			//chiudo le pipe che non uso
			for(int j =0; j<N; j++){
				close(piped[j][0]);//chiudo tutti i lati di lettura
				if(j != i){
					close(piped[j][1]);//chiudo i lati di scrittura di tutti gli altri processi
				}

			}

			//scrivo sulla pipe il mio indice 
			write(piped[i][1],&i,sizeof(i));
			exit(i);
		}

	}

	//codice del padre 
	printf("DEBUG- Sono il processo padre, ho generato i processi figli \n");
	
	//chiudo tutti i lati di scrittura delle pipe
	for(int i =0; i<N; i++){
		close(piped[i][1]);
	}
	int val;
	for(int i=0; i<N; i++){
		while(read(piped[i][0],&val,sizeof(val))){
			printf("Ho letto il valore: %d nella pipe %d \n",val,i);
		}
	}

	//attesa dei processi figli
	for(int i =0; i<N; i++){
		if((pidFiglio = wait(&status))<0){
			printf("Errore nella wait di indice %d con pidFiglio = %d \n",i,pidFiglio);
			exit(-1);
		}

		if((status & 0xFF) != 0){
			printf("Processo figlio di indice %d terminato in modo anomalo con pidFiglio = %d \n",i,pidFiglio);
			exit(-1);
		}
		else{
			ritorno = (int)((status >> 8) & 0xFF);
			printf("Processo figlio di indice %d con pid %d ha ritornato il valore: %d \n",i,pidFiglio,ritorno);
		}

	}
	exit(0);

}
