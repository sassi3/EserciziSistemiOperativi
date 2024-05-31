#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <ctype.h>
#include <time.h>

int mia_random(int n)
{
    int casuale;
    casuale = rand() % n;
    return casuale;
}

typedef int pipe_t[2];         /* definizione del TIPO pipe_t come array di 2 interi */

int main(int argc, char** argv)
{
    /* ------------- Variabili locali ------------- */
    
    int pid;				/* process identifier per le fork() */
    int N;					/* numero di file passati sulla riga di comando */
    int H;                  /* numero intero positivo */
    int i,j;				/* indici per i cicli */
    int tmp;                /* fd per /tmp/creato */
    pipe_t* pipedFP;        /* pipe per comunicazione figlio -> padre */
    pipe_t* pipedPF;        /* pipe per comunicazione padre -> figlio */
    int fd;                 /* fd per i file da cui leggere */
    char linea[255];        /* linea letta */
    int lunghezzaLinea;     /* lunghezza della linea letta */
    int rndNum;             /* numero random generato */
    int status;				/* variabile di stato per la wait */
    int ritorno = 0;		/* variabile usata dal padre per recuperare valore di ritorno di ogni figlio */
    
    /* -------------------------------------------- */
    
    if (argc < 6)
    {
        printf("Numero di parametri errato: argc = %d, ma dovrebbe essere >= 6\n", argc);
        exit(1);
    }
    
    /* Numero di parametri passati da linea di comando */
    N = argc - 2;

    /* Assegno il numero trovato e controllo che sia minore di 255 */
    H = atoi(argv[argc - 1]);
    if ((H <= 0) || (H >= 255))
    {
        printf("Errore: %d e' maggiore o uguale a 255.\n", H);
        exit(2);
    }

    /* Inizializzazione del randomizer */
    srand(time(NULL));

    /* Creazione del file /tmp/creato */
    if ((tmp = open("/tmp/creato", O_CREAT | O_WRONLY | O_TRUNC, 0644)) < 0)
    {
        printf("Errore nella creazione di /tmp/creato.\n");
        exit(3);
    }
    
    /* Creo due vettori di pipe */
    pipedFP = (pipe_t*)malloc(N * sizeof(pipe_t));
    pipedPF = (pipe_t*)malloc(N * sizeof(pipe_t));
    if ((pipedFP == NULL) || (pipedPF == NULL))
    {
        printf("Errore nell'allocazione della memoria per la pipe padre-figli.\n");
        exit(4);
    }

    /* Creo le pipe per consentire la comunicazione tra padre e figli */
    for (i = 0; i < N; i++)
    {
        /* Creazione della pipeFP */
        if (pipe(pipedFP[i]) < 0)
        {
            /* La creazione della pipe ha fallito, stampo un messaggio d'errore ed esco specificando un valore intero d'errore */
            printf("Errore nel piping.\n");
            exit(5);
        }
        /* Creazione della pipePF */
        if (pipe(pipedPF[i]) < 0)
        {
            /* La creazione della pipe ha fallito, stampo un messaggio d'errore ed esco specificando un valore intero d'errore */
            printf("Errore nel piping.\n");
            exit(6);
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
            exit(7);
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

            /* Apertura del file*/
            if ((fd = open(argv[i+1], O_RDONLY)) < 0){
                printf("Errore nell'apertura del file '%s'.\n", argv[i+1]);
                exit(-1);
            }
            
            /* Lettura di ciascun carattere del file linea */
            j = 0;
            while(read(fd, &(linea[j]), 1) != 0)
            {
                /* Trovato il termine di una linea */
                if (linea[j] == '\n')
                {
                    /* Comprendo il terminatore \n */
                    lunghezzaLinea = j + 1;
                    /* Scrivo la lunghezza della linea sulla pipe */
                    write(pipedFP[i][1], &lunghezzaLinea, sizeof(lunghezzaLinea));

                    /* Leggo il numero random */
                    read(pipedPF[i][0], &rndNum, sizeof(rndNum));

                    if (rndNum < lunghezzaLinea)
                    {
                        write(tmp, &(linea[rndNum]), 1);
                        ritorno++;
                    }
                    
                    /* Riparto da 0 per ricominciare a contare la linea successiva */
                    j = 0;
                }
                else
                {
                    /* Aggiornamento indice */
                    j++;
                }
            }
            
            exit(ritorno);
        }
    }

    /* Codice del padre */

    /* Chiudo i file descriptors non necessari */
    for (i = 0; i < N; i++)
    {
        close(pipedFP[i][1]);
        close(pipedPF[i][0]);
    }         

    int lunghezzaLetta;
    int valoreCorretto;
    for (i = 1; i <= H; i ++)
    {
        /* Calcolo un indice per scegliere una lunghezza a caso tra quelle lette */
        rndNum = mia_random(N);

        /* Leggo la lunghezza dal figlio d'indice rndNum*/
        for (j = 0; j < N; j++)
        {
            read(pipedFP[j][0], &lunghezzaLetta, sizeof(int));
            if (i == rndNum)
            {
                /* Ho letto la lunghezza che corrisponde all'indice randomico calcolato, quindi interrompo il ciclo */
                valoreCorretto = lunghezzaLetta;
            }
        }

        /* Calcolo l'indice random da mandare ai figli */
        rndNum = mia_random(valoreCorretto);

        for (j = 0; j < N; j++)
        {
            write(pipedPF[j][1], &rndNum, sizeof(int));
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