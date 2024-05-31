#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>

typedef int pipe_t[2];      /* definizione del TIPO pipe_t come array di 2 interi */

int main(int argc, char** argv)
{
    /* ------------- Variabili locali ------------- */

    int pid;                /* process identifier per le fork() */
    int N;                  /* numero di file passati sulla riga di comando, che corrispondera' al numero di figli */
    pipe_t* piped;          /* array dinamico di pipe descriptors per comunicazioni figli-padre  */
    pipe_t pipednipote;     /* pipe per la comunicazione tra figlio e nipote */
    int i, j;               /* indici per i cicli */
    char numero[11];        /* variabile che contiene il numero di righe letto in caratteri */
    int valore;             /* variabile che contiene il numero di righe in forma numerica */
    long int somma;         /* variabile che contiene la somma di tutti i valori letti */
    int status;			    /* variabile di stato per la wait */
    int ritorno;            /* variabile usata dal padre per recuperare valore di ritorno di ogni figlio */

    /* -------------------------------------------- */

    /* Controllo sul numero di parametri */
    if (argc < 3)
    {
        printf("Numero di parametri errato: argc = %d, ma dovrebbe essere >= 3\n", argc);
        exit(1);
    }

    /* Numero di parametri passati da linea di comando */
    N = argc - 1;

    /* Allocazione dell'array di pipe */
    piped = (pipe_t*) malloc(N * sizeof(pipe_t));
    if (piped == NULL)
    {
        printf("Errore nell'allocazione della memoria per il vettore di pipe.\n");
        exit(2);
    }

    /* Creazione delle N pipe */
    for (i = 0; i < N; i++)
    {
        if (pipe(piped[i]) < 0)
        {
            printf("Errore nel piping.\n");
            exit(3);
        }
    }

    /* Ciclo di generazione dei FIGLI */
    for (i = 0; i < N; i++)
    {
        if ((pid = fork()) < 0)
        {
            printf("Errore nella fork.\n");
            exit(4);
        }

        if (pid == 0)
        {
            /* Codice del FIGLIO */
            printf("DEBUG-Processo filgio %d di indice %d. File associato: %s\n", getpid(), i, argv[i+1]);

            /* Chiusura di tutte le pipe non utilizzate nella comunicazione con il padre */
            for (j = 0; j < N; j++)
            {
                /* Chiudo il lato della pipe non usato dai figli */
                close(piped[j][0]);
                /* Per ogni figlio tengo aperto il lato per la scrittura solo se j == i perche' gli altri canali di comunicazione sono riservati agli altri figli */
                if (j != i)
                {
                    close(piped[j][1]);
                }
            }

            /* Creo una pipe per la comunicazione con il nipote */
            if (pipe(pipednipote) < 0)
            {
                printf("Errore nel piping.\n");
                exit(6);
            }

            /* Creazione del NIPOTE */
            if ((pid = fork()) < 0)
            {
                printf("Errore nella fork.\n");
                exit(5);
            }
            if (pid == 0)
            {
                /* Codice del NIPOTE */

                /* Chiudo il lato di pipe usata dal figlio per comunicare al padre siccome il nipote non la utilizza */
                close(piped[i][1]);

                /* Redirigo stdin sul file associato a questo nipote (tracciato dall'indice i) */
                close(0);
                if (open(argv[i + 1], O_RDONLY) < 0)
                {
                    printf("Errore nell'apertura del file");
                    exit(9);
                }

                /* Ridirigo lo standard output su pipednipote[1] */
                close(1);
                dup(pipednipote[1]);
            
                /* Chiudo il lato di pipe il nipote non utilizza */
                close(pipednipote[0]);
                /* Chiudo il lato di pipe gia' duplicato */
                close(pipednipote[1]);

                /* Per evitare messaggi d'errore ridirigo stderr su /dev/null */
                close(2);
                open("/dev/null", O_WRONLY);

                /* Eseguo il wc -l */
                execlp("wc", "wc", "-l", (char*)0);

                /* Se arrivo qui la exec ha fallito */
                printf("Errore nella exec.\n");
                exit(7);
            }

            /* Codice del figlio */

            /* Chiudo il lato della pipe non utilizzato */
            close(pipednipote[1]);

            /* Devo leggere dalla pipe un carattere alla volta */
            for (j = 0; read(pipednipote[0], &(numero[j]), 1) > 0; j++)
            {
                if (numero[j] != '\n')
                {
                    valore = valore * 10 + atoi(&(numero[j]));
                }
            }

            /* Scrivo il valore su piped[i][1] per comunicarlo al padre */
            write(piped[i][1], &valore, sizeof(valore));

            /* Devo aspettare il nipote per poter restituire il valore al padre */
            ritorno = -1;
            if ((pid = wait(&status)) < 0)
            {
                printf("Errore nella wait del figlio.\n");
            }
            if ((status & 0xFF) != 0)
            {
                printf("Nipote con pid %d terminato in modo anomalo\n", pid);
            }	
    		else
		    { 	
                ritorno = (int)((status >> 8) &	0xFF); 
                printf("DEBUG-Processo nipote %d ha ritornato %d\n", pid, ritorno); 	
		    }

            exit(ritorno);
        }
    }

    /* Codice del padre */
    /* Chiudo i lati di scrittura della pipe non usati dal padre */
    for (j = 0; j < N; j++)
    {
        close(piped[j][1]);
    }

    /* Raccolgo le informazioni dai figli in ordine di indice */
    for (int i = 0; i < N; i++)
    {
        /* Leggo dalla piped[0] il valore scritto dal figlio */
        read(piped[i][0], &valore, sizeof(valore));
        /* Stampo su stdout la lunghezza del file corrispondente */
        printf("Il file %s ha lunghezza pari a %d righe.\n", argv[i + 1], valore);
        somma = somma + (long int)valore;
    }
    printf("La somma risultante di tutti i valori comunicati dai figli e': %ld\n", somma);

    /* Aspetto tutti i figli */
    for (i = 0; i < N; i++)
    {
        if ((pid = wait(&status)) < 0)
        {
            printf("Errore nella wait del padre.\n");
            exit(8);
        }
        if ((status & 0xFF) != 0)
        {
            printf("Figlio con pid %d terminato in modo anomalo\n", pid);
        }	
    	else
	    { 	
            ritorno = (int)((status >> 8) &	0xFF); 
	      	if (ritorno != 0)
            {
                printf("Processo figlio %d ha ritornato %d. Si sono verificati dei problemi: il nipote non è riuscito ad eseguire il wc oppure è terminato in modo anormale\n", pid, ritorno);
            }
	      	else
            {
                printf("Processo figlio %d ha ritornato %d\n", pid, ritorno);
            }  	
	    }
    }

    exit(0);
}