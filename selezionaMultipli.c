#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#define PERM 0644

int myhead(int n, char* f){
	int fd, nr;
	char buffer[n];
	
	//apro il file in lettura
	if((fd = open(f,O_RDONLY)) < 0){
		printf("Errore nell'apertura del file %d \n",fd);
		return 1;
	} 
	
	int i=1;
	while((nr = read(fd,buffer,n)) > 0){
		printf("Il carattere multiplo %c in posizione %d  \n",buffer[n-1],i);
		i++;
	}
	return 0;
}

int main(int argc, char ** argv){

	//controllo il numero di parametri passato 
	if(argc != 3){

		printf("Errore, numero di parametri errato \n");
		exit(1);
	}

	int n = atoi(argv[2]);
	if(n <= 0){
		printf("Il parametro numerico deve essere positivo \n");
		exit(2);
	}

	int status = myhead(n,argv[1]);
	if(status != 0){
			printf("Errore %d \n",status);
			exit(status);
		}
	exit(0);
}
