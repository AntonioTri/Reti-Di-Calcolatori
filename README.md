# Progetto di Reti di Calcolatori
Come far partire il programma:
1. Scaricare i 4 file presenti nella cartella "File".
2. Posizionali in una cartella e compilarli con i comandi gcc.
3. Avviare il "Server Centrale".
4. Avviare a catena (l'ordine non è importante) i server secondari.
5. Il server centrale restituirà feedback ad ogni connessione effettuata con successo
6. Avviare il client con un comando di questo tipo < ./client prova.txt 0 prova2.txt 1 >
   Ad ogni nome del file deve essere associato un numero che varia da 0 al umero di server secondari
   connessi meno 1 (es. numeroServerSecondari = 3, criptazioneMassima = 2).
7. Possono connettersi diversi client e fargli esaurire richieste diverse. 
