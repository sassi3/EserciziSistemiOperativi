#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <ctype.h>

typedef int pipe_t[2];         /* definizione del TIPO pipe_t come array di 2 interi */

typedef struct {
    int c1;         /* campo c1 del testo */
    char c3[250];  /* campo c2 del testo */
                            /* bastano 250 caratteri per contenere ogni riga insieme con il terminatore di linea e con il terminatore di stringa (vedi specifica) */
    int c2;         /* campo c3 del testo */
} Strut;

int main(int argc, char** argv)
{
    /* ------------- Variabili locali ------------- */
    
    int pid;				/* process identifier per le fork() */
    int N;					/* numero di file passati sulla riga di comando */
    int i,j;				/* indici per i cicli */
    pipe_t *piped;			/* array dinamico di pipe descriptors */
    pipe_t pipedNipote;     /* pipe per la comunicazione tra figlio e nipote */
    int status;				/* variabile di stato per la wait */
    int ritorno;			/* variabile usata dal padre per recuperare valore di ritorno di ogni figlio */
    Strut datiLetti;        /* struttura usata dai figli e dal padre */
    
    /* -------------------------------------------- */
    
    if (argc < 4)
    {
        printf("Numero di parametri errato: argc = %d, ma dovrebbe essere >= 4\n", argc);
        exit(1);
    }
    
    /* Numero di parametri passati da linea di comando */
    N = argc - 1;
    
    /* Allocazione dell'array di pipe */
    piped = (pipe_t*)malloc(N * sizeof(pipe_t));
    /* Controllo che l'allocazione di memoria sia andaata a buon fine */
    if (piped == NULL)
    {
        /* In tal caso si e' verificato un errore nell'allocazione: stampo un messaggio d'errore ed esco specificando un valore intero d'errore */
        printf("Errore nell'allocazione della memoria per il vettore di pipe.\n");
        exit(2);
    }
    
    /* Creazione delle N pipe */
    for (i = 0; i < N; i++)
    {
        /* Controllo che ogni creazione vada a buon fine */
        if (pipe(piped[i]) < 0)
        {
            /* Non e' stato possibile creare la pipe con successo: stampo un messaggio d'errore ed esco specificando un valore intero d'errore */
            printf("Errore nel piping.\n");
            exit(3);
        }
    }
    
    /* Generazione degli N figli */
    for (i = 0; i < N; i++)
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

            /* Chiudo i lati di pipe non utilizzati dal figlio */
            for (j = 0; j < N; j++)
            {
                close(piped[j][0]);
                if (i != j)
                {
                    close(piped[j][1]);
                }
            }
            
            /* Creo una pipe per consentire la comunicazione figlio-nipote verificando se l'operazione va a buon fine */
            if ((pipe(pipedNipote)) < 0)
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

                /* Chiusura del lato di pipe del figlio non utilizzata dal nipote, ma ereditata */
                close(piped[i][1]);

                /* Chiusura del lato di pipe non utilizzato dal nipote */
                close(pipedNipote[0]);
                
                /* Chiudo stdout e lo sostituisco con la pipe del nipote */
                close(1);
                dup(pipedNipote[1]);
                close(pipedNipote[1]);

                /* Eseguo il comando sort sul file da ordinare usando la exec */
                execlp("sort", "sort", "-f", argv[i + 1], (char*)0);

                /* Se torno qui, si è verificato un errore */
                perror("Errore in sort.\n");
                exit(-1);
            }
            
            /* Codice del figlio */
            
            /* Chiusura del lato di pipe non utilizzato dal figlio */
            close(pipedNipote[1]);

            /* Scrivo il pid del nipote nella struttura dati */
            datiLetti.c1 = pid;

            /* Leggo dalla pipe solo la prima riga */
            j = 0;
            while (read(pipedNipote[0], &(datiLetti.c3[j]), 1))
            {
                /* Se incontro un \n, ho finito di leggere la prima riga */
                if (datiLetti.c3[j] == '\n')
                {
                    /* Salvo la lunghezza della sequenza di caratteri (\n escluso) */
                    datiLetti.c2 = j;
                    break;
                }
                j++;
            }
            /* Reset di j a 0 */
            j = 0;

            /* Scrivo sulla pipe i dati da inviare al padre */
            write(piped[i][1], &datiLetti, sizeof(datiLetti));

            /* Aspetto il nipote */
            ritorno = -1;
            if ((pid = wait(&status)) < 0)
            {
                printf("Errore del figlio in wait.\n");
            }
            if ((status & 0xFF) != 0)
            {
                printf("Processo nipote %d terminato in modo anomalo.\n", pid);
            }
            else
            {
                /* Ritorno la lunghezza della linea terminatore escluso */
                ritorno = datiLetti.c2 - 1;
            }
            
            exit(ritorno);
        }
    }
    
    /* Codice del padre */

    /* Chiudo il lato di pipe non usato dal padre */
    for (i = 0; i < N; i++)
    {
        close(piped[i][1]);
    }
    
    /* Leggo i dati inviati dai figli */
    for (i = 0; i < N; i++)
    {
        read(piped[i][0], &datiLetti, sizeof(datiLetti));

        /* Inserisco qui il terminatore (ciò signfica trasformare c3 in stringa... FOLLIA)*/
        datiLetti.c3[datiLetti.c2] = '\0';

        printf("%d %d %s\n", datiLetti.c1, datiLetti.c2, datiLetti.c3);
    }

    /* Aspetto i figli */
    for (i = 0; i < N; i++)
    {
        /* Aspetto il figlio */
        if ((pid = wait(&status)) < 0)
        {
            printf("Errore del padre in wait.\n");
            exit(8);
        }
        if ((status & 0xFF) != 0)
        {
            printf("Processo figlio %d terminato in modo anomalo.\n", pid);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il processo figlio %d ha ritornato %d.\n", pid, ritorno);
        }
    }
    
    exit(0);
}