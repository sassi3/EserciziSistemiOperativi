#!/bin/sh

# Controllo dei parametri stretto: un parametro
if
    test $# -eq 1
then
    # Riporto su stdout se il parametro è un nome assoluto, relativo o relativo semplice
    case $1 in
    /*)
        echo "Il parametro e' un nome assoluto"
        if test -d $1; then
            echo "Il parametro e' una directory"
            if test -x $1; then
                echo "La directory $1 e' traversabile"
            else
                echo "La directory non e' traversabile"
            fi
        else
            echo "Il parametro e' un file"
        fi
        ;;
    */*)
        echo "Il parametro e' un nome relativo"
        ;;
    *)
        echo "Il parametro e' un nome relativo semplice"
        ;;
    esac
else
    exit 1
fi
