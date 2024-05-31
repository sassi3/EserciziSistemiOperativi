#!/bin/sh

# Controllo dei parametri stretto: 2 parametri accettati
if test $# -ne 2; then
    echo "Numero parametri errato"
    exit 1
fi

# Controllo che i due parametri siano nomi relativi semplici
for i in $1 $2; do
    case $i in
    */*)
        echo "Il parametro $i non e' un nome relativo semplice"
        exit 3
        ;;
    *)
        echo "Il parametro $i e' un nome relativo semplice"
        ;;
    esac
done

# Controllo che il primo parametro sia un file leggibile
if test ! -f $1 -a ! -r $1; then
    echo "Il parametro $1 non e' valido"
    exit 2
fi

# Controllo che il secondo parametro sia un file scrivibile
if test ! -f $2 -a ! -w $2; then
    echo "Il parametro $2 non e' valido"
    exit 2
fi

# Scrivo il contenuto del file1 nel file2
cat $1 >>$2
echo "Script terminato correttamente"
