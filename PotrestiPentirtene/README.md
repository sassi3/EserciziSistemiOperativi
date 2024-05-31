# EserciziSistemiOperativi

Esercizi sugli script shell, programmazione di sistema in C e utilizzo di make

## Visual Studio Code extentions

Su Visual Studio Code è disponibile un buon spell checker: `ShellCheck`. Questa estensione segnalerà (giustamente) la sostituzione `` `comando` `` come deprecated e consiglierà di sostituirla con la notazione `$(comando)`.
\
Siccome lo spell checker traccia una barra sul testo, esso risulta un po' invasivo. Se si desidera usare la notazione `` `comando` `` (mi dispiace per te), è possibile disattivare questo warning andando nella pagina dell'estensione, premere sull'ingranaggio a fianco del tasto "Disinstalla", aprire "Impostazioni dell'estensione" e aggiungere alla lista "Exclude" il codice warning `SC2006`.

Un'altra comoda estensione, preferibile se si lavora con la bash, è `shell-format`. Essa consente di formattare i file di script sh, bash e zsh con la combinazione di tasti `CTRL` `Shift` `I`. Se questa shortcut non dovesse funzionare, allora aprire la command palette (`CTRL` `Shift` `P`) e lanciare il comando `Format Document`.
\
**Attenzione**: se il file contiene errori, allora non sarà possibile formattarlo prima di averli corretti tutti.
