#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdbool.h>

typedef int pipe_t[2];         /* definizione del TIPO pipe_t come array di 2 interi */

typedef struct caratteriLetti {
    char v1;        // Carattere cercato
    long int v2;    // Numero di occorrenze
} caratteriLetti;

void scambia(caratteriLetti* a, caratteriLetti* b)
{
    caratteriLetti tmp = *a;
    *a = *b;
    *b = tmp;
}

void bubbleSort(caratteriLetti v[], int dim)
{
    int i;
    bool ordinato = false;
    while (dim > 1 && !ordinato)
    {
        ordinato = true;
        for (i = 0; i < dim - 1; i++)
        {
            if (v[i].v2 > v[i + 1].v2)
            {
                scambia(&v[i], &v[i+1]);
                ordinato = false;
            }
        }
        dim--;
    }
}

int main(int argc, char** argv)
{
    /* ------------- Variabili locali ------------- */
    
    int pid;				/* process identifier per le fork() */
    int i, j;				/* indici per i cicli */
    int fd;                 /* file descriptor dell'unico file passato */
    pipe_t piped[26];		/* array statico di pipe descriptors */
    caratteriLetti cl[26];  /* array statico di strutture per salvare il carattere letto e le sue occorrenze */
    int status;				/* variabile di stato per la wait */
    int ritorno;			/* variabile usata dal padre per recuperare valore di ritorno di ogni figlio */
    
    /* -------------------------------------------- */

    if (argc != 2)
    {
        printf("Numero di parametri errato: argc = %d, ma dovrebbe essere = 2\n", argc);
        exit(1);
    }
    
    /* Creo una pipe per consentire la comunicazione tra figli verificando se l'operazione va a buon fine */
    for (i = 0; i < 26; i++)
    {
        /* Creazione della pipe */
        if (pipe(piped[i]) < 0)
        {
            /* La creazione della pipe ha fallito, stampo un messaggio d'errore ed esco specificando un valore intero d'errore */
            printf("Errore nel piping.\n");
            exit(3);
        }
    }

    for (i = 0; i < 26; i++)
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

            /* Chiudo i piped inutilizzati */
            for (j = 0; j < 26; j++)
            {
                if (i == 0 || j != (i - 1))
                {
                    close(piped[j][0]);
                }
                if (j != i)
                {
                    close(piped[j][1]);
                }
            }

            /* Leggo dalla pipe precedente i valori passati */
            if (i != 0)
            {
                read(piped[i - 1][0], cl, sizeof(cl));
            }

            /* Assegno alla struttura il carattere corrispondente */
            cl[i].v1 = 'a' + i;
            /* Faccio iniziare v2 da 0 */
            cl[i].v2 = 0L;

            /* Apertura del file e controllo esistenza */
            if ((fd = open(argv[1], O_RDONLY)) < 0)
            {
                printf("Errore nell'apertura del file '%s'.\n", argv[1]);
                exit(-1);
            }
            
            /* Ciclo di lettura dei caratteri */
            char tmp;
            while(read(fd, &tmp, 1) > 0)
            {
                if (cl[i].v1 == tmp)
                {
                    cl[i].v2++;
                }
            }

            /* Scrivo l'array di strutture sulla pipe */
            write(piped[i][1], cl, sizeof(cl));

            /* Esco scrivendo l'ultimo carattere letto */
            exit(tmp);
        }
    }
    
    /* Codice del padre (fuori dal for perche' mi interessa eseguirlo solo quando il ciclo e' terminato) */

    /* Chiudo i piped inutilizzati (lascio aperto solo l'ultimo per leggere dall'ultimo figlio) */
    for (i = 0; i < 26; i++)
    {
        close(piped[i][1]);
        if (i != 25)
        {
            close(piped[i][0]);
        }
    }

    /* Leggo l'array di strutture finale */
    read(piped[25][0], cl, sizeof(cl));

    /* Ordino i risultati in ordine crescente */
    bubbleSort(cl, 26);

    /* Stampo i risultati su stdout */
    for (i = 0; i < 26; i++)
    {
        printf("Ho trovato il carattere %c %ld volte nel file %s.\n", cl[i].v1, cl[i].v2, argv[1]);
    }
    
    /* Aspetto i figli */
    int pidFiglio;
    for (i = 0; i < 26; i++)
    {
        if ((pidFiglio = wait(&status)) < 0)
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
            printf("Il processo figlio %d ha ritornato %d.\n", pidFiglio, ritorno);
        }
    }
    
    exit(0);
}