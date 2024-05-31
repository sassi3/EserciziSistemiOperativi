#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <ctype.h>
#include <time.h>

typedef int pipe_t[2]; /* definizione del TIPO pipe_t come array di 2 interi */

int main(int argc, char *argv[])
{

    /* ------------- Variabili locali ------------- */

    int pid;  /* process identifier per le fork() */
    int N;    /* numero di file passati sulla riga di comando */
    int i, j; /* indici per i cicli */
    int random;

    int status;  /* variabile di stato per la wait */
    int ritorno; /* variabile usata dal padre per recuperare valore di ritorno di ogni figlio */
    char *endpointer;
    pipe_t pipedFP;
    int fd;
    char bestemmia[BUFSIZ];
    char **bestemmiolista;
    pipe_t contabestemmie;
    int totBestemmie;
    char totBestemmieStringa[255];
    /* -------------------------------------------- */

    if (argc > 2)
    {
        printf("Dio porco, Madonna vaca, hai cannato....\n");
        exit(1);
    }
    srand(time(NULL));
    if (argc == 2)
    {
        N = strtod(argv[1], &endpointer);
        if (*endpointer != 0)
        {
            printf("Ma Dio nimel, sai come funziona un bestemmiatore?");
            exit(2);
        }
    }
    else{
        N=1;
    }

    bestemmiolista = malloc(N * sizeof(char *));

    /* Creazione della pipe */
    if (pipe(pipedFP) < 0)
    {
        /* La creazione della pipe ha fallito, stampo un messaggio d'errore ed esco specificando un valore intero d'errore */
        printf("Errore nel piping.\n");
        exit(4);
    }

    if (pipe(contabestemmie) < 0)
    {
        printf("dio sporco\n");
        exit(36);
    }

    /* Processo conta bestemmie */
    /* Genero un processo figlio */
    /* Controllo che la fork() abbia successo */
    if ((pid = fork()) < 0)
    {
        /* La fork() ha fallito, dunque stampo un messaggio d'errore e ritorno un valore intero d'errore */
        printf("Errore nella fork.\n");
        exit(35);
    }

    /* Se pid == 0, allora la fork() ha avuto successo e possiamo eseguire il codice del figlio */
    if (pid == 0)
    {
        /* Codice del figlio */
        close(contabestemmie[0]);

        close(1);
        dup(contabestemmie[1]);

        close(0);
        if ((fd = open("bestemmiolista.txt", O_RDONLY)) < 0)
        {
            printf("Dio can, bestemmiolista non raggingibile\n");
            exit(-1);
        }
        execlp("wc", "wc", "-l", (char *)0);

        exit(0);
    }

    /* Codice del padre */
    close(contabestemmie[1]);
    read(contabestemmie[0], totBestemmieStringa, sizeof(totBestemmieStringa));
    totBestemmie = atoi(totBestemmieStringa);
    printf("totale bestemmie nella bestemmiolista: %d\n", totBestemmie);

    /* Aspetto il figlio */
    int pidFiglio;
    if ((pidFiglio = wait(&status)) < 0)
    {
        printf("Errore del padre in wait.\n");
        exit(37);
    }
    if ((status & 0xFF) != 0)
    {
        printf("Processo contabestemmie (pid = %d)terminato in modo anomalo.\n", pid);
    }
    else
    {
        ritorno = (int)((status >> 8) & 0xFF);
        printf("Il processo contabestemmie %d e' terminato con successo, ha ritornato %d.\n", pidFiglio, ritorno);
    }

    if ((pid = fork()) < 0)
    {
        /* La fork() ha fallito, dunque stampo un messaggio d'errore e ritorno un valore intero d'errore */
        printf("Errore nella fork.\n");
        exit(3);
    }
    /* Se pid == 0, allora la fork() ha avuto successo e possiamo eseguire il codice del figlio */
    if (pid == 0)
    {
        /* Codice del figlio */
        if ((fd = open("bestemmiolista.txt", O_RDONLY)) == 0)
        {
            printf("Porco Dio, sulla fiducia\n");
            exit(33);
        }
        close(pipedFP[0]);
        /* Lettura di ciascun carattere del file linea del file */
        int j = 0;
        int counter;
        while (read(fd, &(bestemmia[j]), 1) != 0)
        {
            /* Trovato il termine di una linea */
            if (bestemmia[j] == '\n')
            {
                bestemmia[j] = 0;
                j++;
                write(pipedFP[1], &j, sizeof(j));
                write(pipedFP[1], bestemmia, j);
                counter++;
                j = 0;
            }
            else
            {
                /* Aggiornamento indice */
                j++;
            }
        }
        exit(counter);
    }

    close(pipedFP[1]);
    bestemmiolista = malloc(totBestemmie * sizeof(char *));

    for (i = 0; i < totBestemmie; ++i)
    {
        read(pipedFP[0], &j, sizeof(j));
        bestemmiolista[i] = malloc(j);
        read(pipedFP[0], bestemmiolista[i], j);
    }

    /* Aspetto il figlio */
    if ((pidFiglio = wait(&status)) < 0)
    {
        printf("Errore del padre in wait.\n");
        exit(41);
    }
    if ((status & 0xFF) != 0)
    {
        printf("Processo bestemmiatore %d terminato in modo anomalo.\n", pid);
    }
    else
    {
        ritorno = (int)((status >> 8) & 0xFF);
        printf("Il processo bestemmiatore %d e' terminato con successo ,ha ritornato %d.\n", pidFiglio, ritorno);
    }

    for (i = 0; i < N; ++i)
    {
        random = rand() % totBestemmie;
        printf("%s\n", bestemmiolista[random]);
    }

    exit(0);
}