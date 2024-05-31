#!/bin/sh

# Controllo dei parametri lasco
case $# in
0) ls -l | more ;;
1)
    # Controllo se il parametro è un file o una directory
    if
        test -d $1
    then
        # Comportamento in caso sia una directory
        ls -ld $1
    else
        # Comportamento in caso sia un file
        echo SONO DIR.sh
        echo 'Il valore di $0 ===>' $0
        echo 'Il valore di $1 ===>' $1
        echo "DEBUG-Ora eseguo ls -l $1"
        ls -l $1
    fi
    ;;
*) exit 1 ;;
esac
