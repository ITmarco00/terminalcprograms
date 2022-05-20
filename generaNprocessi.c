#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char**argv){	
	int pid, pidFiglio, status, ritorno;

	if(argc != 2){
		printf("Errore, passare un numero intero \n");
		exit(1);
	}

	int n = atoi(argv[1]);
	if(n<=0){
		printf("Il numero passato deve essere un numero maggiore di zero \n");
		exit(2);
	}


	//genero gli n processi 
	for(int i =0; i<n; i++){
		if((pid = fork())<0){
			printf("Errore nella generazione dell' %d processo figlio pid = %d \n",i,pid);
			exit(3);
		}
		
		if(pid == 0){
			printf("Ho generato il %d processo figlio con pid : %d \n ",i,pid);
			exit(i);
		}
	}

	printf("Attendo la terminazione dei processi figli \n");

	//attendo i processi figli
	for(int i =0; i<n; i++){
		if((pidFiglio = wait(&status)) <0){
			printf("Errore nella wait per il processo con pid %d \n",pidFiglio);
			exit(4);
		}

		if((status & 0xFF) != 0){
			printf("Processo con pid %d, terminato in modo anomalo \n",pidFiglio);
		
		}
		else{
			ritorno = (int)((status >> 8) & 0xFF);
			printf("Il processo con pid %d è terminato con codice %d", pidFiglio,ritorno);
		}	
	}
	exit(0);
}

