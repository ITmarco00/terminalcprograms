#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
int myhead(int n){
	char c;

	/*if((fd = open(0,O_RDONLY)) < 0){
		printf("Errore nell'apertura dello stdinput fd=%d \n",fd);
		return 1;
	}*/

	int i =0;
	while(read(0,&c,1)){
	
		if(c == '\n')i++;
		write(1,&c,1);
		if(i>n)break;
	
	}

	return 0;
}

int main(int argc, char**argv){
	if(argc != 2){
	 	printf("Errore, il numero di parametri passato è errato \n");
		exit(1);
	}

	int n = atoi(argv[1]);
	n = n*(-1);
	if(n <= 0){
		printf("Il parametro passato è errano [-n] \n");
		exit(2);
	}

	int status = myhead(n);
	if(status != 0){
		printf("Errore nell'esecuione \n");
	}
	exit(0);
}
