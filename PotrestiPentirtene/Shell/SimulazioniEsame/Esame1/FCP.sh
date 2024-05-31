#!/bin/sh

# Verifica del numero di parametri inserito
if test $# -lt 3; then
    echo "Numero di parametri errato."
    exit 1
fi

# Assegno un contatore per tenere traccia dei passi che faccio nel for
N=1
# Ciclo che verifica che i primi N parametri siano nomi assoluti di directory
for i; do
    if test $N -eq $#; then
        # Se sono entrato qui significa che ho terminato di leggere gli N parametri
        # Ora verifico che l'ultimo parametro sia un numero intero
        case $i in
        *[!0-9]*)
            echo "Il parametro $i non e' un numero."
            exit 5
            ;;
        *)
            if test $i -le 0; then
                echo "Il numero $i non e' positivo."
            else
                echo "Il numero $i e' positivo."
            fi
            X=$i
            ;;
        esac
        break
    fi

    # Lista delle cartelle traversabili
    dirFound=

    # Verifico che i parametri forniti siano path assoluti
    case $i in
    /*)
        echo "Il parametro $i e' un path assoluto."
        # Verifico che i parametri forniti siano directory esistenti
        if test -d $i -a -x $i; then
            # Aggiorno la lista di cartelle trovate
            dirFound="$dirFound $i"
            echo "Il parametro $i e' una directory."
        else
            echo "Il parametro $i non e' una directory."
            exit 4
        fi
        ;;
    *)
        echo "Il parametro $i non e' un path assoluto."
        exit 2
        ;;
    esac

    # Incremento il contatore
    N=`expr $N + 1`
done

# Aggiungo alla variabile d'ambiente PATH il path corrente
PATH=`pwd`:$PATH
# Aggiungo le modifiche effettuate alla variabile d'ambiente esportandola
export PATH

# Creo un file temporaneo per salvare le directory valide
> /tmp/dirValide$$

# Chiamo lo script ricorsivo per ogni directory trovata
echo "Chiamo lo script ricorsivo."
for i in $dirFound; do
    echo "Chiamo lo script ricorsivo per la directory $i."
    FCR.sh "$i" "$X" /tmp/dirValide$$
done

# Conto il numero di directory valide
echo "Il numero di directory valide e' `wc -l < /tmp/dirValide$$`."

for j in `cat /tmp/dirValide$$`; do
    echo "Path assoluto della directory valida: $j"
    if test `ls "$j"` -eq 0; then
        echo "La directory $j e' vuota."
    else
        echo "La directory $j non e' vuota."
        # Mi sposto nella directory $j
        cd "$j"
        # Controllo i file presenti nella directory
        for k in *; do
            echo "File trovato: $k"
            # Controllo se il file ha meno di X righe
            if test `wc -l < "$j/$k"` -lt $X; then
                echo "Il file $k ha meno di $X righe."
            else
                # Stampo la $X-esima riga del file
                head -"$X" "$j/$k" | tail -1 | cat
            fi
        done
    fi
done

# Cancello il file temporaneo
rm /tmp/dirValide$$
echo "File temporaneo rimosso."