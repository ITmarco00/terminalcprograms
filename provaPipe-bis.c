#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>


int main(int argc, char** argv){
	int fd1, fd2;
	int piped[2];

	if(argc != 3){
		printf("Errore, numero errato di parametri passati \n");
		exit(2);
	}	

	if((fd1 = open(argv[1],O_RDONLY)) < 0){
		printf("Errore in apertura del file \n");
		exit(1);
	}
	printf("Valore del primo fd = %d \n",fd1);


	if((fd2 = open(argv[2],O_RDONLY)) < 0){
		printf("Errore in apertura del secondo file \n");
		exit(2);
	}
	printf("Valore del secondo fd= %d \n",fd2);
	
	if(pipe(piped) <0){
		printf("Errore in creazione della pipe \n");
		exit(3);
	}

	printf("Creata pipe con piped[0]= %d \n",piped[0]);
	printf("Creata pipe con piped[1]= %d \n",piped[1]);
	

}
