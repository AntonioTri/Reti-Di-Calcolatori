    /*
        Di seguito troverai il codice per quanto riguarda il client del progetto di reti di calcolatori. Questo progetto permette di avviare un file che si comporta come un client e di passare da terminale i propri file con 
        La criptazione o decriptazione che si vuole ottenere.Il Client si connette ad un server centrale che accetta la connessione e genera un processo figlio che gestisce la richiesta di quel client.
        1 febbraio 9:39
    */

    //Inclusione delle librerie necessarie

    #include <sys/types.h>
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <sys/socket.h>
    #include <stdio.h>
    #include <string.h>
    #include <stdlib.h>
    //Size massimo di ogni buffer
    #define MAXLINE 256

    //Funzioni utili che dovrò mettere in una libreria

    //Apre tutti i file presenti nella lista di argomenti.

    FILE *Apertura_File(FILE *file[],char *argv[],int argc);

    //Converte il doppio puntatore argv in un array di caratteri che restituisce
    char *Conversione_Argv(char *argv[],int argc);

    //Questa funzione restituisce il numero totale di file presenti in un array di file
    int getTotalNumberOfFiles(FILE *files[]);

    //Funzione per inviare un certo numero di file al server
    int Invio_File(FILE *file[],int socket, char criptazioni[3]);

    //Main
    int k=0;

    int main(int argc, char *argv[])
    {

        printf("argv[0]: %s \nargv[1]: %s \n",argv[0],argv[1]);
        //Dichiarazione di tutte le variabili che saranno utili piu avanti

        int sock_fd;
        struct sockaddr_in server_addr;
        int i,j;
        char buffer_input[MAXLINE];
        char buffer_argomenti[MAXLINE];
        FILE *file_client[MAXLINE];

        //Creazione della socket

        if ((sock_fd=socket(AF_INET, SOCK_STREAM, 0)) < 0) 
        {
            perror("Socket creation error");
            exit(EXIT_FAILURE);
        }

        // Inizializzazione dell'indirizzo del server a cui il client si connetterà

        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(6000);
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

        // Connessione al server
        if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) 
        {
            perror("Connection error");
            exit(EXIT_FAILURE);
        }

        //Inizio del loop per dare all'utente la possibilità di fare cose diverse

        while(1)
        {
            printf("Connessione effettuata correttamente al server.\nScegli una delle seguenti opzioni:\n0. Criptazione\n1. Esci\n");
            //Se l'utente non ha immesso nessun carattere gli verrà chiesto nuovamento di iserirlo.

            if(fgets(buffer_input, MAXLINE,stdin)==NULL)
            {
                continue;
            }else
            {
                //In base all'input dell'utente si fanno cose diverse

                switch (buffer_input[0])
                {
                case '0':
                    //Effeutto la conversione di argv e mi salvo quest'ultima nel buffer_argomenti
                    fflush(stdout);
                    //Eseguo l'apertura dei file e li salvo nell'array di file file_client
                    
                    *file_client=Apertura_File(file_client,argv,argc);
                    //Verifico quanti file ci sono
                    
                    printf("Numero totale dei file: %d \n",k);

                    //Invio array contenente i nomi di tutti i file con la relativa criptazione

                    char buffer_argv[MAXLINE]={'\0'};
                    char criptazioniScelte[3];
                    int z = 0;
                    // Copia gli argomenti nel buffer

                    for (int i = 1; i < argc; ++i) 
                    {
                        strncat(buffer_argv, argv[i], sizeof(buffer_argv) - strlen(buffer_argv) - 1);

                        // Aggiungi uno spazio tra gli argomenti, se non è l'ultimo
                        if (i < argc - 1) 
                        {
                            strncat(buffer_argv, " ", sizeof(buffer_argv) - strlen(buffer_argv) - 1);
                        }

                        //Gli argomenti di argv di posizione dispari contengono le criptazioni
                        //Quindi partento da i = 1, se prendiamo le posizioni pari avremo le criptazioni scelte
                        if (i % 2 == 0)
                        {
                            strcpy(&criptazioniScelte[z], argv[i]);
                            printf("Criptazione registrata %c.\n", criptazioniScelte[z]);
                            z++;
                        }
                        
                            
                    }

                    printf("Il buffer degli argomenti vale: %s\n",buffer_argv);

                    ssize_t nwrite_size = send(sock_fd,buffer_argv,strlen(buffer_argv), 0);
                    if (nwrite_size < 0) 
                    {
                        perror("Write error");
                        break;
                    }

                    //Il client ora deve fare un'altra write e per evitare che scriva tutto insieme è necessario inserire una sleep per dare il tempo al server di passare alla prossima read
                    sleep(2);
                    
                    Invio_File(file_client,sock_fd, criptazioniScelte);
                    
                    //Il client ora rimane in attesa di ricevere la risposta sal derver
                    //Modificata
                    printf("\n[...]In attesa di una risposta.\n");
                    char responseBuffer[1024];
                    int b = 1;
                    while (1)
                    {
                        recv(sock_fd, responseBuffer, MAXLINE, 0);
                        printf("[+]Una risposta è arrivata dal server!\n-\"%s\"\n\n", responseBuffer);
                        
                        char nomefile[12];
                        sprintf(nomefile, "output%d.txt", b);
                        FILE *fp = fopen(nomefile, "w");

                        if (strcmp(responseBuffer, "luigi") == 0){
                            printf("[+]Ho ricevuto tutti i file indietro, posso in morire in pace, finalmente.\n");
                            fclose(fp);
                            break;
                        }

                        if (fwrite(responseBuffer, sizeof(char), strlen(responseBuffer), fp) < 0){
                            close(sock_fd);
                            exit(EXIT_FAILURE);
                        }
                        
                        memset(responseBuffer, 0, sizeof(responseBuffer));
                        send(sock_fd,"Fatto",strlen("fatto"), 0);
                        fclose(fp);
                        b++;

                    }
                    

                    printf("[-]Chiusura del client.\n");
                    exit(EXIT_SUCCESS);
                    close(sock_fd);

                    break;
                
                default:
                    printf("[!]Hai inserito un numero non presente nella lista. Riprova.\n");
                    break;
                }

            }

        }

    }


    //Funzioni utilzzate nel codice che dovrò mettere in una libreria

    /*
        La funzione utilizzata per l'apertura dei file in base ad una lista di file che sono posti in modo alternato
        prende 3 argomenti,ossia l'array di file da riempire,l'array con all'interno i nomi dei file disposti in modo alternato e infine il size complessivo di quest'ultimo array.
        
    */
    FILE *Apertura_File(FILE *file[],char *argv[],int argc)
    {
        
        //Indice per gli argomenti che parte da uno poichè il primo argomento si suppone che sia il nome del file da cui si avvia il codice
        int j=1;
        //Apertura file in lettura 
        //Il while viene effettuato finché l'indice j non è arrivato all'ultimo nome del file.
        while (j<argc-1)
        {
            //Al k-esimo passo il file k viene aperto in lettura e gli si da il j-esimo nome.        
            file[k]=fopen(argv[j],"r");
            printf("k: %d, j: %d, argc-1: %d\n",k,j,argc-1);
            
            fflush(stdout);
            //Se il file non è stato aperto correttamente si esce con un errore
            if(!file[k])
                    {
                        printf("Qualcosa è andato storto nell'apertura del file %d \n",k);
                        fflush(stdout);
                        exit(1);
                    }
                    //k viene in crementato di 1 mentre j di due poiché deve sempre saltare l elemento "inutile" e passare al prossimo nome
                    k++;
                    j=j+2;
                    
                }

            //Restituisci l'array di file
            return *file;
    }

    /*
    Questa funzione ci servirà per inviare una serie di file al server o al client
    restituisce 1 se l'operazione è andata a buon fine e 0 se è andata male. Prende
    come parametri l'array di file,iò numero di file presente in quest'ultimo e la socket attraverso cui inviare i dati
    */
    
    int Invio_File(FILE *file[],int socket, char criptazioni[3])
    {
        int j=0;
        char buffer[MAXLINE];
        char supportBuffer[MAXLINE];
        int h=0;
        //Itera su tutti i file finché non sono finiti
        while (j<k){
            int nroIterazioni = 0, haFinitoIlFile = 0;
            //Al j-esimo passo viene inserito il contenuto del file j all'interno di un buffer
            while (fgets(buffer, sizeof(buffer), file[j]) != NULL) 
            {
                printf("Invio %ld elementi. Sizeof %ld \n",strlen(buffer),sizeof(buffer));
                //Il buffer contenente il contenuto del file viene inviato al server
                ssize_t nwrite = send(socket,buffer, sizeof(buffer), 0);
                sleep(2);
                //Se c'è un errore su di un file viene segnalato
                if (nwrite < 0) 
                {
                    printf("Write error on file %d",j);
                    exit(1);
                }

                nroIterazioni++;
            }

            //Se numero iterazioni è maggiore di 1 vuol dire che ha fatto
            //Più send legate allo stesso file, il segnale eof pertanto 
            if (nroIterazioni == 1){
                haFinitoIlFile = 1;
            } else if (nroIterazioni > 1 && strlen(buffer) == 0){
                haFinitoIlFile = 1;
            }
            
               
            
            printf("[+]Ho finito di inviare il file %d\n",j);
            //Inviamo la criptazione
            char criptazione = criptazioni[j];
            printf("[+]Invio crpitazione tipo '%c' per il file '%d'\n", criptazione, j);
            send(socket, &criptazione, sizeof(criptazione), 0);
            //Dopo una sleep per dare tempo al server, viene inviata la criptazione corrispondente
            sleep(1);

            //se il numero di file è uguale ad 1 oppure se ci troviamo nell'invio dell'ultimo file
            //Non inviamo il segnale eof per evitare che il server vada in loop.
            //Nel buffer verrebbe riscritto "EOF" prima di "exit" e quindi non verrebbe letto
            
            if ((j == 0 || j == k-1) || haFinitoIlFile){
                printf("[+]Invio del segnale di terminazione file\n");
                send(socket,"EOF",sizeof("EOF"),0);
            } 
            sleep(1);
            fflush(stdout);

            //printf("Stampo il buffer corrente %s \n",buffer);
            //Si passa al file successivo
            j++; 
        }
        //Sto per inviare la terminazione
        printf("[+]Invio il segnale di terminazione\n");
        send(socket,"exit",sizeof("exit"),0);
    
    }




    






