#!/bin/sh

# Controllo parametri stretto: un solo parametro accettato
if test $# -ne 1; then
    exit 1
fi

# Controllo se il parametro è un file o una directory
if
    test -d $1
then
    ls -ld $1
else
    echo SONO DIR.sh
    echo 'Il valore di $0 ===>' $0
    echo 'Il valore di $1 ===>' $1
    echo "DEBUG-Ora eseguo ls -l $1"
    ls -l $1
fi
