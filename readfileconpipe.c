#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

int main(int argc, char** argv){
	int fd;
	int pid, pidFiglio, status, ritorno;
	int piped[2];
	char msg[BUFSIZ];

	if(argc != 2){
		printf("Errore nel paggaggio dei parametri, passare solo il nome di un file esistente \n");
		exit(1);
	}


	//creo la pipe 
	if(pipe(piped) <0){
		printf("Errore in creazione della pipe \n");	
		exit(2);
	}

	if((pid = fork())<0){
		printf("Errore nella generazione del processo figlio \n");
		exit(3);
	}
	if(pid == 0){
		//sono nel processo figlio 
		//apro il file da leggere
		if((fd = open(argv[1],O_RDONLY)) < 0){
			printf("Errore in apertura del file da leggere \n");
			exit(4);
		}

		printf("Il processo figlio con pid: %d sta per iniziare a scrivere sulla pipe \n", getpid());
		
		//chiudo il lato di lettura
		close(piped[0]);
		int j=0;
		while(read(fd,msg,BUFSIZ)){
			write(piped[1],msg,BUFSIZ);
			j++;
		}
		printf("Il figlio ha scritto %d messaggi sulla piepe \n",j);
		exit(0);
	}

	//processo padre
	close(piped[1]);
	while(read(piped[0],msg,BUFSIZ)){
		printf("%s ",msg);
	}

	if((pidFiglio = wait(&status))<0){
		printf("Processo figlio terminato in modo anomalo \n");
		exit(5);
	}
	if((status & 0xFF) !=0){
		printf("Errore wait \n");
		exit(6);
	}
	else{
		ritorno =  (int)((status >> 8) & 0xFF);
		printf("Il figlio con pid %d ha terminato con valore %d (se 255 errore ) \n",pidFiglio,ritorno);
	}
	exit(0);
}
