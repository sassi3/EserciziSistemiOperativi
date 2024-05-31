#!/bin/sh

# Mi sposto nella directory passata come primo parametro
cd $1

# Scorro i file presenti nella directory
for i in *; do
    # Controllo se il file e' leggibile, scrivibile e si chiama come il secondo parametro
    if test -f $i -a -r $i -a -w $i -a "$2.txt" = "$i"
    then
        echo "Il file $i e' leggibile, scrivibile e si chiama $2.txt."
        # Scrivo in append la lunghezza e il path assoluto del file sul file temporaneo passato come terzo parametro
        echo "`pwd`/$i `wc -l < $i`" >> "$3"
    else
        # Se non e' un file, controllo se e' una directory e se essa e' traversabile
        if test -d $i -a -x $i
        then
            echo "Il file $i e' una directory traversabile."
            # Chiamo il file ricorsivo FCR.sh passando i parametri richiesti
            FCR.sh "`pwd`/$i" "$2" "$3"
        fi
    fi
done