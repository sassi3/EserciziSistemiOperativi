#!/bin/sh

# Controllo che il numero di parametri sia rientri nella soglia desiderata
if test $# -lt 4
then
    # In tal caso il numero dei parametri non riesntra nell'intervallo desiderato, quindi interrompo l'esecuzione
    echo "Numero parametri errato. Processo interrotto."
    # Esco specificando un valore intero di errore
    exit 1;
fi

# Controllo se il parametro $1 e' un numero intero positivo
case $1 in
*[!0-9]*)
    # In tal caso il parametro non e' un numero intero, quindi interrompo l'esecuzione
    echo "Il parametro $1 non e' un numero intero. Processo interrotto."
    # Esco specificando un valore intero di errore
    exit 5;;
*)
    echo "Il parametro $1 e' un numero intero."
    # Controllo se il numero e' positivo
    if test "$1" -le 0; then
        echo "Il numero $1 non e' positivo."
        # Esco specificando un valore intero di errore
        exit 6
    else
        echo "Il numero $1 e' positivo."
    fi;;
esac

# Salvo il parametro in una variabile 
W=$1

# Controllo che il secondo parametro sia una stringa
case $2 in
    *[0-9]*) 
        echo "Il parametro $2 non e' una stringa. Processo interrotto."
        # Esco specificando un valore intero di errore
        exit 9;;
    *) 
        echo "Il parametro $2 e' una stringa.";;
esac

# Lo salvo in una variabile
S=".$2"

# Uso shift per scartare i primi due parametri
shift 2

# Dichiaro una variabile lista per i path
dirList=

# Controllo che i parametri rimanenti siano path assoluti e al contempo directory traversabili
for i; do
    # Uso un case per verificare se il parametro e' un path assoluto
    case $i in
    /*)
        echo "Il parametro $i e' un path assoluto."
        # Ora verifico l'esistenza della directory e che essa sia traversabile
        if test -d "$i" -a -x "$i"
        then
            echo "Il parametro $i e' una directory traversabile."
            # Aggiungo il path alla lista
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

# Scorro la lista di directory
for G in $dirList; do
    # Chiamo lo script ricorsivo
    FCR.sh "$G" "$S" "/tmp/nomiAssoluti"
done

# Salvo in una variabile il numero di directory trovate
numDir=`wc -l < /tmp/nomiAssoluti`

if test "$numDir" -lt "$W"; then
    # Stampo il numero totale di directory trovate
    echo "Il numero totale di directory trovate e': $numDir."
else 
    # Chiedo a Mattia di inserire un numero intero compreso tra 1 e W
    echo "Mattia, inserisci un numero intero compreso tra 1 e $W:"
    # Leggo il numero inserito da tastiera
    read X
    # Controllo se il parametro e' un numero intero
    case $X in
    *[!0-9]*)
        # In tal caso il parametro non e' un numero intero, quindi interrompo l'esecuzione
        echo "Il parametro $X non e' un numero intero. Processo interrotto."
        # Esco specificando un valore intero di errore
        exit 10;;
    *)
        # Controllo se il numero e' compreso tra 1 e $W
        if test "$X" -le 0 -o "$X" -gt "$W"; then
            echo "Il numero $X non e' compreso tra 1 e $W. Processo interrotto."
            # Esco specificando un valore intero di errore
            exit 11
        else
            echo "Path assoluto della directory numero $X:"
            # Stampo il path assoluto della directory numero X
            head -"$X" /tmp/nomiAssoluti | tail -n 1
        fi;;
    esac
fi

# Elimino il file temporaneo
echo "Elimino il file temporaneo."
rm /tmp/nomiAssoluti
