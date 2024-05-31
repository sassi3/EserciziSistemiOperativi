# Questo è letteralmente un commento

# Nome del primo file comandi dell'esame: FCP.sh
# Nome del secondo file comandi dell'esame: FCR.sh

# Per eventuali problemi riscontrati lavorando con i match tra stringhe è possibile che si risolvano con i seguenti comandi e variabili
$ LC_ALL=C
$ export LC_ALL     # vedi in seguito il significato di export
# In alternativa
$ LC_COLLATE=C 
$ export LC_COLLATE

# --------- METACARATTERI ---------
#
# & esegue un processo in background
#
# -- MATCH --
# * fa match con qualunque stringa, anche vuota
# ? fa match con un qualunque carattere, ma non con il carattere vuoto
# [abc] (fa schifo) oppure [a,b,c] fa match con qualunque stringa che contiene almeno uno dei caratteri tra quelli nell'insieme. Può essere negato con ! all'interno delle quadre
# [c1-cn] fa match con le stringhe che contengono almeno un carattere tra quelli nel range specificato (estremi inclusi). Può essere negato con ! all'interno delle quadre
# Utilizzi notevoli: 
#   [abc]*              stringhe che iniziano per 'a', 'b' o 'c'
#   *[0-9]              stringhe che terminano con un carattere compreso tra '0' e '9' (dunque che terminano con un numero intero positivo)
#   *[0-9]*             stringhe che in qualunque posizione devono contenere un carattere compreso tra '0' e '9'. Chiaramente questo pattern fa match solo con numeri interi positivi
#   [a-p,1-7]*[!0-9]    nomi che iniziano con un carattere compreso tra 'a' e 'p' o tra '1' e '7' e che non terminano con un numero intero positivo
# . i nomi dei file che iniziano con "." UNIX li considera nascosti. Per fare match con le stringhe che contengono un . e' obbligatorio usare "*\.*" perche' il punto e' classificato come metacarattere
# \ carattere di escape che consente di ignorare i metacaratteri
#
# -- RIDIREZIONI --
# | piping
# > redirezione di stdout su file (se quest'ultimo non esiste viene creato)
# >> redirezione di stdout su file in modalità append (se quest'ultimo non esiste viene creato)
# < redirezione di stdin da file
# Per decidere dove stampare l'output delle ridirezioni si può porre un numero davanti al simbolo di ridirezioni:
#   0 stampa su stdin
#   1 stampa su stdout
#   2 stampa su stderr
#   se il numero non è specificato, la destinazione è implicita
# Tutto ciò che viene ridiretto su /dev/null viene perso
# Tutto ciò che viene ridiretto su /dev/tty (teletypewriter) è assicurato che verrà visualizzato sul terminale corrente
# Se volessi ridirigere sia stdout che stderr su un file dovrei scrivere:
#   ls -l z* p* > file 2>&1         dove ciò che indica tale volontà è 2>&1



# --------- IMPORTANTE ---------
# Le opzioni possono essere combinate tra loro, ad esempio: ls -la 
# corrisponde ai risultati di ls -l combinati a quelli di ls -a
# (l'ordine non influenza il risultato: -la e -al sono equivalenti)
# 
# Quando un comando richiede un file o una directory contenuto nel sistema
# è sottointeso che vada inserito il path assoluto o relativo.



# --------- COMANDI-FILTRO ---------
# Un comando filtro e' tale nel momento in cui funziona prendendo in input un flusso (stdin) o un generico file (in RIDIREZIONE)
# e restituendo un flusso in output (stdout o file generico in ridirezione). Si dice "filtro" perche' filtra e/o manipola dati secondo determinati criteri
$ cat
$ more
$ sort
$ grep
$ wc
$ head
$ tail
$ rev
$ tee



# Effettuare il logout
$ exit
# terminare un processo (ha senso che sia una sottoshell) con un determinato valore di ritorno
$ exit n        # dove n è un intero positivo

# Data e orario corrente
$ date

# cat è comando-filtro, ma esiste anche la versione comando
# Stampare su stdout il contenuto di uno o più file
$ cat string        # (catenate) versione comando-filtro
$ cat file          # versione comando
# cat accetta i pattern:
# se ad esempio mi trovo in una cartella in cui ci sono più file che iniziano per f e scrivo
$ cat f*
# verranno stampati su stdout i contenuti di tutti i file concatenati indistintamente (difficile leggibilità)
# è possibile chiamare il comando cat senza parametri
$ cat
# esso stamperà su stdout ciò che l'utente inserirà da tastiera (stdin)

# more è comando-filtro, ma esiste anche la versione comando
# Stampare su stdout il contenuto di uno o più file con intestazione e separatore
$ more string           # versione comando-filtro
$ more file             # versione comando
# l'output sarà
# :::::::::::::
# file
# :::::::::::::
# contenuto del file
# 
# ciò è utile per visualizzare più file evitando il problema di leggibilità di cat

# Visualizzare le informazioni relative all'utente corrente (in particolare UID e GID)
$ id

# Visualizzare gli utenti attivi sul sistema
$ who
$ w         # formato dell'output diverso (piu' completo), ma informazioni analoghe

# Per sapere dove si trova un file eseguibile
$ which exe         # viene verificato che si trovi in uno dei percorsi contenuti in PATH

# Visualizza dove si trova un file binario, un file source o del manuale di un certo comando 
$ whereis binfile
$ whereis sourcefile
$ whereis command

# Avviare una shell qualsiasi
$ sh            # Bourne Shell
$ bash          # Bourne Again Shell
    # options:
    -x          # (xtrace) esegue la Bourne Shell, ma fornisce informazioni sull'esecuzione 
                # di certi comandi espandendoli
                # Esempio: $ ls -l file*
                #          + ls -l file file1 file2 file3 file4 filetmp      <-- questa si chiama espansione
                # utile per sapere se i match vengono eseguiti correttamente
    -v          # utile per stampare le linee di un eventuale file comandi invocato così come sono lette dalla shell
# infatti è possibile assegnare un file.sh a una shell per eseguirlo
$ sh file.sh

# Ottenere la lista dei processi attivi nel sistema della sessione interattiva corrente
$ ps    # (process status)
# voci dell'output:
# PID: Process ID
# TTY: terminale
# TIME: formattato in ore:minuti:secondi corrisponde al tempo della CPU
# CMD: nome del comando eseguito
    # options:
    -f              # (-full) fornisce maggiori informazioni sui processi attivi aggiungendo
                    # le seguenti voci:
                    # UID: User ID
                    # PPID: Parent Process ID
                    # C:
                    # STIME:
    -l              # (-long) fornisce altre informazioni sui processi attivi aggiungendo
                    # le seguenti voci:
                    # S: (Status) definisce se il programma è in stato sospeso (S) o running/ready (R)
                    # PRI:
                    # NI:
                    # ADDR:
                    # SZ:
    -e              # (-every) consente di visualizzare tutti i processi attivi nel sistema

# PROCESSI NOTEVOLI:
#   creato al BOOT: maybe-ubiquity (percorso: /sbin/init)
#   gestore MEMORIA VIRTUALE: [kswapd0] (k=kernel, swap=termine usaato in modo improprio, d=daemon)
#   processo creato da init per gestire un TERMINALE FISICO: percorso: /sbin/getty oppure /sbin/agetty

# Ottenere la lista dei file e directory contenuti nella directory corrente
$ ls [dir]      # (list) se passato l'argomento dir, verrà fatta la lista del suo contenuto
                # anziché della directory corrente
    # options:
    -l              # (-log) lista, oltre dei nomi, di tutte le informazioni associate
                    # ai file (tipo del file, permessi, numero link, proprietario
                    # gruppo, dimensione in byte, ultima modifica)
    -a              # (-all) lista anche dei nomi dei file nascosti, cioè quei
                    # file il cui nome inizia con "."
    -A              # (-All) funziona come -a, ma esclude . e ..
    -F              # lista dei nomi dei file visualizzando gli eseguibili
                    # con il suffisso "*" e le directory con "/"
    -d nomedir      # lista delle informazioni associate alla directory "nomedir"
                    # considerandola quindi come un file, senza stamparne dunque il contenuto
    -R              # (-Recursive) lista ricorsiva dei file contenuti nella gerarchia a partire
                    # dalla directory corrente a tutti i file o directory contenuti 
                    # in essa (fa la lista dei contenuti di eventuali altre
                    # sottodirectory ricorsivamente)
    -i              # lista degli i-number dei file (oltre al loro nome)
    -r              # (-reverse) lista dei file in ordine opposto al normale ordine alfabetico
    -t              # (-time) lista dei nomi dei file in ordine di ultima modifica
                    # ordinati dal più recente al meno recente
    
# Leggere il manuale relativo a un determinato comando
$ man comando       # (manual)
    # options:
    -w          # (where) per sapere dove si trova il manuale di un comando
                # (equivalente del comando whereis, con un output leggermente diverso)

# Conoscere il path corrente
$ pwd           # (print working directory)

# Creare una directory
$ mkdir dir

# Cancellare una directory
$ rmdir dir
# questo comando ha effetto solo se non contiene nessun file
# (quindi contiene solo i file "." e ".." creati dal sistema)

# Spostarsi dalla directory corrente al percorso destinazione specificato
$ cd destinazione       # (change directory)
$ cd                    # questa variante conduce direttamente alla home

# Stampare in output la stringa consegnata come parametro
$ echo stringa
$ echo $HOME        # ottengo in output il contenuto della variabile HOME
$ echo ciao?        # ottengo in output tutti i file o directory contenuti nella
                    # directory corrente che si chiamano ciaostringaqualsiasi. 
                    # Se non vi è match con nessun file, allora viene stampata in output
                    # la stringa "ciao?". Lo stesso vale per le stringhe contenenti "*"
$ echo $?           # in ouput stampa un intero che determina se il comando precedente ha avuto
                    # successo (0) o insuccesso (qualunque numero diverso da 0)
$ echo -n           # non termina la stringa stampata con il \n

# Modifica la data e l'ora di ultimia modifica di un file. Se il file non esiste, viene creato
$ touch file

# Lavorare su un file con editor di testo
$ vi file
# se sei sano di mente usa questo editor di testo
$ nano file

# Riporta le linee che presentano delle differenze tra due file (l'output varia a seconda dell'ordine di inserimento dei file)
$ diff file1 file2
    # options:
    -c          # (context) riporta le linee che presentano delle differenze tra i due file
                # e le linee circostanti
    -u          # (unified) riporta le linee che presentano delle differenze tra i due file
                # e le linee circostanti in un formato più compatto
    -r          # (recursive) se i file sono directory, allora la comparazione è ricorsiva

# Riporta tutti i nomi assoluti del file di cui viene fornito il nome a partire dalla directory specificata
$ find dir -name file

# Cambiare i permessi relativi a un file
$ chmod [u | g | o | a][+ | -][r | w | x]      # (change mod)
# consente di conferire (+) o negare (-) a:
#   u = user 
#   g = group
#   o = others
#   a = all
# un certo permesso di:
#   r = read
#   w = write
#   x = execute
# al posto di [u | g | o | a] [+ | -][r | w | x] è possibile utilizzare numeri
# in ottale che verranno convertiti in binario dal sistema
# esempi:
$ chmod 600 file       # in ottale conferisce il diritto di lettura all'utente e nessun diritto a group e others
$ chmod 640 file       # in ottale conferisce il diritto di lettura all'utente, scrittura a group e nessun diritto a others

# (Riservati al superuser) cambiano rispettivamente il proprietario o il gruppo proprietario del file
$ chown user file       # (change owner)
$ chown group file

# Copiare un file in un'altra directory
$ cp source destination       # (copy)
    # options:
    -p          # (preserve) nella copia conserva i dati timestamps e ownership
    -r          # (recursive) indica che la copia deve essere ricorsiva, dunque
                # si sottointende che si vuole copiare una directory

# Creare un link tra due file
$ ln file1 file2       # (link)
    # options: 
    -s          # Instaura un link software

# Spostare o rinominare un file
$ mv source destination       # (move) spostare
$ mv file newname           # rinominare

# Rimuovere un file
$ rm file           # (remove)
# può essere utilizzato lo stesso comando per eliminare una directory
$ rm dir
    # options:
    -i          # (interactive) chiede all'utente una conferma prima di eliminare il file
    -r          # (recursive) indica che l'eliminazione deve essere ricorsiva ed 
                # è una options necessaria se la directory è vuota

# Metodo molto comodo per creare un file vuoto o vuotarne uno esistente
$ > file        # (viene ridirezionata una stringa vuota "" nel file)

# sort è comando-filtro, ma esiste anche la versione comando
# Ordina le linee dello standard input
$ sort          # versione comando-filtro
$ sort file     # versione comando
    # options:
    -f          # (fold) le maiuscole non vengono differenziate dalle minuscole durante l'ordinamento
    -r          # (reverse) l'ordine alfabetico è invertito
    -c          # (check) il comando si ferma alla prima parola fuori ordine trovata e
                # il suo ouput viene stampato su stderr
    -C          # funziona come la precedente, ma non stampa la prima linea fuori ordine
    -u          # (unique) elimina i duplicati

# Promuovere una variabile di shell a variabile d'ambiente 
$ export variabile
# tutte le volte che una variabile d'ambiente è stata modificata, per ragioni storiche, conviene rifare l'export.
# L'operazione di promozione ha visibilità verso il basso, ma non verso l'alto! Le modifiche a una variabile d'ambiente
# effettuate in un sottoprocesso saranno inefficaci per il processo padre

# grep è comando-filtro, ma esiste anche la versione comando
# Cerca un certo pattern (caratteri vicini, non per forza parole) nella stringa o nel file specificato
# e ne stampa la linea corrispondente
$ grep pattern string   # (general regular expression print) versione comando-filtro
$ grep pattern file     # versione comando
    # options:
    -n          # (number) a fianco della linea stampa il suo numero di riga
    -i          # (ignore case) non fa differenza tra maiuscole e minuscole (non è case sensitive)
    -v          # (invert match) inverte il senso del comando, quindi stampa tutte le linee
                # che NON contengono il pattern specificato
    -c          # (count) conta le occorrenze del pattern
# Il pattern può anche essere specificato tra apici per evitare errori involntari: 'pattern'
# Per cercare tutte le righe che iniziano con un certo pattern: '^pattern'
# Per cercare tutte le righe che terminano con un certo pattern: 'pattern$'
# Per riportare le righe che terminano con il carattere '.' o altri caratteri speciali serve l'escape: '\.$'

# wc è comando-filtro, ma esiste anche la versione comando
# Conta linee, parole e caratteri dello standard input
$ wc string         # (word count) versione comando-filtro
$ wc file           # versione comando
    # options:
    -l              # (lines) stampa solo il numero di linee
    -c              # (characters) stampa solo il numero di caratteri
    -w              # (words) stampa solo il numero di parole
# se non si specifica nessuna opzione, allora conterà, secondo il seguente ordine, il numero di 
# linee, parole e caratteri
# Si sottolinea che quando si usa wc come comando (quindi non filtro) oltre al suo normale output, viene riportato anche
# il nome del file specificato

# head è comando-filtro, ma esiste anche la versione comando
# Riporta le prime -numerolinee linee (se esistono). Se -numerolinee non è specificato
# vengono riportate fino a 10 linee di default
$ head -n number string       # versione comando-filtro
$ head -n number file         # versione comando
# dove number è il numero di linee che voglio selezionare in testa

# tail è comando-filtro, ma esiste anche la versione comando
# Riporta le ultime -numerolinee linee (se esistono). Se -numerolinee non è specificato
# vengono riportate fino a 10 linee di default
$ tail -n number string       # versione comando-filtro
$ tail -n number file         # versione comando
# dove number è il numero di linee che voglio selezionare in coda

# rev è comando-filtro, ma esiste anche la versione comando
# Rovescia le linee dello standard input (in alcune distribuzioni questo comando è stato rimosso)
$ rev string        # versione comando-filtro
$ rev file          # versione comando

# Compilare programmi in C via linea di comando 
$ gcc -o nomeoutput -Wall       # GNU C Compiler
# se non viene definita la options -o con il corrispondente nomeouput, l'output della 
# compilazione verrà chiamato "a.out"
# -Wall serve per attivare tutti i warning

##### PRIMA DI LANCIARE UN FILE COMANDI CONFERIRE SEMPRE I DIRITTI DI ESECUZIONE! ##### (SONO DISATTIVATI DI DEFAULT!) #####
# Lanciare un file comandi
$ file.sh       # prima di lanciarlo magari assicurarsi che la shell che si sta usando corrisponda con l'estensione
# se la directory corrente non è salvata nella variabile d'ambiente PATH
$ ./file.sh         # . sarà sempre nel PATH siccome è un autolink alla directory corrente
# se si desidera invocare una specifica shell per eseguirlo
$ sh file.sh        # chiaramente la shell deve essere quella corrispondente all'estensione

# Visualizzare l'ambiente corrente di un processo shell
$ env

# tee è un comando-filtro
# Passa il contenuto di stdin sia su stdout che sul file
$ tee file      # (pipe tee, in italiano raccordo a T)

# Siccome le variabili contengono solo stringhe, per eseguire operazioni matematiche si usa expr
$ i=12
$ j=`expr $i + 1`
# j contiene ora la stringa 13

# Pseudovariabili
$*      # insieme di tutte le variabili posizionali che corrispondono a tutti gli
        # argomenti del comando ($0 escluso)
$#      # numero di argomenti passati al comando
$?      # valore di ritorno dell'ultimo comando eseguito (0 = successo, numero positivo = insuccesso)
$$      # numero del processo in esecuzione

# Aggiungere un valore al PATH (ad esempio pwd)
PATH=`pwd`:$PATH
# Per far sì che la variabile diventi d'ambiente
export PATH

# Controlli di flusso
# Nei casi di errore uscire sempre con il comando 'exit n' dove n è un intero positivo
# if
if lista-comandi
then comandi
[else comandi]
fi
# equivalente
if lista-comandi; then comandi; [else comandi;] fi

# Valutazione di un'espressione che ha come valore di ritorno 0 in caso di successo
# o un numero positivo in caso di insuccesso
$ test -opzione-obbligatoria file-o-directory
    # options:
    -f          # verifica l'esistenza di un file
    -d          # verifica l'esistenza di una directory
    -r          # verifica il diritto di lettura sul file o directory
    -w          # verifica il diritto di scrittura sul file o directory
    -x          # verifica il diritto di esecuzione sul file o directory
    -a          # and logico
    -o          # or logico
# verificare se due stringhe sono uguali o diverse
$ test str1 = str2
$ test str1 != str2
# verifica se la stringa è vuota
$ test -z str
# verifica se la stringa NON è vuota
$ test str
# confronta tra loro due stringhe numeriche usando uno degli operatori relazionli indicati
$ test numero1 [-eq -ne -gt -ge -lt -le] numero2
# tutte le opzioni del comando test possono essere utilizzate con la negazione "!"

# Richiedere all'utente di inserire delle stringhe
$ read var1 var2 var3   # le stringhe inserite verranno salvate nelle variabili var1, var2 e var3

# Alternativa multipla
$ case $var in
pattern-1) comandi;;        # i pattern devono essere stringhe
...
pattern-i | pattern-j | pattern-k) comandi;;    # | sta per or logico
...
pattern-n) comandi;;
esac

# Cicli
# per continuare o uscire da un ciclo, come in C, si usano 'continue' e 'break'
# for
for var [in list]
do
comandi
done
# caso particolare
for i in pattern      # pattern viene espanso come "seleziona tutti i file contenuti nella directory che fanno match con tale pattern"
do
comandi
done
# caso ordinario
for i in `cat file`     # dove file contiene le stringhe che verranno usate dal for
do
comandi
done

# while
while lista-comandi     # l'iterazione continua finchè il valore di ritorno dell'ULTIMO COMANDO DELLA LISTA ha successo
do
comandi
done
# alternativa opposta al while
until lista-comandi     # l'iterazione continua finchè il valore di ritorno dell'ULTIMO COMANDO DELLA LISTA ha INsuccesso
do
comandi
done