#!/bin/sh

# Mi sposto nella directory passata come primo parametro
cd $1

# Dichiaro una variabile trovato per tenere traccia se ho trovato un file con terminazione .$2
trovato="no"

# Controllo che la directory contenga almeno un file con terminazione .$2
for F in *; do
    # Controllo se $i e' un file leggibile, se ha terminazione .$2 e se non ho ancora trovato un file con terminazione .$2 
    if test "$trovato" = "no" -a -f "$F" -a -r "$F"; then
        case $F in
            *$2)
                echo "Il file $F ha terminazione $2. La directory e' valida."
                # Scrivo la directory corrente sul file temporaneo
                echo "$1" >> "$3"
                # Interrompo la ricerca di file compatibili perche' me ne basta uno
                trovato="si"
                ;;
            *)
                echo "Il file $F non ha terminazione $2.";;
        esac
    fi

    # Controllo se $i e' una directory traversabile
    if test -d "$F" -a -x "$F"; then
        # Chiamo lo script ricorsivo
        FCR.sh "$1/$F" "$2" "$3"
    fi
done