#!/bin/sh

# Mi sposto nella directory passata come parametro
cd "$1"

# Varibile che segnala se ho trovato o no una sottodir (0 se non c'e', 1 altrimenti)
flag=0
echo "flag = $flag"
for i in *; do
    # Controllo se il file e' una directory
    if test -d "$i"; then
        echo "Directory non valida. Sottodirectory $i trovata."
        flag=1
        echo "flag = $flag"
        # In tal caso avvio un ulteriore processo di esplorazione
        FCR.sh "`pwd`/$i" "$2"
    else
        # Controllo se il file e' di lunghezza inferiore o uguale a $2
        if test `wc -l < $i` -le "$2"; then
            echo "Directory non valida. File $i di lunghezza (in linee) inferirore o uguale a $2 trovato."
            flag=1
            echo "flag = $flag"
        fi
    fi
done

# Controllo se tra i file c'e' almeno una directory
if test "$flag" -eq 0; then
    echo "Directory valida."
    # Salvo il nome della directory in un file temporaneo (se non esiste, lo creo)
    echo `pwd`/"$1" >> "$3"
fi
