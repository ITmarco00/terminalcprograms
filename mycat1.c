#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#define BUFSIZE 1

int main(int argc, char** argv){
	int nread;		//valore di ritorno della nread
	char buffer[BUFSIZE];	//usato per i caratteri
	int fd = 0;		//usato per la open 
	int finito = 0;
	int N=0;
	int i=0;

	if(argc >= 2)N=argc-1;

	while(!finito){
		
		if(argc >= 2){
			if((fd = open(argv[i+1], O_RDONLY)) < 0){
				printf("Errore in apertura del file %s, dato che fd=%d\n",argv[i+1], fd);
				exit(2);
			}
		}
		i++;
		while((nread = read(fd, buffer, BUFSIZE)) > 0){
			write(1,buffer,nread);
		}

		if((argc == 1) || (i == N)){
			finito=1;
		}
		
	}
	exit(0);

}
