#!/bin/sh

# Controllo che il numero di parametri rientri nella soglia desiderata
if test $# -lt 4; then
    # In tal caso il numero dei parametri non riesntra nell'intervallo desiderato, quindi interrompo l'esecuzione
    echo "Numero parametri errato. Processo interrotto."
    # Esco specificando un valore intero di errore
    exit 1;
fi

# Controllo che il parametro $1 sia una stringa
case $1 in
*[0-9]*)
    echo "Il parametro $1 non e' una stringa. Processo interrotto."
    # Esco specificando un valore intero di errore
    exit 9;;
*)
    echo "Il parametro  e' una stringa.";;
esac

# Salvo il primo parametro in una variabile
S1=$1

# Controllo che il parametro $2 sia una stringa
case $2 in
*[0-9]*)
    echo "Il parametro $2 non e' una stringa. Processo interrotto."
    # Esco specificando un valore intero di errore
    exit 9;;
*)
    echo "Il parametro $2 e' una stringa.";;
esac

# Salvo il secondo parametro in una variabile
S2=$2

# Shift per scorrere i parametri successivi piu' agevolmente
shift 2

# Dichiaro una variabile lista per i path assoluti
Q=

# Scorro i parametri
for G; do
    # Uso un case per verificare se il parametro e' un path assoluto
    case $G in
    /*)
        echo "Il parametro $G e' un path assoluto."
        # Ora verifico l'esistenza della directory e che essa sia traversabile
        if test -d "$G" -a -x "$G"; then
            echo "Il parametro $G e' una directory traversabile."
            # Aggiorno la lista dei path assoluti
            Q="$Q $G"
        else
            echo "Il parametro $G non e' una directory traversabile. Processo interrotto."
            # Esco specificando un valore intero di errore
            exit 3
        fi;;
    *)
        echo "Il parametro $G non e' un path assoluto. Processo interrotto."
        # Esco specificando un valore intero di errore
        exit 2;;
    esac
done

# Creo i file temporanei
echo "Creo i file temporanei."
> /tmp/nomiAssoluti$S1
> /tmp/nomiAssoluti$S2

# Aggiungo alla variabile d'ambiente PATH il path corrente
PATH=`pwd`:$PATH
# Aggiungo le modifiche effettuate alla variabile d'ambiente esportandola
export PATH

# Scorro la lista parametri in un ciclo che utilizza il contatore n
for G; do
    FCR.sh "$G" "$S1" "$S2" "/tmp/nomiAssoluti$S1" "/tmp/nomiAssoluti$S2"
done

# Conto le occorrenze dei file con estensione S1
TOT1=`wc -l < "/tmp/nomiAssoluti$S1"`
echo "Il numero di file trovati con estensione $S1 e': $TOT1"

# Conto le occorrenze dei file con estensione S2
TOT2=`wc -l < "/tmp/nomiAssoluti$S2"`
echo "Il numero di file trovati con estensione $S2 e': $TOT2"

# Controllo se $TOT1 <= $TOT2
if test "$TOT1" -le "$TOT2"; then
    # Interrogo l'utente
    echo "Mattia, inserisci un numero intero compreso tra 1 e $TOT1: "
    # Leggo la risposta dell'utente
    read -r X
    # Controllo se il parametro e' un numero intero
    case $X in
    *[!0-9]*)
        # In tal caso il parametro non e' un numero intero, quindi interrompo l'esecuzione
        echo "Il parametro i non e' un numero intero. Processo interrotto."
        # Esco specificando un valore intero di errore
        exit 5;;
    *)
        echo "Il parametro $X e' un numero intero."
        # Controllo se il numero e' positivo
        if test "$X" -le 0; then
            echo "Il numero $X non e' positivo."
            # Esco specificando un valore intero di errore
            exit 6
        else
            echo "Il numero $X e' positivo."
        fi;;
    esac
    # Stampo l'X-esimo file sia avente estensione $S1 sia $S2
    head -n "$X" "/tmp/nomiAssoluti$S1" | tail -n 1
    head -n "$X" "/tmp/nomiAssoluti$S2" | tail -n 1
fi

# Elimino i file temporanei
echo "Elimino i file temporanei."
rm "/tmp/nomiAssoluti$S1"
rm "/tmp/nomiAssoluti$S2"
