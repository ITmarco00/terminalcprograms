#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char ** argv){
	int fd1, fd2, fd3;
	
	if(argc != 4) // controllo il numero di parametri e devono essere esattamente 3
	{
		printf("Errore nel numero di parametri, dato che argc = %d \n",argc);
		exit(1);
	}
	
	//creo il file fd1
	if((fd1 = open(argv[1],O_RDONLY)) < 0){
		//se l'apertura non ha successo termino con errore
		printf("Errore in apertura file dato che fd1 = %d \n",fd1);
		exit(2);
	}
	printf("Valore di fd1= %d\n",fd1);
	close(fd1); //chiudo il file

	//creo il file fd2
	if((fd2 = open(argv[2],O_RDONLY)) < 0){
		printf("Errore in apertura file dato che fd2 = %d\n",fd2);
		exit(3);
	}
	printf("Valore di fd2 = %d\n",fd2);
	close(fd2);

	//creo il file fd3
	if((fd3 = open(argv[3],O_RDONLY)) < 0){
		printf("Errore in apertura file dato fd3 = %d \n",fd3);
		exit(4);
	}
	printf("Valore di fd3 = %d\n",fd3);
	close(fd3);

	exit(0);
}
