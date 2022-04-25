//programma per verificare la dimensione della tabella dei file aperti 
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int main(int argc, char ** argv){
	int i =0;	//contatore per il ciclo infinito
	int fd; //variabile per il valore di ritorno di open
	
	if(argc != 2){
		printf("Errore nel numero di parametri passato argc = %d\n",argc);
		exit(1);
	}

	while(1)//ciclo infinito
	{
		if((fd = open(argv[1], O_RDONLY)) < 0){
			printf("Errore nell'apertura del file fd = %d",fd);
			printf("Valore di i = %d\n",i);

			exit(2);
		}
		else i++; //contatore che tiene traccia di quante volte apro il file (finchè c'è posto nella tabella dei file aperti)
	}
	
	exit(0);
}	
