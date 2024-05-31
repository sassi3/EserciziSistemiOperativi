#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <ctype.h>
#include <signal.h>
#include <stdbool.h>

typedef int pipe_t[2];         /* definizione del TIPO pipe_t come array di 2 interi */

int main(int argc, char** argv)
{
    /* ------------- Variabili locali ------------- */
    
    int pid;				/* process identifier per le fork() */
    int pidPrimo;           /* pid del primo processo */
    int N;					/* numero di file passati sulla riga di comando */
    int n,j;				/* indici per i cicli */
    char linea[200];        /* buffer per la linea letta da ogni figlio */
    char output[10];        /* per la conversione dell'output di wc -l in int */
    int numeroLinee;        /* numero totale delle linee di ogni file */
    int lunghezzaLinea;     /* lunghezza della linea letta da ogni figlio */
    pipe_t primaPipe;       /* pipe singola utilizzata dal figlio 'speciale' */
    pipe_t* piped;          /* vettore di pipe per la comunicazione tra figli e padre */
    int fd;                 /* file descriptor di ogni file aperto dai figli */
    int fcreato;            /* file descriptor per il file temporaneo */
    int pidFiglio;          /* pid del figlio atteso */
    int status;				/* variabile di stato per la wait */
    int ritorno;			/* variabile usata dal padre per recuperare valore di ritorno di ogni figlio */
    
    /* -------------------------------------------- */
    
    /* Controllo se il numero di parametri passati da linea di comando è corretto */
    if (argc < 3)
    {
        printf("Numero di parametri errato: argc = %d, ma dovrebbe essere >= 3\n", argc);
        exit(1);
    }
    
    /* Numero di parametri passati da linea di comando */
    N = argc - 1;

    /* Creazione del file "/tmp/MattiaMassarenti" */
    if ((fcreato = creat("/tmp/MattiaMassarenti", 0644)) < 0)
    {
        printf("Errore nella creazione del file\n");
        exit(9);
    }
    
    /* Creo una pipe per consentire la comunicazione tra padre e figlio 'speciale' */
    if (pipe(primaPipe) < 0)
    {
        /* La creazione della pipe ha fallito, stampo un messaggio d'errore ed esco specificando un valore intero d'errore */
        printf("Errore nel piping.\n");
        exit(2);
    }
    
    /* Genero un processo figlio 'speciale' */
    /* Controllo che la fork() abbia successo */
    if ((pidPrimo = fork()) < 0)
    {
        /* La fork() ha fallito, dunque stampo un messaggio d'errore e ritorno un valore intero d'errore */
        printf("Errore nella fork.\n");
        exit(3);
    }
    
    /* Se pid == 0, allora la fork() ha avuto successo e possiamo eseguire il codice del figlio 'speciale' */
    if (pidPrimo == 0)
    {
        /* Codice del figlio 'speciale' */
        printf("DEBUG-Esecuzione del processo figlio 'speciale' %d\n", getpid());
        
        /* Ridirigo stdin su fd */
        close(0);
        
        /* Apertura del file */
        if ((open(argv[1], O_RDONLY)) < 0)
        {
            printf("Errore nell'apertura del file '%s'.\n", argv[1]);
            exit(-1);
        }

        /* Ridirigo stdout su primaPipe[1] */
        close(1);
        dup(primaPipe[1]);
        
        /* Chiudo il lato di pipe non utilizzato */
        close(primaPipe[0]);
        /* Chiudo primaPipe[1] che ho appena duplicato, quindi non serve piu' */
        close(primaPipe[1]);
        
        /* Eseguo il comando wc per contare il numero di linee del file */
        execlp("wc", "wc", "-l", (char*)0);
        
        /* Codice che viene eseguito solo in caso di fallimento della exec */
        perror("Errore in exec!\n");
        exit(-1);
    }
    
    /* Codice del padre */
    
    /* Chiudo il lato di pipe inutilizzato */
    close(primaPipe[1]);

    /* Leggo l'output di wc -l */
    /* Lettura di ciascun carattere del file */
    j = 0;
    while (read(primaPipe[0], &(output[j]), 1))
    {
        j++;
    }
    /* Controllo se ho letto qualcosa */
    if (j != 0)
    {
        output[j - 1] = '\0';
        /* Converto l'output in int */
        numeroLinee = atoi(output);
        printf("Il padre ha letto che la lunghezza dei file in linee è: %d.\n", numeroLinee);
    }
    else
    {
        printf("Errore nel calcolo del numero di linee.\n");
        exit(5);
    }

    /* Chiudo anche l'altro lato di primaPipe perchè non ho altro da leggere */
    close(primaPipe[0]);

    /* Creo un vettore di pipe */
    piped = (pipe_t*)malloc(N * sizeof(pipe_t));
    if (piped == NULL)
    {
        printf("Errore nell'allocazione della memoria per la pipe padre-figli\n");
        exit(6);
    }
    
    /* Creo le pipe per consentire la comunicazione tra padre e figli */
    for (n = 0; n < N; n++)
    {
        /* Creazione della pipe */
        if (pipe(piped[n]) < 0)
        {
            /* La creazione della pipe ha fallito, stampo un messaggio d'errore ed esco specificando un valore intero d'errore */
            printf("Errore nel piping.\n");
            exit(7);
        }
    }
    
    for (n = 0; n < N; n++)
    {
        /* Genero un processo figlio */
        /* Controllo che la fork() abbia successo */
        if ((pid = fork()) < 0)
        {
            /* La fork() ha fallito, dunque stampo un messaggio d'errore e ritorno un valore intero d'errore */
            printf("Errore nella fork.\n");
            exit(8);
        }
        
        /* Se pid == 0, allora la fork() ha avuto successo e possiamo eseguire il codice del figlio */
        if (pid == 0)
        {
            /* Codice del figlio */
            printf("DEBUG-Esecuzione del processo figlio %d\n", getpid());
            
            /* Chiudo i file descriptors non necessari */
            for (j = 0; j < N; j++)
            {
                close(piped[j][0]);
                if (n != j)
                {
                    close(piped[j][1]);
                }
            }

            /* Apertura del file */
            if ((fd = open(argv[n + 1], O_RDONLY)) < 0)
            {
                printf("Errore nell'apertura del file '%s'.\n", argv[n + 1]);
                exit(-1);
            }
            
            /* Lettura di ciascun carattere del file linea */
            j = 0;
            while (read(fd, &(linea[j]), 1) != 0)
            {
                /* Trovato il termine di una linea */
                if (linea[j] == '\n')
                {
                    /* Assegno la lunghezza di tale linea alla variabile corrispondente */
                    lunghezzaLinea = j + 1;

                    /* Invio al padre la lunghezza della linea */
                    if (write(piped[n][1], &lunghezzaLinea, sizeof(lunghezzaLinea)) == 0)
                    {
                        printf("Errore: fallita la write della lunghezza della linea.\n");
                        exit(-1);
                    }

                    /* Scrivo la linea letta sulla pipe per passarla al padre (di dimensione j + 1 in caso avessi scritto linee più lunghe prima) */
                    if (write(piped[n][1], linea, lunghezzaLinea) == 0)
                    {
                        printf("Errore: fallita la write del figlio di indice %d sulla pipe.\n", n);
                        exit(-1);
                    }
                    
                    j = 0;
                }
                else
                {
                    /* Aggiornamento indice */
                    j++;
                }
            }
            
            exit(lunghezzaLinea);
        }
    }

    /* Codice del padre */
        
    /* Chiudo i file descriptors non necessari */
    for (n = 0; n < N; n++)
    {
        close(piped[n][1]);
    }
    
    /* Scorro le linee in un ciclo */
    for (j = 0; j < numeroLinee; j++)
    {
        /* Leggo una linea per volta da ogni figlio */
        for (n = 0; n < N; n++)
        {
            /* Leggo la lunghezza della linea dalla pipe  */
            if (read(piped[n][0], &lunghezzaLinea, sizeof(lunghezzaLinea)) == 0)
            {
                printf("Errore: fallita la read della lunghezza della linea.\n");
                exit(10);
            }
            
            /* Leggo la linea dalla pipe */
            if (read(piped[n][0], linea, lunghezzaLinea) == 0)
            {
                printf("Errore: fallita la read della linea %d dal figlio di indice %d.\n", j, n);
                exit(11);
            }
            
            /* Scrivo la linea sul file temporaneo */
            if (write(fcreato, linea, lunghezzaLinea) == 0)
            {
                printf("Errore: fallita la write .\n");
                exit(12);
            }
        }
    }

    /* Aspetto i figli */
    for (n = 0; n < N + 1; n++)
    {
        if ((pidFiglio = wait(&status)) < 0)
        {
            printf("Errore del padre in wait.\n");
            exit(13);
        }
        if ((status & 0xFF) != 0)
        {
            printf("Processo figlio %d terminato in modo anomalo.\n", pidFiglio);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            if (pid == pidPrimo)
            {
                printf("Il processo figlio 'speciale' %d ha ritornato %d.\n", pidFiglio, ritorno);
            }
            else
            {
                printf("Il processo figlio %d ha ritornato %d.\n", pidFiglio, ritorno);
            }
        }
    }
    
    exit(0);
}