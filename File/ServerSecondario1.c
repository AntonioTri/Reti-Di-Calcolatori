#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 6000
#define BUFFER_SIZE 1024

struct ThreadData {
    char bufferCopy[BUFFER_SIZE];
    int centralServerSocket;
};

void reverseString(char* str) {
    int start = 0;
    int end = strlen(str) - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

void *elaboraRichiesta(void* arg) {
    struct ThreadData *threadData = (struct ThreadData*)arg;

    // Creare una copia locale dei dati all'interno del thread
    struct ThreadData localData;
    memcpy(&localData, threadData, sizeof(struct ThreadData));

    printf("[+]Thread Creato.\n[-]Frase ricevuta dal thread:\n%s\n", localData.bufferCopy);

    // Inverti la stringa
    reverseString(localData.bufferCopy);
    printf("[-]Frase modificata dal Thread:\n%s\n", localData.bufferCopy);

    // Invia la risposta al server centrale
    int bytesSent = send(localData.centralServerSocket, localData.bufferCopy, strlen(localData.bufferCopy), 0);
    if (bytesSent == -1) {
        perror("[!]Errore nell'invio dati");
    }

    printf("[+]Frase inviata con successo alla socket %d! Chiusura del Thread.\n", localData.centralServerSocket);

    pthread_exit(NULL);
}


int main() {
    int secondarySocket;
    struct sockaddr_in serverAddr;

    // Creazione del socket secondario
    secondarySocket = socket(AF_INET, SOCK_STREAM, 0);
    if (secondarySocket == -1) {
        perror("[!]Errore nella creazione del socket secondario");
        exit(EXIT_FAILURE);
    }

    // Configurazione dell'indirizzo del server secondario
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    // Connessione al server principale
    if (connect(secondarySocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("[!]Errore nella connessione al server principale");
        close(secondarySocket);
        exit(EXIT_FAILURE);
    }

    printf("[+]Connesso al server centrale.\n");
    printf("[-]Inizio della fase di ascolto...\n");

    // Ciclo di ricezione e gestione richieste
    while (1) {
        struct ThreadData *threadData = malloc(sizeof(struct ThreadData));

        // Ricevi il buffer dalla socket secondaria
        int bytesReceived = recv(secondarySocket, threadData->bufferCopy, sizeof(threadData->bufferCopy), 0);

        if (bytesReceived < 0) {
            perror("[!]Errore nella ricezione dati");
            free(threadData);  // Liberare la memoria in caso di errore
            break;
        } else if (bytesReceived == 0) {
            // Connessione chiusa dal server centrale
            free(threadData);  // Liberare la memoria in caso di chiusura della connessione
            break;
        }

        // Aggiungi altri dati alla struttura se necessario
        threadData->centralServerSocket = secondarySocket;

        // Crea un nuovo thread per gestire la richiesta
        printf("[+]Creazione del Thread...\n");
        pthread_t thread;
        pthread_create(&thread, NULL, elaboraRichiesta, (void*)threadData);
    }

    // Chiusura del socket secondario
    close(secondarySocket);

    return 0;
}
