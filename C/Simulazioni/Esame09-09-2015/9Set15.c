#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#include <signal.h>
#include <stdbool.h>

typedef int pipe_t[2];         /* definizione del TIPO pipe_t come array di 2 interi */

/* Dichiarata all'esterno per ragioni di visibilità */
int i;

/* Definisco un handler */
void handler(int signo)
{
    printf("DEBUG-Il processo figlio %d di indice %d ha ricevuto il segnale %d. Terminazione forzata, ma normale.\n", getpid(), i, signo);
    exit(0);
}

int main(int argc, char** argv)
{
    /* ------------- Variabili locali ------------- */
    
    int N;					/* numero di file passati sulla riga di comando */
    int j;				    /* indici per i cicli */
    int AF;                 /* file descriptor del file associato al padre */
    char cF;                /* carattere letto dal figlio */
    char cP;                /* carattere letto dal padre */
    int fd;                 /* file descriptor dei file associati ai figli */
    char sig = 'v';         /* istruzione inviata tramite la pipe sinc */
    pipe_t* piped;          /* vettore di pipe per la comunicazione dai figli al padre */
    int* pid;               /* vettore di pid necessario per far sì che il padre possa inviare SIGKILL al figlio corretto */
    pipe_t* sinc;           /* vettore di pipe per consentire al padre di comunicare ai figli il permesso di proseguire */
    bool* bloccati;         /* vettore che ricorda quali figli non possono più leggere caratteri */
    int status;				/* variabile di stato per la wait */
    int ritorno;			/* variabile usata dal padre per recuperare valore di ritorno di ogni figlio */
    
    /* -------------------------------------------- */
    
    /* Controllo se il numero di parametri passati da linea di comando è corretto */
    if (argc < 4)
    {
        printf("Numero di parametri errato: argc = %d, ma dovrebbe essere >= 4\n", argc);
        exit(1);
    }
    
    /* Numero di parametri passati da linea di comando */
    N = argc - 2;
    
    /* Creo i vettori di pipe */
    piped = (pipe_t*)malloc(N * sizeof(pipe_t));
    sinc = (pipe_t*)malloc(N * sizeof(pipe_t));
    if (piped == NULL || sinc == NULL)
    {
        printf("Errore nell'allocazione della memoria per le pipe padre-figli\n");
        exit(2);
    }
    
    /* Creo le pipe per consentire la comunicazione tra padre e figli */
    for (i = 0; i < N; i++)
    {
        /* Creazione della pipe piped */
        if (pipe(piped[i]) < 0)
        {
            /* La creazione della pipe ha fallito, stampo un messaggio d'errore ed esco specificando un valore intero d'errore */
            printf("Errore nel piping.\n");
            exit(3);
        }
        /* Creazione della pipe sinc */
        if (pipe(sinc[i]) < 0)
        {
            /* La creazione della pipe ha fallito, stampo un messaggio d'errore ed esco specificando un valore intero d'errore */
            printf("Errore nel piping.\n");
            exit(4);
        }
    }

    /* Creo il vettore di pid */
    pid = (int*)malloc(N * sizeof(int));
    if (pid == NULL)
    {
        printf("Errore nell'allocazione della memoria per il vettore di pid.\n");
        exit(5);
    }

    /* Creo il vettore di controllo letture */
    bloccati = (bool*)calloc(N, sizeof(bool));
    if (bloccati == NULL)
    {
        printf("Errore nell'allocazione della memoria per il vettore di controllo letture.\n");
        exit(6);
    }

    /* Installo l'handler handler per gestire il segnale SIGUSR1 */
    signal(SIGUSR1, handler);
    
    for (i = 0; i < N; i++)
    {
        /* Genero un processo figlio */
        /* Controllo che la fork() abbia successo */
        if ((pid[i] = fork()) < 0)
        {
            /* La fork() ha fallito, dunque stampo un messaggio d'errore e ritorno un valore intero d'errore */
            printf("Errore nella fork.\n");
            exit(7);
        }
        
        /* Se pid == 0, allora la fork() ha avuto successo e possiamo eseguire il codice del figlio */
        if (pid[i] == 0)
        {
            /* Codice del figlio */
            printf("DEBUG-Esecuzione del processo figlio %d\n", getpid());
            
            /* Chiudo i file descriptors non necessari */
            for (j = 0; j < N; j++)
            {
                close(piped[j][0]);
                close(sinc[j][1]);
                if (j != i)
                {
                    close(piped[j][1]);
                    close(sinc[j][0]);
                }
            }
            
            /* Apro il file da cui devo leggere i caratteri */
            /* Apertura del file */
            if ((fd = open(argv[i + 1], O_RDONLY)) < 0)
            {
                printf("Errore nell'apertura del file '%s'.\n", argv[i + 1]);
                exit(-1);
            }
            
            /* Lettura di ciascun carattere del file */
            while (read(fd, &cF, 1))
            {
                /* Scrivo il primo carattere letto sulla pipe */
                write(piped[i][1], &cF, 1);
                
                /* Attendo il via dal padre tramite la pipe di sincronizzazione */
                read(sinc[i][0], &sig, 1);
            }
            
            exit(0);
        }
    }

    /* Codice del padre */
    /* Chiudo i file descriptors non necessari */
    for (i = 0; i < N; i++)
    {
        close(piped[i][1]);
        close(sinc[i][0]);
    }
    
    /* Apro il file associato al padre */
    /* Apertura del file */
    if ((AF = open(argv[argc - 1], O_RDONLY)) < 0)
    {
        printf("Errore nell'apertura del file '%s'.\n", argv[argc - 1]);
        exit(8);
    }
    
    /* Leggo i caratteri inviati dai figli */
    while (read(AF, &cP, sizeof(cP)))
    {
        /* Leggo che cosa i figli hanno scritto */
        for (i = 0; i < N; i++)
        {
            /* Scrivo al figlio di procedere e leggo dalla sua pipe solo se non è bloccato */
            if (!bloccati[i])
            {
                /* Comunico al figlio di procedere e leggo il carattere che mi comunica */
                write(sinc[i][1], &sig, 1);
                read(piped[i][0], &cF, 1);

                /* Se i due caratteri sono diversi, allora scrivo al figlio di terminare la lettura e lo segnalo come bloccato */
                if (cF != cP)
                {
                    bloccati[i] = true;
                    /* Non scrivo niente al figlio cosicché rimanga in attesa fino a quando verrà ucciso dalla kill */
                }
            }
        }
    }

    /* Termino forzatamente eventuali figli che avevo bloccato */
    for (i = 0; i < N; i++)
    {
        /* Se il confronto è fallito, termino forzatamente il figlio */
        if (bloccati[i])
        {
            if (kill(pid[i], SIGKILL) == -1)
            {
                printf("Il figlio di pid %d non esiste, quindi e' gia' terminato.\n", pid[i]);
            }
        }
        else
        {
            /* Se non è fallito, termino il figlio facendolo uscire dal ciclo, dunque evitando un deadlock */
            kill(pid[i], SIGUSR1);
        }
    }

    /* Aspetto i figli */
    int pidFiglio;
    for (i = 0; i < N; i++)
    {
        if ((pidFiglio = wait(&status)) < 0)
        {
            printf("Errore del padre in wait.\n");
            exit(9);
        }
        if ((status & 0xFF) != 0)
        {
            printf("Processo figlio %d terminato in modo anomalo.\n", pidFiglio);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il processo figlio %d ha ritornato %d.\n", pidFiglio, ritorno);
            for (j = 0; j < N; j++)
            {
                if (pidFiglio == pid[j])
                {
                    printf("Il processo figlio %d di indice %d e' terminato con successo, quindi i file %s e %s sono uguali.\n", pid[j], j, argv[argc - 1], argv[j + 1]);
                }
            }
        }
    }
    
    exit(0);
}