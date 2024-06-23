#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <asm-generic/socket.h>
#include <netdb.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <signal.h>

#include "types.h"
#include "macros.h"
#include "utils.h"

int client_fd;
Message *response;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t risposta = PTHREAD_COND_INITIALIZER;
volatile int running = 1;

void handle_sigint(int sig) {
    printf("\nClosing client...\n");
    running = 0;
    Message *msg = malloc(sizeof(Message));
    msg->type = MSG_USCITA;
    msg->data = "";
    msg->length = 0;
    sendMessage(msg, client_fd);
    free(msg);
    if (client_fd != -1) {
        close(client_fd);
    }
    exit(0);
}

void *receive(void *arg) {
    while (running) {
        response = receiveMessage(client_fd);
        if (!running) break;
        pthread_mutex_lock(&mutex);
        if(response->type == MSG_PUNTI_FINALI) {
          printMessage(response, "server");
        } else if (response->type == MSG_MATRICE) {
            printMatrix(strToMatr(response->data, ROWS, COLS), ROWS, COLS);
        } else if(response->type == MSG_TEMPO_PARTITA) {
            printMessage(response, "server");
        }
        pthread_cond_signal(&risposta);
        pthread_mutex_unlock(&mutex);
    }
    close(client_fd);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int port;
    char *nome_server;
    int retvalue;
    struct sockaddr_in server_addr;
    struct addrinfo hints, *res, *p;
    char buffer[BUFFERSIZE];
    Message *message = (Message *)malloc(sizeof(Message));
    pthread_t tid;

    // Controllo dei parametri
    if (argc != 3) {
        printf("Errore: parametri obbligatori mancanti\n");
        printf("Sintassi: %s nome_server port\n", argv[0]);
        exit(1);
    }

    signal(SIGINT, handle_sigint);

    port = atoi(argv[2]);
    nome_server = argv[1];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    SYSC(retvalue, getaddrinfo(nome_server, NULL, &hints, &res), "Getaddrinfo error");

    for (p = res; p != NULL; p = p->ai_next) {
        if (p->ai_family == AF_INET) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            memcpy(&server_addr, ipv4, sizeof(struct sockaddr_in));
            break;
        }
    }

    freeaddrinfo(res);

    if (p == NULL) {
        fprintf(stderr, "Errore: non è stato trovato un indirizzo valido per il server\n");
        exit(1);
    }

    server_addr.sin_port = htons(port);

    SYSC(client_fd, socket(AF_INET, SOCK_STREAM, 0), "Socket error");

    SYSC(retvalue, connect(client_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)), "Connect error");

    pthread_create(&tid, NULL, receive, NULL);

    while (1) {
        printShell();
        SYSC(retvalue, read(STDIN_FILENO, buffer, BUFFERSIZE), "Read error");
        buffer[retvalue - 1] = '\0';

        char *tokens[2];
        int num_tokens = 0;

        char *token = strtok(buffer, " ");

        while (token != NULL) {
            tokens[num_tokens] = token;
            num_tokens++;

            token = strtok(NULL, " ");
        }

        pthread_mutex_lock(&mutex);
        if (strcmp(tokens[0], "registra_utente") == 0) {
            message->type = MSG_REGISTRA_UTENTE;
            message->length = strlen(tokens[1]);
            message->data = tokens[1];
            sendMessage(message, client_fd);
        } else if (strcmp(tokens[0], "matrice") == 0) {
            message->type = MSG_MATRICE;
            message->length = 0;
            message->data = "";
            sendMessage(message, client_fd);
        } else if (strcmp(tokens[0], "tempo") == 0) {
            message->type = MSG_TEMPO_PARTITA;
            message->length = 0;
            message->data = "";
            sendMessage(message, client_fd);
        } else if (strcmp(tokens[0], "p") == 0) {
            message->type = MSG_PAROLA;
            message->length = strlen(tokens[1]);
            message->data = tokens[1];
            sendMessage(message, client_fd);
        } else if (strcmp(tokens[0], "punteggio") == 0) {
            message->type = MSG_PUNTI_FINALI;
            message->length = 0;
            message->data = "";
            sendMessage(message, client_fd);
        } else if (strcmp(tokens[0], "aiuto") == 0) {
            message->type = MSG_HELP;
            message->length = 0;
            message->data = "";
            sendMessage(message, client_fd);
        } else if (strcmp(tokens[0], "fine") == 0) {
            message->type = MSG_USCITA;
            message->length = 0;
            message->data = "";
            sendMessage(message, client_fd);
            running = 0;
            pthread_mutex_unlock(&mutex);
            pthread_join(tid, NULL);
            close(client_fd);
            free(message);
            exit(0);
            break;
        } else {
            printf("Comando non riconosciuto\n");
            continue;
        }
        
        pthread_cond_wait(&risposta, &mutex);
        if (response->type == MSG_CLASSIFICA) {
            // printRank(response->data);
        } else {
            printMessage(response, "server");
        }
        pthread_mutex_unlock(&mutex);
    }
}
