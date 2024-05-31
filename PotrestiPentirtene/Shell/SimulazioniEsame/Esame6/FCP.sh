#!/bin/sh

# Controllo che il numero di parametri rientri nella soglia desiderata
if test $# -lt 3
then
    # In tal caso il numero dei parametri non riesntra nell'intervallo desiderato, quindi interrompo l'esecuzione
    echo "Numero parametri errato. Utilizzo corretto di $0: $0 numero_intero dir_ass1 dir_ass2. Processo interrotto."
    # Esco specificando un valore intero di errore
    exit 1;
fi

# Controllo se il primo parametro e' un numero intero
case $1 in
*[!0-9]*)
    # In tal caso il parametro non e' un numero intero, quindi interrompo l'esecuzione
    echo "Il parametro $1 non e' un numero intero. Processo interrotto."
    # Esco specificando un valore intero di errore
    exit 5;;
*)
    echo "Il parametro $1 e' un numero intero."
    # Controllo se il numero e' positivo
    if test "$1" -le 0; then
        echo "Il numero $1 non e' positivo."
        # Esco specificando un valore intero di errore
        exit 6
    else
        echo "Il numero $1 e' positivo."
    fi;;
esac

# Salvo il primo parametro in una variabile
X=$1

# Shift per scorrere i parametri piu' agevolmente
shift

# Dichiaro una variabile lista per i path assoluti
Q=

# Controllo che i parametri rimanenti siano path assoluti e al contempo directory traversabili
for i; do
    # Uso un case per verificare se il parametro e' un path assoluto
    case $i in
    /*)
        echo "Il parametro $i e' un path assoluto."
        # Ora verifico l'esistenza della directory e che essa sia traversabile
        if test -d "$i" -a -x "$i"
        then
            echo "Il parametro $i e' una directory traversabile."
            # Aggiungo il path alla lista
            Q="$Q $i"
        else
            echo "Il parametro $i non e' una directory traversabile. Processo interrotto."
            # Esco specificando un valore intero di errore
            exit 3
        fi;;
    *)
        echo "Il parametro $i non e' un path assoluto. Processo interrotto."
        # Esco specificando un valore intero di errore
        exit 2;;
    esac
done

# Aggiungo alla variabile d'ambiente PATH il path corrente
PATH=`pwd`:$PATH
# Aggiungo le modifiche effettuate alla variabile d'ambiente esportandola
export PATH

# Creo un file temporaneo
echo "Creo un file temporaneo."
> /tmp/nomiAssoluti

# Creo una variabile dove salvare passo passo il numero di file trovati
trovati=0

# Variabile contenente il numero di file della prima gerarchia (in seguito si vedra' perche' serve)
G1num=0

# Scorro la lista di directory
for G in $Q; do
    # Chiamo lo script ricorsivo
    FCR.sh "$G" "$X" "/tmp/nomiAssoluti"

    # Salvo in una variabile il numero globale di file trovati
    global=`wc -l < /tmp/nomiAssoluti`

    # Stampo il numero di file trovati nella gerarchia corrente detraendo a il numero globale (aggiornato nell'operazione precedente) 
    # il numero di file precedentemente scritti nel file temporaneo
    echo "Numero di file trovati nella gerarchia $G: `expr "$global" - "$trovati"`" 

    # Se $trovati e' 0, allora ho esplorato la prima gerarchia
    if test "$trovati" -eq 0; then
        # Siccome questo e' il primo blocco di file, salvo in G1 il numero di file
        G1num=$global
    fi

    # Aggiorno il numero di file trovati
    trovati=$global
done

# Numero di file trovati escludendo quelli della prima gerarchia (utile per effettuare il tail)
confrontabili=`expr "$trovati" - "$G1num"`
echo "Numero di file da confrontare con i file contenuti in $1: $confrontabili."

# Ora eseguo le operazioni di confronto tra i file di G1 e quelli delle altre gerarchie
for F1 in `tail -"$confrontabili" /tmp/nomiAssoluti`; do
    for F2 in `head -"$G1num" /tmp/nomiAssoluti`; do
        # Controllo che i file siano uguali
        if test "`cat "$F2"`" = "`cat "$F1"`"; then
            echo "I file $F2 e $F1 sono uguali."
        else 
            echo "I file $F2 e $F1 sono diversi."
        fi
    done
done

# Elimino il file temporaneo
echo "Elimino il file temporaneo."
rm /tmp/nomiAssoluti
