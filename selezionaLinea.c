#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

int sel_linea(int n, char*f){
	char c;
	int fd;
	
	if((fd = open(f,O_RDONLY)) < 0){
		printf("Errore nell'apertura del file \n");
		return 3;
	}	

	int cont=0;
	while(read(fd,&c,1)){
		
		if(cont == (n-1)){
			int nwrite = write(1,&c,1);
			if(nwrite != 1){
				printf("Errore nella scrittura \n");
				return 2;
			}
		}

		if(c == '\n'){
                	cont++;
		}
		//if(cont == n)write(1,&c,1);
	}
	if(cont < n){
		printf("La linea non è presente nel file \n");
	}
	exit(0);
}

int main(int argc, char** argv){

	if(argc != 3){
		printf("Errore, il numero di parametri è errato, passare [File] [n]\n");
		return 1;
	}
	
	//prendo il parametro numerico e lo controllo
	int n = atoi(argv[2]);
	if(n <= 0){
		printf("Errore, il parametro 2 deve essere strettamente maggiore di zero\n");
		return 2;
	}
	
	int status = sel_linea(n,argv[1]);
	if( status != 0){
		printf("Errore, nella selezione della linea \n");
		return 3;
	}
	
	return 0;
}
