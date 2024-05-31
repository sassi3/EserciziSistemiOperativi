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
#include <sys/types.h>

typedef int pipe_t[2];         /* definizione del TIPO pipe_t come array di 2 interi */

int main(int argc, char** argv)
{
    /* ------------- Variabili locali ------------- */
    
    int pid;				    /* process identifier per le fork() */
    int W;					    /* numero intero strettamente positivo passato come primo parametro */
    int w;				        /* indice per i processi figli */
    int i;                      /* indici per i cicli */
    int fcreato;                /* file descriptor usato dal padre per la creazione del file passato come secondo parametro */
    char buff[250];             /* variabile usata dal padre per leggere le singole linee inviate dai figli */
    char opzione[20];           /* array di char che contiene l'opzione (contenente il pid del padre) da passare alla exec del figlio */
    pipe_t* pipedFigli;         /* vettore di pipe per la comunicazione tra figli e padre */
    pipe_t pipedNipote;         /* singola pipe per la comunicazione tra un figlio e il proprio nipote */
    int pidFiglio;              /* variabile per la raccolta del valore di ritorno della wait (ovvero del pid del figlio che ha appena terminato) effettuata dal padre */
    int status;				    /* variabile di stato per la wait */
    int ritorno;			    /* variabile usata dal padre per recuperare il valore di ritorno di ogni figlio */
    
    /* -------------------------------------------- */
    
    /* CONTROLLO se il numero di parametri passati da linea di comando è corretto (deve essere esattamente pari a 3) */
    if (argc != 3)
    {
        printf("Numero di parametri errato: argc = %d, ma dovrebbe essere == 3\n", argc);
        exit(1);
    }

    /* CONTROLLO che il primo parametro sia un numero intero strettamente positivo */
    if ((W = atoi(argv[1])) <= 0)
    {
        printf("Il primo parametro non e' un numero intero strettamente positivo.\n");
        exit(2);
    }

    printf("DEBUG-Esecuzione del processo padre %d\n", getpid());

    /* Per prima cosa creo nella directory corrente un file con nome corrispondente al secondo parametro (se esiste gia' viene sovrascritto) */
    if ((fcreato = creat(argv[2], 0644)) < 0)
    {
        printf("Errore nella creazione del file %s passato come secondo parametro.\n", argv[2]);
        exit(3);
    }

    /* Creo un vettore di pipe */
    pipedFigli = (pipe_t*)malloc(W * sizeof(pipe_t));
    /* Controllo che l'allocazione della memoria sia stata effettuata correttamente */
    if (pipedFigli == NULL)
    {
        printf("Errore nell'allocazione della memoria per la pipe figli-padre\n");
        exit(4);
    }
    
    /* Creo le pipe per consentire la comunicazione tra figli e padre */
    for (i = 0; i < W; i++)
    {
        /* Creazione della pipe */
        if (pipe(pipedFigli[i]) < 0)
        {
            /* La creazione della pipe ha fallito, stampo un messaggio d'errore ed esco specificando un valore intero d'errore */
            printf("Errore nel piping.\n");
            exit(5);
        }
    }

    /* Genero W processi figli */
    for (w = 0; w < W; w++)
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

            /* D'ora in avanti qualunque exit che indichi il fallimento ritornerà il valore -1 (255) */

            /* Chiudo il file descriptor del file fcreato siccome non viene utilizzato dal figlio e neppure dal nipote */
            close(fcreato);
            /* Chiudo i file descriptors dei lati di pipe che non servono nè al figlio nè al nipote */
            for (i = 0; i < W; i++)
            {
                /* Chiudo i lati di lettura che verranno utilizzati dal padre */
                close(pipedFigli[i][0]);
                /* Chiudo tutti i lati di scrittura utilizzati dagli altri figli, tenendo aperto solo quello corripondente a il figlio w */
                if (i != w)
                {
                    close(pipedFigli[i][1]);
                }
            }

            /* Creo una pipe per consentire la comunicazione tra nipote e figlio corrispondente */
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

                /* Chiudo i file descriptors dei lati di pipe che il nipote non usa */
                close(pipedFigli[w][1]);
                close(pipedNipote[0]);
                
                /* Ridirigo stdout su pipedNipote[1] */
                close(1);
                dup(pipedNipote[1]);
                
                /* Chiudo pipedNipote[1] che ho appena duplicato, quindi non serve piu' */
                close(pipedNipote[1]);
                
                /* Eseguo il comando ps il cui output andrà su pipedNipote. Non uso nessuna opzione perché viene richiesto l'elenco semplice */
                execlp("ps", "ps", (char*)0);
                
                /* Codice che viene eseguito solo in caso di fallimento della exec */
                printf("Errore in exec!\n");
                exit(-1);
            }
            
            /* Codice del figlio eseguito concorrentemente a quello del nipote */

            /* Ridirigo stdin su pipedNipote[0] così da poter usare grep come comando filtro */
            close(0);
            dup(pipedNipote[0]);

            /* Ridirigo stdout su pipedFigli[w][1] */
            close(1);
            dup(pipedFigli[w][1]);
            
            /* Chiudo pipedFigli[w][1] che ho appena duplicato, quindi non serve piu' */
            close(pipedFigli[w][1]);
            /* Chiudo pipedNipote[0] che ho appena duplicato, quindi non serve piu' */
            close(pipedNipote[0]);
            /* Chiudo il file descriptor del lato di pipe non utilizzato dal figlio */
            close(pipedNipote[1]);

            /* Scrivo su opzione il pid del padre che grep deve cercare */
            sprintf(opzione, "%d", getppid());
            
            /* Eseguo il comando-filtro grep sull'output prodotto dal figlio (rediretto su stdin), selezionando la linea corrispondente al PID del processo padre (contenuto in opzione) */
            execlp("grep", "grep", opzione, (char*)0);
            
            /* Codice che viene eseguito solo in caso di fallimento della exec */
            printf("Errore in exec!\n");
            exit(-1);
        }
    }
    
    /* Codice del padre */

    /* Chiudo i file descriptors dei lati di pipe non necessari per il padre */
    for (i = 0; i < W; i++)
    {
        close(pipedFigli[i][1]);
    }

    /* Leggo l'output dei figli rispettando il loro ordine */
    for (w = 0; w < W; w++)
    {
        /* Inizio un ciclo per la lettura di ogni singola linea */
        i = 0;
        while (read(pipedFigli[w][0], &(buff[i]), 1) != 0)
        {
            /* Trovato il termine di una linea */
            if (buff[i] == '\n')
            {
                /* Scrivo sul file fcreato la linea (buff) che ho appena finito di leggere (terminatore di linea compreso) */
                if (write(fcreato, buff, (i + 1)) == 0)
                {
                    printf("Errore: fallita la write della linea sul file %s creato dal padre nella directory corrente.\n", argv[2]);
                    exit(7);
                }
                
                /* Siccome il padre per ogni figlio e' uno solo e grep scrive sulla pipe la linea associata al pid del padre, questa sarà per forza una sola, quindi il ciclo può essere interrotto qui */
                break;
            }
            else
            {
                /* Aggiornamento indice */
                i++;
            }
        }
    }

    /* Aspetto i figli uno ad uno */
    for (w = 0; w < W; w++)
    {
        /* Recupero il valore di ritorno della wait assicurandomi che essa non incorra in errori */
        if ((pidFiglio = wait(&status)) < 0)
        {
            printf("Errore del padre in wait.\n");
            exit(8);
        }
        
        /* Controllo un eventuale terminazione anomala del figlio che è appena terminato */
        if ((status & 0xFF) != 0)
        {
            printf("Processo figlio %d terminato in modo anomalo.\n", pidFiglio);
        }
        else
        {
            /* In tal caso non si è verificata nessuna anomalia, quindi raccolgo il valore di ritorno contenuto in status */
            ritorno = (int)((status >> 8) & 0xFF);

            /* Controllo se il comando-filtro grep ha avuto successo (0) o ha riscontrato problemi (valore diverso da 0) */
            if (ritorno == 0)
            {
                printf("Il processo figlio %d ha ritornato %d, quindi la sua esecuzione ha avuto successo.\n", pidFiglio, ritorno);
            }
            else
            {
                printf("Il processo figlio %d ha ritornato %d, quindi ha fallito.\n", pidFiglio, ritorno);
            }
        }
    }

    exit(0);
}