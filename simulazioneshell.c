#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>

int main(int argc, char** argv){
	int pid;
	int pidFiglio, status, ritorno;
	char st[80]; //comando senza opzioni
	char risp[3];
	for(;;){
		printf("Inserire il comando da eseguire: \n");
		scanf("%s", st);


		if((pid = fork())<0){perror("fork");exit(errno);}
		
		if(pid == 0){
			execlp(st,st,(char *)0);
			perror("Errore esecuzione comando");
			exit(errno);
		}


		if((pidFiglio = wait(&status)) <0){
			perror("errore wait");
			exit(errno);
		}

		if((status & 0xFF) != 0)
			printf("Figlio con pid=%d terminato in modo anomalo \n", pidFiglio);
		else{
			ritorno = (int)((status >> 8) & 0xFF);
			printf("Il figlio con pid %d ha terminato con valore %d \n", pidFiglio,ritorno);


			printf("Eseguire un altro comando? (si\\no) \n");
			scanf("%s",risp);
			
			if(strcmp(risp,"si"))
				exit(0);
		}
	}

}
