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
    int i,j;				/* indici per i cicli */
    long int counter;       /* contatore di posizioni nel file */
    int C;                  /* carattere passato come ultimo parametro */
    int fd;                 /* file descriptor del file da aprire per ogni figlio */
    char ch;                /* carattere letto da file */
    char sostituto;         /* carattere che il padre manda al figlio */
    int nsostituzioni;      /* numero di sostituzioni effettuate per ogni figlio */
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
    N = argc - 2;
    
    /* Controllo che l'ultimo parametro sia un singolo carattere */
    if (strlen(argv[argc - 1]) > 1 || !isalpha(argv[argc - 1][0]))
    {
        printf("L'ultimo parametro non e' un singolo carattere come richiesto.\n");
        exit(2);
    }

    C = argv[argc - 1][0];

    /* Creo due vettori di pipe per i due versi di comunicazione */
    pipe_t* pipedFP = (pipe_t*)malloc(N * sizeof(pipe_t));
    pipe_t* pipedPF = (pipe_t*)malloc(N * sizeof(pipe_t));
    if (pipedFP == NULL || pipedPF == NULL)
    {
        printf("Errore nell'allocazione della memoria per la pipe padre-figli\n");
        exit(3);
    }
    
    /* Creo le pipe per consentire la comunicazione tra padre e figli */
    for (i = 0; i < N; i++)
    {
        /* Creazione della pipe */
        if (pipe(pipedFP[i]) < 0 || pipe(pipedPF[i]) < 0)
        {
            /* La creazione della pipe ha fallito, stampo un messaggio d'errore ed esco specificando un valore intero d'errore */
            printf("Errore nel piping.\n");
            exit(4);
        }
    }

    for (i = 0; i < N; i++)
    {
        /* Genero un processo figlio */
        /* Controllo che la fork() abbia successo */
        if ((pid = fork()) < 0)
        {
            /* La fork() ha fallito, dunque stampo un messaggio d'errore e ritorno un valore intero d'errore */
            printf("Errore nella fork.\n");
            exit(5);
        }
        
        /* Se pid == 0, allora la fork() ha avuto successo e possiamo eseguire il codice del figlio */
        if (pid == 0)
        {
            /* Codice del figlio */
            printf("DEBUG-Esecuzione del processo figlio %d\n", getpid());
            
            /* Chiudo i file descriptors non necessari */
            for (j = 0; j < N; j++)
            {
                close(pipedFP[j][0]);
                close(pipedPF[j][1]);
                if (j != i)
                {
                    close(pipedFP[j][1]);
                    close(pipedPF[j][0]);
                }
            }
            
            /* Apro in lettura il file corrispondente all'indice del figlio */
            /* Apertura del file*/
            if ((fd = open(argv[i + 1], O_RDWR)) < 0){
                printf("Errore nell'apertura del file '%s'.\n", argv[i + 1]);
                exit(-1);
            }
            
            /* Lettura di ciascun carattere del file */
            counter = 0;        /* Si considera la prima posizione = 0 */
            nsostituzioni = 0;
            while (read(fd, &ch, 1)){
                /* Cerco occorrenze del carattere C */
                if (ch == C)
                {
                    /* Comunico al padre la posizione in cui ho trovato il carattere */
                    if (write(pipedFP[i][1], &counter, sizeof(counter)) == 0)
                    {
                        printf("Errore: fallita la write nel figlio %d.\n", getpid());
                        exit(-1);
                    }

                    /* Attendo istruzioni dal padre sul da farsi */
                    if (read(pipedPF[i][0], &sostituto, 1) == 0)
                    {
                        printf("Errore: fallita la read nel figlio %d.\n", getpid());
                        exit(-1);
                    }
                    
                    /* Se il carattere ricevuto non e' uno spazio, allora devo sostituire il carattere dal file */
                    if (!isblank(sostituto))
                    {
                        /* Indietreggio di uno spazio nel file */
                        lseek(fd, -1, SEEK_CUR);

                        /* Scrivo il carattere sostitutivo */
                        if (write(fd, &sostituto, 1) == 0)
                        {
                            printf("Errore: fallita la write di sostituzione sul file %s nel figlio %d.\n", argv[i + 1], getpid());
                            exit(-1);
                        }
                        nsostituzioni++;
                    }

                }
                counter++;
            }
            
            exit(nsostituzioni);
        }
    }

    /* Codice del padre */
    
    /* Chiudo i file descriptors non necessari */
    for (i = 0; i < N; i++)
    {
        close(pipedFP[i][1]);
        close(pipedPF[i][0]);
    }
    
    /* Uso una variabile che controlla se c'è ancora almeno un figlio che deve finire di leggere. Se tutti hanno finito continua rimane = false, quindi il ciclo termina */
    bool continua = true;
    while (continua) {
        /* Reset ogni volta della variabile continua */
        continua = false;

        /* Ciclo di lettura */
        for (i = 0; i < N; i++)
        {
            /* Leggo la posizione inviata da ogni figlio */
            if (read(pipedFP[i][0], &counter, sizeof(counter)))
            {
                continua = true;
            }
    
            /* Chiedo all'utente un carattere da sostituire */
            printf("Il figlio di indice %d ha letto il carattere %c in posizione %ld del file %s. Con quale carattere lo si vuole sostituire? [lasciare la linea vuota per non sostituirlo]\n", i, C, counter, argv[i + 1]);
            read(0, &sostituto, 1);
            
            /* Comunico al figlio il carattere */
            if (write(pipedPF[i][1], &sostituto, 1) == 0)
            {
                printf("Errore: fallita la write .\n");
                exit(7);
            }
        }
    }
    
    /* Aspetto i figli */
    int pidFiglio;
    for (i = 0; i < N; i++)
    {
        if ((pidFiglio = wait(&status)) < 0)
        {
            printf("Errore del padre in wait.\n");
            exit(8);
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