#!/bin/sh

# Controllo che il numero di parametri sia rientri nella soglia desiderata
if test $# -lt 3
then
    # In tal caso il numero dei parametri non riesntra nell'intervallo desiderato, quindi interrompo l'esecuzione
    echo "Numero parametri errato. Processo interrotto."
    # Esco specificando un valore intero di errore
    exit 1;
fi

# Verifico che il primo parametro sia un carattere
case $1 in
    [a-z,A-Z])
        echo "Il primo parametro $1 e' un carattere.";;
    *)
        echo "Il primo parametro $1 non e' un carattere. Processo interrotto."
        # Esco specificando un valore intero di errore
        exit 2;;
esac

# Salvo il primo parametro in una variabile
C=$1

# Uso shift per non considerare il primo parametro
shift

# Dichiaro una variabile lista che conterra' gli N parametri
dirList=

# Uso un ciclo for per scorrere gli N parametri
for i; do
    # Uso un case per verificare se il parametro e' un path assoluto
    case $i in
    /*)
        echo "Il parametro $i e' un path assoluto."
        # Ora verifico l'esistenza della directory e che essa sia traversabile
        if test -d "$i" -a -x "$i"
        then
            echo "Il parametro $i e' una directory traversabile."
            # Aggiungo il parametro alla lista
            dirList="$dirList $i"
        else
            echo "Il parametro $i non e' una directory traversabile. Processo interrotto."
            # Esco specificando un valore intero di errore
            exit 3
        fi;;
    *)
        echo "Il parametro $i non e' un path assoluto. Processo interrotto."
        # Esco specificando un valore intero di errore
        exit 2;;
    esac
done

# Aggiungo alla variabile d'ambiente PATH il path corrente
PATH=`pwd`:$PATH
# Aggiungo le modifiche effettuate alla variabile d'ambiente esportandola
export PATH

# Creo un file temporaneo
echo "Creo un file temporaneo."
> /tmp/nomiAssoluti

# Uso un ciclo for per scorrere la lista delle directory
for i in $dirList; do
    # Chiamo lo script ricorsivo
    FCR.sh "$i" "$C" "/tmp/nomiAssoluti"
done

# Stampo il numero di directory trovate
echo "Numero di directory trovate: `wc -l /tmp/nomiAssoluti`"

# Scorro le righe del file temporaneo
for i in `cat /tmp/nomiAssoluti`; do
    # Stampo il contenuto della riga
    echo "Directory: $i. Elena, vorresti visualizzare anche gli elementi nascosti della directory? [S/N]"
    # Leggo la risposta di Elena
    read -r answer
    # Controllo la risposta di Elena
    case $answer in
    S*|s*|Y*|y*)
        # Visualizzo gli elementi nascosti della directory
        ls -a "$i";;
    N*|n*)
        # Non visualizzo gli elementi nascosti della directory
        ls "$i";;
    *)
        # Risposta non valida
        echo "Risposta non valida.";;
    esac
done

# Elimino il file temporaneo
echo "Elimino il file temporaneo."
rm /tmp/nomiAssoluti
