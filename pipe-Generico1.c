#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#define MSGSIZE 512

int main(int argc, char** argv){
	int pid, pidFiglio, ritorno, status;
	int piped[2];
	char mess[MSGSIZE];


	if(argc != 2){
		printf("Errore, passare un solo parametro nome di file \n");
		exit(1);
	}	

	//creo la pipe
	if(pipe(piped) < 0){
		printf("Errore nella creazione della piepe \n");
		exit(2);
	}


	//creo il processo figlio
	if((pid = fork()) < 0){
		printf("Errore nella creazione del processo figlio \n");
		exit(3);
	}

	if(pid == 0){
		//sono nel figlio 

		int fd, cont=0;
		close(piped[0]);//chudo la parte di lettura della pipe

		if((fd = open(argv[1],O_RDONLY)) < 0){
			printf("Errore in apertura del file %s con fd %d", argv[1],fd);
		}
		
		int nummess=0;
		while(read(fd,&(mess[cont]),1) != 0){ //leggo un carattere alla volta
			if(mess[cont] == '\n'){
				mess[cont] = '\0'; //inserisco il terminatore 
				write(piped[1],&cont,sizeof(cont)); //scrivo la lunghezza della stringa
				write(piped[1],mess,cont); //scrivo la stringa
				cont=0;
				nummess++;
			}
			else cont++;
		}

		printf("Il figlio %d ha scritto %d messaggi sulla pipe \n",getpid(),nummess);
		exit(0);
	}

	//processo padre 
	close(piped[1]); //chiudo la parte di scrittura
	printf("Il processo padre %d sta per iniziare a lggere dalla pipe \n",getpid());
	
	int lunghezza;
	int cont=0;
	while(read(piped[0],&lunghezza,sizeof(lunghezza))){
		
		read(piped[0], mess, lunghezza);
		
		printf("%d: %s \n",cont,mess);
		cont++;
		lunghezza=0;
	}

	//attendo la terminazione del processo figlio
	if((pidFiglio = wait(&status)) < 0){
		printf("Errore nella wait \n"); 
	}

	if((status & 0xFF) != 0){
		printf("Processo figlio terminato in modo anomalo \n");
		exit(6);
	}
	else{
		ritorno = (int)((status >> 8) & 0xFF);
		printf("Processo figlio %d terminato con codice %d (se 255 problemi) \n",pidFiglio, ritorno);
	}
	exit(0);
}
