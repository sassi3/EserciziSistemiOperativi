#!/bin/sh

# Mi sposto nella directory passata come primo parametro
cd "$1"

# Mi assicuro che la directory non sia vuota
if test "`ls -A`" = ""; then
    echo "La directory $1 e' vuota. Directory scartata."
    # Esco con valore 0 perché cio' non e' un errore
    exit 0
fi

# Controllo che la directory contenga almeno un file con terminazione .$2
for F in *; do
    if test -f "$F" -a -r "$F" -a "`wc -l < "$F"`" -eq "$2"; then
        echo "Il file $F contiene $2 righe."
        # Scrivo il path assoluto del file nel file temporaneo
        echo "$1/$F" >> "$3"
    fi
done

# Cerco directory da esplorare all'interno della directory corrente
for D in *; do
    if test -d "$D" -a -x "$D"; then
        # Chiamo lo script ricorsivo entrando nella directory
        FCR.sh "$1/$D" "$2" "$3"
    fi
done