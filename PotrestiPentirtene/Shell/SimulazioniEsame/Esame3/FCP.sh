#!/bin/sh

# Controllo che il numero di parametri sia rientri nella soglia desiderata
if test $# -lt 3
then
    # In tal caso il numero dei parametri non riesntra nell'intervallo desiderato, quindi interrompo l'esecuzione
    echo "Numero parametri errato. Processo interrotto."
    # Esco specificando un valore intero di errore
    exit 1;
fi

# Dichiaro una variabile come lista di parametri per le directory
dirList=
# Dichiaro una variabile da usare come contatore
J=1
# Scorro la lista parametri in un ciclo che utilizza il contatore J
for i; do
    # Escludo l'ultimo parametro
    if test "$J" -eq $#
    then
        echo "Escludo dai controlli l'ultimo parametro: $i."
        # Salvo il parametro nella variabile S
        S=$i
        break
    fi

    # Uso un case per verificare se il parametro e' un path assoluto
    case $i in
    /*)
        echo "Il parametro $i e' un path assoluto."
        # Ora verifico l'esistenza della directory
        if test -d $i -a -x $i
        then
            echo "Il parametro $i e' una directory traversabile."
            # Inserisco il nuovo elemento in coda alla lista
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

    # Incremento la variabile contatore di 1 a ogni iterazione
    J=`expr "$J" + 1`
done
# Reimposto il contatore al punto di partenza, ovvero a 1
J=1

# Aggiungo alla variabile d'ambiente PATH il path corrente
PATH=`pwd`:$PATH
# Aggiungo le modifiche effettuate alla variabile d'ambiente esportandola
export PATH

# Creo un file temporaneo
echo "Creo un file temporaneo."
> /tmp/temp$$

# Ciclo di chiamate al file ricorsivo FCR.sh
for i in $dirList; do
    # Chiamo il file ricorsivo FCR.sh passando i parametri richiesti
    FCR.sh "$i" "$S" /tmp/temp$$
    # Incremento la variabile contatore di 1 a ogni iterazione
    J=`expr "$J" + 1`
done

# Conto il numero di righe del file temporaneo
echo "Il numero di file $S.txt nelle gerarchie specificate e': `wc -l < /tmp/temp$$`."

# Stampo le informazioni dei file scritti nel file temporaneo
cat /tmp/temp$$

# Chiedo all'utente se vuole visualizzare in ordine alfabetico i file
echo "Vuoi visualizzare in ordine alfabetico i file? [S/N]"
# Leggo la risposta dell'utente
read risposta
# Controllo la risposta dell'utente
case $risposta in
S*|s*|Y*|y*)
    # Ordino alfabeticamente i file presenti nel file temporaneo senza separare le maiuscole dalle minuscole
    sort -f /tmp/temp$$;;
N*|n*)
    # Non faccio nulla
    ;;
*)
    # Risposta non valida
    echo "Risposta non valida.";;
esac

# Elimino il file temporaneo
echo "Elimino il file temporaneo."
rm /tmp/temp$$
