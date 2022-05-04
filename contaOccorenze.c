#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#define BUFSIZE 1  //leggo un char alla volta  

int main(int argc, char** argv){
	
	long int totale=0;
	int infile,nread; //variabile usata per l'apertura del file 
	char cletto; //contenitore dei dati letti 
	char c; //carattere passato come parametro
	
	if(argc != 3){
		printf("Errore, argc ha %d parametri ma deve avere 2 parametri:\n",argc
);
		printf("comando file [char]");
		exit(1);
	}
	
	if((infile = open(argv[1],O_RDONLY)) < 0){
		printf("Errore nellapertura del file infile: %d",infile);
		exit(2);
	}

	if(strlen(argv[2]) != 1){
		printf("Errore, il secondo parametro non è un solo carattere\n");
		exit(3);
	}
	c=argv[2][0];

	while((nread = read(infile,&cletto,BUFSIZE)) > 0){
		if(cletto == c)totale++;
	}
	
	printf("Trovate %ld occorenze \n",totale);

	exit(0);
}
