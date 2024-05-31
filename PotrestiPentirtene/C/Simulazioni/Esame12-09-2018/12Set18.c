#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <ctype.h>

typedef int pipe_t[2];         /* definizione del TIPO pipe_t come array di 2 interi */

int main(int argc, char** argv)
{
    /* ------------- Variabili locali ------------- */
    
    int pid;				/* process identifier per le fork() */
    int N;					/* numero di file passati sulla riga di comando */
    int i,j;				/* indici per i cicli */
    pipe_t* pipedFigli;     /* pipe per la comunicazione tra padre e figli */
    pipe_t* pipedNipoti;    /* pipe per la comunicazione tra padre e nipoti */
    int status;				/* variabile di stato per la wait */
    int ritorno;			/* variabile usata dal padre per recuperare valore di ritorno di ogni figlio */
    
    /* -------------------------------------------- */
    
    if (argc < 3)
    {
        printf("Numero di parametri errato: argc = %d, ma dovrebbe essere >= 3\n", argc);
        exit(1);
    }
    
    /* Numero di parametri passati da linea di comando */
    N = argc - 1;

    pipedFigli = (pipe_t*)malloc(N * sizeof(pipe_t));
    if (pipedFigli == NULL)
    {
        printf("Errore nella allocazione della memoria pipe figli-padre\n");
        exit(3);
    }
    pipedNipoti = (pipe_t*)malloc(N * sizeof(pipe_t));
    if (pipedNipoti == NULL)
    {
        printf("Errore nella allocazione della memoria pipe nipoti-padre\n");
        exit(3);
    }
    
    /* Creo le pipe per consentire la comunicazione tra padre e figli verificando se l'operazione va a buon fine */
    for (i = 0; i < N; i++)
    {
        /* Creazione della pipe */
        if (pipe(pipedFigli[i]) < 0)
        {
            /* La creazione della pipe ha fallito, stampo un messaggio d'errore ed esco specificando un valore intero d'errore */
            printf("Errore nel piping.\n");
            exit(3);
        }
        /* Creazione della pipe */
        if (pipe(pipedNipoti[i]) < 0)
        {
            /* La creazione della pipe ha fallito, stampo un messaggio d'errore ed esco specificando un valore intero d'errore */
            printf("Errore nel piping.\n");
            exit(4);
        }
    }

    /* Creo le N fork */
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
            /* Genero un processo nipote */
            /* Controllo che la fork() abbia successo */
            if ((pid = fork()) < 0)
            {
                /* La fork() ha fallito, dunque stampo un messaggio d'errore e ritorno un valore intero d'errore */
                printf("Errore nella fork.\n");
                exit(6);
            }
            
            /* Se pid == 0, allora la fork() ha avuto successo e possiamo eseguire il codice del nipote */
            if (pid == 0)
            {
                /* Codice del nipote */
                printf("DEBUG-Esecuzione del processo nipote %d\n", getpid());
                
                /* Chiudo i file descriptors non necessari */
                for (j = 0; j < N; j++)
                {
                    close(pipedNipoti[j][0]);
                    if (j != i)
                    {
                        close(pipedNipoti[j][1]);
                    }
                    close(pipedFigli[j][0]);
                    close(pipedFigli[j][1]);
                }

                /* Apertura del file e controllo esistenza */
                int fd;
                if((fd = open(argv[i+1], O_RDWR)) < 0){
                    printf("Errore nell'apertura del file '%s'.\n", argv[i+1]);
                    exit(-1);
                }
                
                /* Cerco le occorrenze numeriche */
                /* Lettura di ciascun carattere del file */
                char ch;
                long int occur = 0;
                while(read(fd, &ch, 1))
                {
                    if (islower(ch))
                    {
                        occur++;
                        /* Eseguo la trasformazione in carattere maiuscolo */
                        ch -= 32;
                        lseek(fd, -1L, SEEK_CUR);
                        write(fd, &ch, 1);
                    }
                }

                /* Scrivo le occorrenze sulla pipe */
                write(pipedFigli[i][1], &occur, sizeof(long int));

                exit(occur / 256);
            }

            /* Codice del figlio */
            printf("DEBUG-Esecuzione del processo figlio %d\n", getpid());
        
            /* Chiudo i file descriptors non necessari */
            for (j = 0; j < N; j++)
            {
                close(pipedFigli[j][0]);
                if (j != i)
                {
                    close(pipedFigli[j][1]);
                }
                close(pipedNipoti[j][0]);
                close(pipedNipoti[j][1]);
            }
        
            /* Apertura del file e controllo esistenza */
            int fd;
            if ((fd = open(argv[i+1], O_RDWR)) < 0){
                printf("Errore nell'apertura del file '%s'.\n", argv[i+1]);
                exit(-1);
            }

            /* Cerco le occorrenze alfabetiche */
            /* Lettura di ciascun carattere del file */
            char ch;
            long int occur = 0;
            while(read(fd, &ch, 1)){
                if (isdigit(ch))
                {
                    occur++;
                    /* Eseguo la trasformazione in spazio */
                    ch = ' ';
                    lseek(fd, -1L, SEEK_CUR);
                    write(fd, &ch, 1);
                }
            }

            /* Scrivo le occorrenze sulla pipe */
            write(pipedFigli[i][1], &occur, sizeof(long int));

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

            ritorno = (int)(occur / 256L);
            exit(ritorno);
        }
    }

    /* Codice del padre */

    /* Chiudo i lati di pipe inutilizzati */
    for ( i = 0; i < N; i++)
    {
        close(pipedFigli[i][1]);
        close(pipedNipoti[i][1]);
    }

    /* Raccolgo i dati dalle pipe e li stampo su stdout seguiti da relative informazioni */
    for (i = 0; i < N; i++)
    {
        long int figlioOccur, nipoteOccur;
        read(pipedFigli[i][0], &figlioOccur, sizeof(long int));
        read(pipedFigli[i][0], &nipoteOccur, sizeof(long int));

        printf("Il figlio di indice %d ha effettuato %ld trasformazioni sul file %s.\n", i, figlioOccur, argv[i + 1]);
        printf("Il nipote di indice %d ha effettuato %ld trasformazioni sul file %s.\n", i, nipoteOccur, argv[i + 1]);
    }
    
    /* Aspetto i figli */
    int pidFiglio;
    for (i = 0; i < N; i++)
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