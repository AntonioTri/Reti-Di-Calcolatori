
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#define BACKLOG 10
#define MAXLINE 256
#define MAX_CLIENTS 10

typedef struct{

    char buffer[100*MAXLINE];
    int criptazioneScelta;


} CollezioneDati;

typedef struct{

    int clientSocket;
    int nroFile;
    int handledRequests;
    int endedRequests;
    CollezioneDati collezionedati[10];

} ClientData;

void handleRequest(int secondarySocket, ClientData *clientData, int i, int choice, fd_set *active_fds, int *clientSockets, int *numberOfConnecteClients) {
    
        fflush(stdout);

        int clientSocket = clientData[i].clientSocket;
        char tempBuffer[MAXLINE];
        size_t lenght = strlen(clientData[i].collezionedati[choice].buffer);
        // Copia dei dati nel buffer di supporto
        memcpy(tempBuffer, clientData[i].collezionedati[choice].buffer, lenght);
        memset(clientData[i].collezionedati[choice].buffer, 0, lenght);

        send(secondarySocket, tempBuffer, lenght, 0);
        recv(secondarySocket, clientData[i].collezionedati[choice].buffer, MAXLINE, 0);
        sleep(i);
        printf("\n\n[!]Messaggio ricevuto dal server terzo:\n-\"%s\"\n\n", clientData[i].collezionedati[choice].buffer);
        fflush(stdout);
        clientData[i].endedRequests++;

        //chiusura della connessione
        if (clientData[i].endedRequests == clientData[i].nroFile - 1){
            
            sleep(2);
            printf("[-]Tutti i file sono stati criptati, inizio l'invio.\n");

            for(int l = 0; l< clientData[i].nroFile - 1; l++){
                
                printf("[.]Invio del messaggio: \n%s\n",clientData[i].collezionedati[l].buffer);
                send(clientData[i].clientSocket, clientData[i].collezionedati[l].buffer, strlen(clientData[i].collezionedati[l].buffer), 0);
                printf("\n[?]In attesa del segnale di ricezione avvenuta...\n");
                recv(clientData[i].clientSocket, tempBuffer, MAXLINE, 0);
                printf("[+]Ricezione file %d da parte del client avvenuta con successo!\n", l + 1);
                sleep(1);

            }

            printf("\n[+]Invio dei file avvenuto con successo. Chiusura della connessione...\n\n");
                
                send(clientData[i].clientSocket, "luigi", strlen("luigi"), 0);

                sleep(1);
                close(clientData[i].clientSocket);
                FD_CLR(clientData[i].clientSocket, *(&active_fds));
                
                clientData[i].clientSocket = 0;
                clientData[i].endedRequests = 0;
                clientData[i].handledRequests = 0;
                for (int d = 0; d < clientData[i].nroFile; d++){
                    clientData[i].collezionedati[d].criptazioneScelta = 0;
                    memset(clientData[i].collezionedati[d].buffer, 0, sizeof(clientData[i].collezionedati[d].buffer));
                }
                clientData[i].nroFile = 0;
                clientSockets[i] = -1;
                *numberOfConnecteClients -= 1;

        }
       
}


int main() {


    struct timeval timeout;
    int server_socket, client_sockets[MAX_CLIENTS];
    int secondaryServer1, secondaryServer2, secondaryServer3;
    struct sockaddr_in server_addr, client_addr, secondaryAddress1, secondaryAddress2, secondaryAdress3;
    socklen_t client_len;
    socklen_t addrSize = sizeof(struct sockaddr_in);
    fd_set read_fds, active_fds;
    int h=0;
    timeout.tv_sec = 15; // Imposta il timeout a 5 secondi
    timeout.tv_usec = 0;
    char argomenti[MAXLINE];

    // Creazione del socket del server
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    // Inizializzazione dell'indirizzo del server
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(6000);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Binding del socket del server
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind error");
        exit(EXIT_FAILURE);
    }

    // Impostazione del socket in modalità di ascolto
    if (listen(server_socket, BACKLOG) < 0) {
        perror("Listen error");
        exit(EXIT_FAILURE);
    }

    printf("[+]Server listening on port 65000...\n");
    printf("[-]Waiting for Secondary Servers to connect...\n");
    secondaryServer1 = accept(server_socket, (struct sockaddr*)&secondaryAddress1, &addrSize);
    printf("[+]Connessione accettata da Server Secondario 1\n");
    secondaryServer2 = accept(server_socket, (struct sockaddr*)&secondaryAddress2, &addrSize);
    printf("[+]Connessione accettata da Server Secondario 2\n");
    sleep(1);
    printf("[-]Waiting for Clients to connect...");

    FD_ZERO(&active_fds);
    FD_SET(server_socket, &active_fds);
    ClientData clientData[MAX_CLIENTS];
    //Questidue elementi ci aiutano a tenere traccia dei client attualmente serviti
    //E di quanti client sono connessi
    int clientService[MAX_CLIENTS];
    int numberOfConnectedClients = 0;

    int max_fd = server_socket;

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        client_sockets[i] = -1;
        clientService[i] = 0;
    }

    while (1) {

        read_fds = active_fds;

        // Utilizzo di select per gestire gli eventi su tutti i socket aperti
        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) {
            perror("[!]Select error: ");
            exit(EXIT_FAILURE);
        }

        printf("[-]Aspetto la connessione di un Client...\n");
        // Verifica se il socket di ascolto ha attività (nuova connessione)
        if (FD_ISSET(server_socket, &read_fds)) {
            client_len = sizeof(client_addr);
            fflush(stdout);
            int new_client = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);

            if (new_client < 0) {
                perror("Accept error");
                exit(EXIT_FAILURE);
            }

            // Aggiungi il nuovo client alla lista
            int i;
            for (i = 0; i < MAX_CLIENTS; ++i) {
                if (client_sockets[i] == -1) {
                    client_sockets[i] = new_client;
                    clientData[i].clientSocket = new_client;
                    numberOfConnectedClients++;
                    break;
                }
            }

            // Aggiorna il massimo descrittore di file se necessario
            if (new_client > max_fd) {
                max_fd = new_client;
            }

            FD_SET(new_client, &active_fds);
            printf("\n[+]Nuova connessione da un client accettata!\n");
        }

        
        // Gestisci l'input da tutti i client
        for (int i = 0; i < numberOfConnectedClients; ++i) {
            int nroFile = 0;
            int client_socket = client_sockets[i];
           
            if (client_socket != -1 && !clientService[i] && FD_ISSET(client_socket, &read_fds)) {
                printf("\n-----------------------------------------------\n\n[+]Gestisco il client: %d \n",i);

                char buffer[MAXLINE];
                char buffer_argv[MAXLINE];

                //ricezione dell'array contenente i nomi dei file e il relativo decriptaggio
                ssize_t nread_argv;
                //Si memorizza nell'array di data la socket
                clientData[i].clientSocket = client_socket;
                //Si effettua una simulazione dell'HandShake
                nread_argv = recv(client_socket, buffer_argv,sizeof(buffer_argv),0);
                printf("[+]Client Socket: '%d'\n", client_socket);
                if (nread_argv < 0) {
                    perror("Errore nella lettura degli argomenti");
                    exit(1);
                } else {
                    printf("letti: %ld\n",nread_argv);
                }

                printf("[-]Decrzione degli elementi in arrivo: \"%s\"\n",buffer_argv);
                
                //Qui ci dovrebbe essere un ciclo che itera su  tutti i file
                //Testing
                
                while (1) {   
                    //Effettuiamo la recv per gestire tutte le casistiche
                    ssize_t nread = recv(client_socket, buffer, sizeof(buffer), 0);
                    
                    if (nread < 0) {

                        perror("Read error");
                        close(client_socket);
                        FD_CLR(client_socket, &active_fds);
                        client_sockets[i] = -1;
                        printf("Client %d disconnected\n", i);

                    //Chiusura connessione
                    } else if (nread == 0) {
                        // Connessione chiusa dal client
                        close(client_socket);
                        FD_CLR(client_socket, &active_fds);
                        client_sockets[i] = -1;
                        printf("Client %d disconnected\n", i);

                    //Si legge il buffer ricevuto se non ci sono stati casi eccezionali e si gestiscono i casi
                    } else if (nread > 0){

                        //Caso in cui sia stata inviato il discriminanate
                        if(strcmp(buffer, "0") == 0 || strcmp(buffer, "1") == 0 ||strcmp(buffer, "2") == 0){
                            printf("[+]Criptazione ricevuta: %s\n", buffer);
                            clientData[i].collezionedati[nroFile].criptazioneScelta = atoi(buffer);
                            printf("[+]Criptazione scleta per il file %d: %d\n\n", nroFile, clientData[i].collezionedati[nroFile].criptazioneScelta);
                            memset(buffer, 0, sizeof(buffer));

                        //Aumento il contatore dei file se è stato inviato tutto il file corrente
                        } else if (strcmp(buffer,"EOF")==0) {
                            printf("[+]Segnale di fine file 'EOF' ricevuto.\n\n");
                            nroFile++;
                            memset(buffer, 0, sizeof(buffer));

                        //Caso di ricevimento del segnale di fine file
                        } else  if(strcmp(buffer,"exit")==0){
                            
                            printf("[+]Segnale di completamento invio file ricevuto.\n[+]Proseguo con l'handling delle richieste.\n\n");
                            clientData[i].nroFile = nroFile + 1;
                            clientData[i].clientSocket = client_socket;
                            clientData[i].handledRequests = 0;
                            clientData[i].endedRequests = 0;
                            break;

                        //Caso base in cui si concatena la stringa ricevuta al file già presente
                        } else {

                            strcat(clientData[i].collezionedati[nroFile].buffer, buffer);
                            printf("[+]Contenuto iterazione '%d':\n%s\n", h, clientData[i].collezionedati[nroFile].buffer);
                            memset(buffer, 0, sizeof(buffer));
                            h++;
                            
                        }
                    }
                }

                printf("[+]Caratteristiche registrate del client '%d' con socket = '%d'\n", i, clientData[i].clientSocket);
                printf("- Socket: %d.\n- Numero File appartenenti alla socket: %d.\n", clientData[i].clientSocket, clientData[i].nroFile);

            }

            //Si esegue un ciclo for per un numero di volte pari al numero di file ottenuti dal client iesimo
            for (int f = 0; f < clientData[i].nroFile - 1; f++){
                printf("\n[-]Controllo del client %d sul suo file %d\n", i, f);
                //Se al client corrente corrisponde una richiesta già attiva non ne viene creata
                //Una nuova e si passa al successivo

                if (!clientService[i]){
                    //Per ogni richiesta si incrementa un contatore personale
                    printf("[?]Caso scelto: %d\n", clientData[f].collezionedati[i].criptazioneScelta + 1);
                    //Lo switch viene eseguito sulla decriptazione del file corrente. L'indice i rimane quello del for
                    //in cui si trova questo secondo for. Mandiamo come argomento la socket del server di arrivo,
                    //i dati da inviare, la lunghezza di questi dati ed il caso in cui ci troviamo
                    switch (clientData[i].collezionedati[f].criptazioneScelta + 1) {
                    
                        case 1:
                            clientData[i].handledRequests++;
                            printf("[+]Creo una nuova richiesta caso %d\n", clientData[i].collezionedati[f].criptazioneScelta + 1);
                            handleRequest(secondaryServer1, clientData, i, f, &active_fds, client_sockets, &numberOfConnectedClients);
                            break;

                        case 2:
                            clientData[i].handledRequests++;
                            printf("[+]Creo una nuova richiesta caso %d\n", clientData[i].collezionedati[f].criptazioneScelta + 1);
                            handleRequest(secondaryServer2, clientData, i, f, &active_fds, client_sockets, &numberOfConnectedClients);
                            break;

                        case 3:
                            clientData[i].handledRequests++;
                            printf("[+]Creo una nuova richiesta caso %d\n", clientData[i].collezionedati[f].criptazioneScelta + 1);
                            handleRequest(secondaryServer1, clientData, i, f, &active_fds, client_sockets, &numberOfConnectedClients);
                            break;

                        default:
                            printf("[!]Scelta non valida. Riprova.\n");
                            continue;
                    }

                    sleep(3);

                } else {
                    printf("[+]Tutte le richieste del client %d sono state esaurite.\n[!]Nro Richieste esaurite: %d\n\n-----------------------------------------------\n\n", i, clientData[i].handledRequests);
                    
                    //Si resettano tutti i buffer quando i campi della struct
                    //Quando tutte le richieste sono state esaurite
                    clientService[i] = 0;
                }

                //Se il numero di richieste esaudite corrisponde al numero di file mandati allora 
                //Si segnala con una flag che quel client non ha altre richieste pendenti
                if (clientData[i].handledRequests == clientData[i].nroFile - 1){
                    printf("\n[!]Client corrente segnalato come servito!\n\n");
                    clientService[i] = 1;
                }
            }
        }
    }

    // Chiusura del socket del server
    close(server_socket);

    return 0;
}

