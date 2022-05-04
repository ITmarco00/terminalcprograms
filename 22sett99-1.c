#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#define BUFSIZE 1

int main(int argc, char** argv){
	int fin;
	char buffer; // contenitore dei caratteri letti
	
	if(argc != 4){
		printf("Errore, i parametri passati devono essere 3: [file] [char] [char]\n");
		exit(1);
	}

	if(strlen(argv[2]) != 1){
	 printf("Errore, il secondo parametro deve essere un singolo carattere\n");
		exit(2);
	}
	
	if(strlen(argv[3]) != 1){
		printf("Errore, il terzo parametro deve essere un carattere\n");
		exit(3);
	}
	
	if((fin = open(argv[1],O_RDWR)) < 0){
		printf("Errore di apertura del file %s \n",argv[1]);
		exit(4);
	}
	
	while(read(fin,&buffer,BUFSIZE)){
		if(buffer == argv[2][0]){
			lseek(fin, -1L, 1); //sposto il puntatore prima del carattere
			write(fin, &argv[3][0], 1);
		}
	}
	exit(0);
}
