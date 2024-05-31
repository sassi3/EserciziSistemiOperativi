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

typedef struct {
    int c1;
    char c2[250];
    int c3;
} Strut;

int main(int argc, char** argv)
{
    /* ------------- Variabili locali ------------- */
    
    int pid;				/* process identifier per le fork() */
    int N;					/* numero di file passati sulla riga di comando */
    int j,n;				/* indici per i cicli */
    Strut datiLetti;        /* struttura dati per raccogliere le informazioni */
    pipe_t* piped;          /* vettore di pipe per la comunicazione tra padre e figli */
    pipe_t pipedNipote;     /* pipe per la comunicazione tra figlio e nipote */
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
    
    /* Creo un vettore di pipe */
    piped = (pipe_t*)malloc(N * sizeof(pipe_t));
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
    
    for (n = 0; n < N; n++)
    {
        /* Genero un processo figlio */
        /* Controllo che la fork() abbia successo */
        if ((pid = fork()) < 0)
        {
            /* La fork() ha fallito, dunque stampo un messaggio d'errore e ritorno un valore intero d'errore */
            printf("Errore nella fork.\n");
            exit(4);
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

                /* Eseguo il comando exec */
                execlp("rev", "rev", argv[n + 1], (char*)0);
                
                /* Codice che viene eseguito solo in caso di fallimento della exec */
                printf("Errore in exec!\n");
                exit(-1);
            }
            
            /* Codice del figlio */
            
            close(pipedNipote[1]);

            /* Salvo il pid del nipote */
            datiLetti.c1 = pid;

            /* Lettura di ciascun carattere del file linea */
            j = 0;
            while (read(pipedNipote[0], &(datiLetti.c2[j]), 1) != 0)
            {
                /* Trovato il termine di una linea */
                if (datiLetti.c2[j] == '\n')
                {
                    datiLetti.c3 = strlen(datiLetti.c2);
                    datiLetti.c2[j + 1] = '\0';
                    j = 0;
                }
                else
                {
                    /* Aggiornamento indice */
                    j++;
                }
            }
            
            /* Scrivo i dati letti sulla pipe */
            write(piped[n][1], &datiLetti, sizeof(datiLetti));

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
            
            exit(datiLetti.c3 - 1);
        }
    }

    /* Codice del padre */

    /* Chiudo i file descriptors non necessari */
    for (j = 0; j < N; j++)
    {
        close(piped[j][1]);
    }
    
    for (n = 0; n < N; n++)
    {
        /* Leggo la struttura dati passata da ciascun figlio */
        if (read(piped[n][0], &datiLetti, sizeof(datiLetti)) == 0)
        {
            printf("Errore: fallita la read della struttura di dati.\n");
            exit(5);
        }
        
        printf("Processo nipote %d di indice %d ha letto: \n", datiLetti.c1, n);
        printf("%s", datiLetti.c2);
        printf("Lunghezza della linea: %d\n", datiLetti.c3);
    }

    /* Aspetto i figli */
    int pidFiglio;
    for (n = 0; n < N; n++)
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
            printf("Il processo figlio %d ha ritornato %d.\n", pidFiglio, ritorno);
        }
    }
    
    exit(0);
}