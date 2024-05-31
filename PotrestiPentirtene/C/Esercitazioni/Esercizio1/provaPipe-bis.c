#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

int main(int argc, char **argv) 
{
    // variabili per contenere i valori di ritorno delle open
    int fd1, fd2;
    
    // controllo il numero di parametri
    if (argc != 3)
    {
        printf("Errore nel numero di parametri; argc = %d, ma dovrebbe essere = 3.\n", argc);
        exit(1);
    }

    // controllo se il primo file si apre correttamente
    if (fd1 = open(argv[1], O_RDONLY) < 0)
    {
        printf("Errore in apertura file per %s dato che fd1 = %d\n", argv[1], fd1);
        exit(2);
    }
    printf("Valore di fd1 = %d per %s\n", fd1, argv[1]);

    // controllo se il secondo file si apre correttamente
    if (fd2 = open(argv[2], O_RDONLY) < 0)
    {
        printf("Errore in apertura file per %s dato che fd1 = %d\n", argv[2], fd2);
        exit(3);
    }
    printf("Valore di fd2 = %d per %s\n", fd2, argv[2]);

    close(fd1);

    int piped[2];

    // creo una pipe
    if (pipe(piped) < 0)
    {
        printf("Errore nella creazione pipe\n");
		exit(4);
    }

    printf("Creata pipe con piped[0]= %d \n", piped[0]);
	printf("Creata pipe con piped[1]= %d \n", piped[1]);

    exit(0);
}