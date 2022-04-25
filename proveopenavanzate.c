#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#define PERM 0644	//in ottale per i diritti UNIX

int main(int argc, char** argv){

	int fd; //usiamo una sola variabile, non serve agire contemporaneamente su diversi file
	
	if(argc != 2){
		printf("Errore, numero di parametri sbagliato dato che argc = %d\n",argc);
		printf("Ci vuole 1 solo argomento con scritto \"prima\" oppure \"seconda\" \n");
		exit(1);
	}
	
	//apertura in creazione
	if((fd = open("file1",O_CREAT | O_WRONLY,PERM)) < 0){
		printf("Errore in creazione file \"filename\"\n");
		exit(1);
	}
	else printf("File \"file1\" creato correttamente! \n");
	
	if(strcmp(argv[1],"prima") == 0){//se il parmetro che ho passato è uguale a "prima" scrivo per la prima volta
		char * str = "Questa è la prima volta che scrivo sul file file1";
		write(fd,str,strlen(str));
	}
	else{ 
		char * str = "Seconda volta che scriviamo sul file file1";
		write(fd,str,strlen(str));
	}
	//apertura in creazione solo se non esiste
	if((fd = open("file1", O_CREAT | O_EXCL | O_WRONLY, PERM)) < 0){
		printf("il file file1 esiste\n");
	}
	else
	{
		printf("Il file file1 non esisteva: creato\n");
		char * str = "Questa è la prima volta che scriviamo sul file";
		write(fd,str, strlen(str));
	}

	//apertura distruggendo il contenuto precedente
	if((fd = open("paperina", O_TRUNC | O_WRONLY)) < 0){
		printf("il file papaerina non esiste \n");
		exit(2);
	}
	else{
		printf("Il file paperina esisteva: troncato\n");
		if(strcmp(argv[1],"prima") == 0){
			char * str = "Questa è la prima volta che scriviamo sul file\n";
			write(fd,str,strlen(str));
		}
		else{
			char * str = "Questa è la seconda volta che scriviamo sul file\n";
			write(fd,str,strlen(str));
		}
	}
	
	exit(0);
}
