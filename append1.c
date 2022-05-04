#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#define PERM 0644

int main(int argc, char**argv){

	int fd, nread;
	char buffer[BUFSIZ];


	if(argc != 2){
		printf("Errore, passare il nome di un file \n");
		exit(1);
	}

	if((fd = open(argv[1],O_CREAT|O_APPEND | O_WRONLY,PERM)) < 0){
		printf("Errore nell'apertura del file: %d \n",fd);
		exit(2);
	}
	
	while((nread = read(0,buffer,BUFSIZ)) > 0){ //leggo da input
	
		if(write(fd,buffer,nread) < nread){
			close(fd);
			exit(3);
		}
	}
	exit(0);
}
