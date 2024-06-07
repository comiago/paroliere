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

void handle_sigint(int sig) {
  printf("\nClosing client...\n");
  // Message *msg = malloc(sizeof(Message));
  // int retvalue;
  // msg->type = MSG_USCITA;
  // msg->data = "";
  // msg->length = 0;
  // char *buf = tokenize(msg);
  // SYSC(retvalue, write(client_fd, buf, BUFFERSIZE), "Write error");
  if (client_fd != -1) {
    close(client_fd);
  }
  exit(0);
}

int main(int argc, char *argv[]) {
  int port;
  char *nome_server;
  int retvalue;
  struct sockaddr_in server_addr;
  struct addrinfo hints, *res, *p;
  char ***matrix, buffer[BUFFERSIZE];
  Message *message = (Message *)malloc(sizeof(Message));

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

  while(1) {
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

    if (strcmp(tokens[0], "registra_utente") == 0) {
      message->type = MSG_REGISTRA_UTENTE;
      message->length = strlen(tokens[1]);
      message->data = tokens[1];
      sendMessage(message, client_fd);
      free(message);
      message = receiveMessage(client_fd);
      printMessage(message);
    } else if (strcmp(tokens[0], "matrice") == 0) {
      message->type = MSG_MATRICE;
      message->length = 0;
      message->data = "";
      sendMessage(message, client_fd);
      free(message);
      message = receiveMessage(client_fd);
      if(message->type == MSG_MATRICE) {
        (strToMatr(message->data, ROWS, COLS), ROWS, COLS);
        message = receiveMessage(client_fd);
        printMessage(message);
        message = receiveMessage(client_fd);
      }
      printMessage(message);
    } else if (strcmp(tokens[0], "tempo") == 0) {
      message->type = MSG_TEMPO_PARTITA;
      message->length = 0;
      message->data = "";
      sendMessage(message, client_fd);
      free(message);
      message = receiveMessage(client_fd);
      printMessage(message);
    } else if (strcmp(tokens[0], "p") == 0) {
      message->type = MSG_PAROLA;
      message->length = strlen(tokens[1]);
      message->data = tokens[1];
      sendMessage(message, client_fd);
      free(message);
      message = receiveMessage(client_fd);
      printMessage(message);
    } else if (strcmp(tokens[0], "punteggio") == 0) {
      message->type = MSG_PUNTI_FINALI;
      message->length = 0;
      message->data = "";
      sendMessage(message, client_fd);
      free(message);
      message = receiveMessage(client_fd);
      printMessage(message);
    } else if (strcmp(tokens[0], "aiuto") == 0) {
      message->type = MSG_HELP;
      message->length = 0;
      message->data = "";
      sendMessage(message, client_fd);
      free(message);
      message = receiveMessage(client_fd);
      printMessage(message);
    } else if (strcmp(tokens[0], "fine") == 0) {
      message->type = MSG_USCITA;
      message->length = 0;
      message->data = "";
      sendMessage(message, client_fd);
      free(message);
      message = receiveMessage(client_fd);
      printMessage(message);
      close(client_fd);
      exit(0);
    } else {
      printf("Comando non riconosciuto\n");
    }
  }
}
