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

typedef int pipe_t[2]; /* definizione del TIPO pipe_t come array di 2 interi */

int main(int argc, char **argv)
{
    /* ------------- Variabili locali ------------- */

    int pid;                 /* process identifier per le fork() */
    int N;                   /* numero di file passati sulla riga di comando */
    int n, j;                /* indici per i cicli */
    pipe_t *piped;           /* pipe tra padre e figli */
    pipe_t pipedFratelli;    /* pipe tra fratelli */
    pipe_t pipedNipote;      /* pipe tra figlio e nipote */
    int fd;                  /* file descriptor per i file passati come argomenti */
    const int MSGSIZE = 255; /* lunghezza delle linee dei file */
    char linea[MSGSIZE];     /* array di char per la stringa letta dal figlio 0 */
    char buffer[MSGSIZE];    /* nome inviato ai fratelli */
    int status;              /* variabile di stato per la wait */
    int ritorno;             /* variabile usata dal padre per recuperare valore di ritorno di ogni figlio */

    /* -------------------------------------------- */

    /* Controllo se il numero di parametri passati da linea di comando è corretto */
    if (argc < 4)
    {
        printf("Numero di parametri errato: argc = %d, ma dovrebbe essere >= 4\n", argc);
        exit(1);
    }

    /* Numero di parametri passati da linea di comando */
    N = argc - 1;

    /* Creo un vettore di pipe */
    piped = (pipe_t *)malloc(N * sizeof(pipe_t));
    if (piped == NULL)
    {
        printf("Errore nell'allocazione della memoria per la pipe padre-figli\n");
        exit(2);
    }

    /* Creo le pipe per consentire la comunicazione tra padre e figli */
    for (n = 0; n < N; n++)
    {
        /* Creazione della pipe */
        if (pipe(piped[n]) < 0)
        {
            /* La creazione della pipe ha fallito, stampo un messaggio d'errore ed esco specificando un valore intero d'errore */
            printf("Errore nel piping.\n");
            exit(3);
        }
    }

    /* Creo la pipe per i fratelli */
    if (pipe(pipedFratelli) < 0)
    {
        /* La creazione della pipe ha fallito, stampo un messaggio d'errore ed esco specificando un valore intero d'errore */
        printf("Errore nel piping.\n");
        exit(5);
    }

    for (n = 0; n < N; n++)
    {
        /* Genero un processo figlio */
        /* Controllo che la fork() abbia successo */
        if ((pid = fork()) < 0)
        {
            /* La fork() ha fallito, dunque stampo un messaggio d'errore e ritorno un valore intero d'errore */
            printf("Errore nella fork.\n");
            exit(6);
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
                if (j != 0)
                {
                    close(pipedFratelli[1]);
                }
                else
                {
                    close(pipedFratelli[0]);
                }
                if (n != j)
                {
                    close(piped[j][1]);
                }
            }

            /* Apertura del file*/
            if ((fd = open(argv[n + 1], O_RDONLY)) < 0)
            {
                printf("Errore nell'apertura del file '%s'.\n", argv[n + 1]);
                exit(-1);
            }

            if (n == 0)
            {
                /* Lettura di ciascun carattere del file linea */
                j = 0;
                while (read(fd, &(linea[j]), 1) != 0)
                {
                    /* Trovato il termine di una linea */
                    if (linea[j] == '\n')
                    {
                        /* Trasformo linea in stringa */
                        linea[j + 1] = '\0';

                        /* Invio la linea letta a tutti i fratelli */
                        for (j = 0; j < N; j++)
                        {
                            if (j != n)
                            {
                                if (write(pipedFratelli[1], linea, sizeof(linea)) == 0)
                                {
                                    printf("Errore: fallita la write .\n");
                                    exit(-1);
                                }
                            }
                        }

                        j = 0;
                    }
                    else
                    {
                        /* Aggiornamento indice */
                        j++;
                    }
                }
            }
            else
            {
                /* Leggo la linea scritta dal fratello 0 */
                if (read(pipedFratelli[0], buffer, sizeof(buffer)) == 0)
                {
                    printf("Errore: fallita la read .\n");
                    exit(-1);
                }

                /* Apertura del file*/
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
                        /* Creo una pipe per consentire la comunicazione tra figlio e nipote */
                        if (pipe(pipedNipote) < 0)
                        {
                            /* La creazione della pipe ha fallito, stampo un messaggio d'errore ed esco specificando un valore intero d'errore */
                            printf("Errore nel piping.\n");
                            exit(-1);
                        }

                        /* Genero un processo nipote */
                        /* Controllo che la fork() abbia successo */
                        if ((pid = fork()) < 0)
                        {
                            /* La fork() ha fallito, dunque stampo un messaggio d'errore e ritorno un valore intero d'errore */
                            printf("Errore nella fork.\n");
                            exit(-1);
                        }

                        /* Se pid == 0, allora la fork() ha avuto successo e possiamo eseguire il codice del nipote */
                        if (pid == 0)
                        {
                            /* Codice del nipote */
                            printf("DEBUG-Esecuzione del processo nipote %d\n", getpid());

                            close(piped[n][1]);
                            close(pipedNipote[0]);

                            /* Ridirigo stdout su pipedNipote[1] */
                            close(1);
                            dup(pipedNipote[1]);
                            
                            /* Chiudo pipedNipote[1] che ho appena duplicato, quindi non serve piu' */
                            close(pipedNipote[1]);

                            /* Per evitare messaggi d'errore ridirigo stderr su /dev/null */
                            close(2);
                            open("/dev/null", O_WRONLY);
                            
                            execlp("diff", "diff", "-q", linea, buffer, (char*)0);

                            exit(0);
                        }

                        /* Codice del figlio */

                        close(pipedNipote[1]);

                        /* Leggo l'output di diff -q */
                        char output[BUFSIZ];
                        if (read(pipedNipote[0], output, sizeof(linea)) == 0)
                        {
                            printf("Errore: fallita la read .\n");
                            exit(-1);
                        }
                        
                        /* Controllo se i file sono uguali o diversi */
                        if (strstr(output, "differ"))
                        {
                            printf("I file %s e %s sono diversi.\n", linea, buffer);
                        }
                        else
                        {
                            printf("I file %s e %s sono uguali.\n", linea, buffer);
                        }

                        /* Aspetto il nipote */
                        int pidNipote;
                        ritorno = -1;
                        if ((pidNipote = wait(&status)) < 0)
                        {
                            printf("Errore del figlio in wait.\n");
                        }
                        if ((status & 0xFF) != 0)
                        {
                            printf("Processo nipote %d terminato in modo anomalo.\n", pidNipote);
                        }
                        else
                        {
                            ritorno = (int)((status >> 8) & 0xFF);
                            printf("Il processo nipote %d ha ritornato %d.\n", pidNipote, ritorno);
                        }

                        j = 0;
                    }
                    else
                    {
                        /* Aggiornamento indice */
                        j++;
                    }
                }
            }

            exit(n);
        }
    }

    /* Codice del padre */

    /* Aspetto i figli */
    int pidFiglio;
    for (n = 0; n < N; n++)
    {
        if ((pidFiglio = wait(&status)) < 0)
        {
            printf("Errore del padre in wait.\n");
            exit(5);
        }
        if ((status & 0xFF) != 0)
        {
            printf("Processo figlio %d terminato in modo anomalo.\n", pidFiglio);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il processo figlio %d ha ritornato %d.\n", pidFiglio, ritorno);
        }
    }

    exit(0);
}
