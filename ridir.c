//programma che implementa la ridirezione sia in ingresso che in uscita 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#define PERM 0644 //in ottale per i diritti unix
#define BUFSIZE 1


int copyfile(char * f1, char * f2){
	int infile, outfile, nread; //usati per open e valore di ritorno read
	char buffer[BUFSIZE];	//usato per i caratteri
	
	close(0); //chiudo lo standard input
	if((infile = open(f1,O_RDONLY)) < 0) return 2; //apro il file f1 al posto dello stdinput nella TFA
	
	close(1); //chiudo lo standard output
	if((outfile = creat(f2,PERM)) < 0){ //apro il secondo file al posto dello stdoutput
		close(infile);
		return 3;
	}
	
	//se la creat ha successo viene occupato l'elemento di posto 1 della TFA del processo corrente 
	//e quindi il secondo file verrà usato come standard output

	while((nread = read(infile, buffer, BUFSIZE)) > 0){
		if(write(outfile, buffer, nread) < nread){
			//errore se non si riesce a scrivere 
			close(infile);
			close(outfile);
			return 4;
		}
	}

	close(infile);
	close(outfile);
	return 0;
}

int main(int argc, char** argv){

	int status;

	if(argc != 3){
		printf("Errore, numero di argomenti sbagliato dato che argc= %d\n",argc);
		printf("Ci vogliono due argomenti: nome-file-sorgente nome-file-destinazione\n");
		exit(1);
	}
	status = copyfile(argv[1], argv[2]);
	if(status != 0){
		printf("Ci sono stati degli errori durante la copia dato che status = %d\n",status);
		exit(status);
	}

}

