#!/bin/sh

# Controllo dei parametri stretto: un parametro
if
test $# -eq 1
then
# Riporto su stdout se il parametro è un nome assoluto, relativo o relativo semplice
case $1 in
/*) echo "Il parametro e' un nome assoluto";;
*/*) echo "Il parametro e' un nome relativo";;
*) echo "Il parametro e' un nome relativo semplice";;
esac
else 
exit 1
fi
