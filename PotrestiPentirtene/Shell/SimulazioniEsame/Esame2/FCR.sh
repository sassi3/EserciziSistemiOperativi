#!/bin/sh

# Mi sposto nella directory passata come primo parametro
cd "$1"

# Scorro tutti i file nella directory
for i in *; do
    # Verifico di avere diritto di lettura del file e che esso abbia Y righe o piu'
    if test -r "$i" -a `wc -l <"$i"` -ge "$2"; then
        head -5 "$i" | tail -1 | cat >"$i".quinta
        echo "`pwd`/$i" >> "$3"
    else
        # Controllo se e' una directory e nel caso se e' traversabile
        if test "$i" -d; then
            if test "$i" -x; then
                FCR.sh "$j" "$2" "$3"
            fi
        else
            > "$i".NOquinta
        fi
    fi
done
