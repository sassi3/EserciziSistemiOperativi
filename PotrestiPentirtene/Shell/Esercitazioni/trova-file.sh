#!/bin/sh

# Controllo dei parametri stretto: 1 parametro
if test $# -ne 1; then
    echo "Numero di parametri scorretto!"
    exit 1
fi

# Controllo che il parametro sia un nome relativo semplice
case $1 in
/*)
    echo "Il file e' un nome assoluto"
    exit 2
    ;;
*/*)
    echo "Il file e' un nome relativo"
    exit 3
    ;;
*) echo "Il file e' un nome relativo semplice" ;;
esac

# Verifico se nella directory corrente esiste un file con il nome del parametro specificato
if test -f $1; then
    echo "Il nome assoluto del file e': $(pwd)$1"
else
    echo "Nella cartella corrente non esiste il file $1"
fi
