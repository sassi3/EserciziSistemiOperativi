#!/bin/sh

# Mi sposto nella directory passata come primo parametro
cd $1

# Mi assicuro che la directory non sia vuota
if test "`ls -A`" = ""; then
    echo "La directory $1 e' vuota. Directory scartata."
    # Esco con valore 0 perché cio' non e' un errore
    exit 0
fi

for F in *; do
    if test -f "$F" -a -r "$F"; then
        # Controllo se il file ha estensione $2 o $3
        case $F in
        *.$2)
            echo "Il file `pwd`/$F ha estensione $2."
            echo "`pwd`/$F" >> "$4";;
        *.$3)
            echo "Il file `pwd`/$F ha estensione $2."
            echo "`pwd`/$F" >> "$5";;
        *)
            ;;
        esac
    fi
done

# Cerco directory da esplorare all'interno della directory corrente
for D in *; do
    if test -d "$D" -a -x "$D"; then
        # Chiamo lo script ricorsivo entrando nella directory
        FCR.sh "$1/$D" "$2" "$3"
    fi
done