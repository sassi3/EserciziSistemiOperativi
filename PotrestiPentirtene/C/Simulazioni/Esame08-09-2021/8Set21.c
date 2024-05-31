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
    int N;					/* numero di file passati sulla riga di comando */
    int n,j;				/* indici per i cicli */
    int i;                  /* Indice di linea del file */
    const int nro = 255;    /* massima lunghezza della linea */
    char linea[nro];        /* per la linea corrente letta dai figli dal proprio file */
    pipe_t* piped;          /* pipe per la comunicazione tra padre e figli */
    int fcreato;            /* file descriptor del file creato dal padre */
    int fd;                 /* file descriptor del file aperto dal figlio */
    int lunghezzaRiga;      /* lunghezza della linea letta */
    int maxLungh = -1;       /* massima lunghezza letta */
    int status;				/* variabile di stato per la wait */
    int ritorno;			/* variabile usata dal padre per recuperare valore di ritorno di ogni figlio */
    
    /* -------------------------------------------- */
    
    /* Controllo se il numero di parametri passati da linea di comando è corretto */
    if (argc < 2)
    {
        printf("Numero di parametri errato: argc = %d, ma dovrebbe essere >= 2\n", argc);
        exit(1);
    }
    
    /* Numero di parametri passati da linea di comando */
    N = argc - 1;
    
    /* Creazione del file "massarenti.log" */
    if ((fcreato = open("massarenti.log", O_CREAT | O_RDWR | O_TRUNC)) < 0)
    {
        printf("Errore nella creazione del file\n");
        exit(2);
    }

    /* Creo un vettore di pipe */
    piped = (pipe_t*)malloc((2 * N) * sizeof(pipe_t));
    if (piped == NULL)
    {
        printf("Errore nell'allocazione della memoria per la pipe padre-figli\n");
        exit(3);
    }
    
    /* Creo le pipe per consentire la comunicazione tra padre e figli */
    for (n = 0; n < (2 * N); n++)
    {
        /* Creazione della pipe */
        if (pipe(piped[n]) < 0)
        {
            /* La creazione della pipe ha fallito, stampo un messaggio d'errore ed esco specificando un valore intero d'errore */
            printf("Errore nel piping.\n");
            exit(4);
        }
    }
    
    /* Genero 2N figli */
    for (n = 0; n < (2 * N); n++)
    {
        if ((n % 2) == 0)
        {
            /* Genero un processo figlio PARI */
            /* Controllo che la fork() abbia successo */
            if ((pid = fork()) < 0)
            {
                /* La fork() ha fallito, dunque stampo un messaggio d'errore e ritorno un valore intero d'errore */
                printf("Errore nella fork.\n");
                exit(5);
            }
            
            /* Se pid == 0, allora la fork() ha avuto successo e possiamo eseguire il codice del figlio pari */
            if (pid == 0)
            {
                /* Codice del figlio pari */
                printf("DEBUG-Esecuzione del processo figlio pari %d\n", getpid());

                /* Chiudo i file descriptors non necessari */
                for (j = 0; j < (2 * N); j++)
                {
                    close(piped[j][0]);
                    if (j != n)
                    {
                        close(piped[j][1]);
                    }
                }
                
                /* Apertura del file */
                if ((fd = open(argv[(n / 2) + 1], O_RDONLY)) < 0)
                {
                    printf("Errore nell'apertura del file '%s'.\n", argv[(n / 2) + 1]);
                    exit(-1);
                }
                
                /* Lettura di ciascun carattere del file linea */
                j = 0;
                i = 1;
                while (read(fd, &(linea[j]), 1) != 0)
                {
                    /* Trovato il termine di una linea */
                    if (linea[j] == '\n')
                    {
                        if ((i % 2) == 0)
                        {
                            lunghezzaRiga = j + 1;
                            if (write(piped[n][1], &lunghezzaRiga, sizeof(lunghezzaRiga)) == 0)
                            {
                                printf("Errore: fallita la write della lunghezza della riga sulla pipe nel figlio pari.\n");
                                exit(-1);
                            }
                            
                            if (maxLungh < lunghezzaRiga)
                            {
                                maxLungh = lunghezzaRiga;
                            }
                        }
                        
                        j = 0;
                        i++;
                    }
                    else
                    {
                        /* Aggiornamento indice */
                        j++;
                    }
                }
                
                exit(maxLungh);
            }
        }
        else
        {
            /* Genero un processo figlio DISPARI */
            /* Controllo che la fork() abbia successo */
            if ((pid = fork()) < 0)
            {
                /* La fork() ha fallito, dunque stampo un messaggio d'errore e ritorno un valore intero d'errore */
                printf("Errore nella fork.\n");
                exit(5);
            }
            
            /* Se pid == 0, allora la fork() ha avuto successo e possiamo eseguire il codice del figlio dispari */
            if (pid == 0)
            {
                /* Codice del figlio dispari */
                printf("DEBUG-Esecuzione del processo figlio dispari %d\n", getpid());
                
                /* Chiudo i file descriptors non necessari */
                for (j = 0; j < (2 * N); j++)
                {
                    close(piped[j][0]);
                    if (j != n)
                    {
                        close(piped[j][1]);
                    }
                }

                /* Apertura del file */
                if ((fd = open(argv[(n + 1) / 2], O_RDONLY)) < 0)
                {
                    printf("Errore nell'apertura del file '%s'.\n", argv[(n + 1) / 2]);
                    exit(-1);
                }
                
                /* Lettura di ciascun carattere del file linea */
                j = 0;
                i = 1;
                while (read(fd, &(linea[j]), 1) != 0)
                {
                    /* Trovato il termine di una linea */
                    if (linea[j] == '\n')
                    {
                        if ((i % 2) == 1)
                        {
                            lunghezzaRiga = j + 1;
                            if (write(piped[n][1], &lunghezzaRiga, sizeof(lunghezzaRiga)) == 0)
                            {
                                printf("Errore: fallita la write della lunghezza della riga sulla pipe nel figlio pari.\n");
                                exit(-1);
                            }
                            
                            if (maxLungh < lunghezzaRiga)
                            {
                                maxLungh = lunghezzaRiga;
                            }
                        }
                        
                        j = 0;
                        i++;
                    }
                    else
                    {
                        /* Aggiornamento indice */
                        j++;
                    }
                }
                
                exit(maxLungh);
            }
        }
    }

    /* Codice del padre */

    /* Chiudo i file descriptors non necessari */
    for (n = 0; n < (2 * N); n++)
    {
        close(piped[n][1]);
    }

    int lunghezzaPari, lunghezzaDispari;
    /* Leggo ciò che i figli scrivono sulla pipe */
    for (n = 0; n < (N * 2); n = n + 2)
    {
        printf("Lettura delle line del file %s:\n", argv[(n / 2) + 1]);
        j = 1;
        while (1)
        {
            if (read(piped[n + 1][0], &lunghezzaDispari, sizeof(lunghezzaDispari)))
            {
                printf("Letta la riga numero %d di lunghezza %d.\n", j, lunghezzaDispari);
                j++;
            }
            else
            {
                break;
            }
            if (read(piped[n][0], &lunghezzaPari, sizeof(lunghezzaPari)))
            {
                printf("Letta la riga numero %d di lunghezza %d.\n", j, lunghezzaPari);
                j++;
            }
            else
            {
                break;
            }
        }
    }

    /* Aspetto i figli */
    int pidFiglio;
    for (n = 0; n < (2 * N); n++)
    {
        if ((pidFiglio = wait(&status)) < 0)
        {
            printf("Errore del padre in wait.\n");
            exit(6);
        }
        if ((status & 0xFF) != 0)
        {
            printf("Processo figlio %d terminato in modo anomalo.\n", pidFiglio);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            sprintf(linea, "Il processo figlio %d ha ritornato %d.\n", pidFiglio, ritorno);
            write(fcreato, linea, strlen(linea));
        }
    }
    
    exit(0);
}