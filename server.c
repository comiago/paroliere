#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <pthread.h>
#include <asm-generic/socket.h>
#include <arpa/inet.h>
#include <signal.h>

#include "types.h"
#include "utils.h"
#include "macros.h"

pthread_mutex_t pmutex = PTHREAD_MUTEX_INITIALIZER;
Players *p;
int server_fd, durata = 3;
volatile sig_atomic_t inGame = 0;
volatile sig_atomic_t remainingTime = 0;
char ***matrix = NULL;
Dictionary *dictionary;

int getTime() {
  remainingTime = alarm(0);
  alarm(remainingTime);
  return remainingTime;
}

void handle_sigint(int sig) {
  printf("\nClosing server...\n");
  if (server_fd != -1) {
    close(server_fd);
  }
  exit(0);
}

void handle_alarm(int sig) {
  pthread_mutex_lock(&pmutex);
  int count = p->count;
  pthread_mutex_unlock(&pmutex);
  if (inGame) {
    printf("Game over. Pausing for 1 minute...\n");
    gameOff(p);
    inGame = 0;
    alarm(60);
  } else {
    if (count >= 2) {
      printf("Pause over. Starting a new game for %d minutes...\n", durata);
      gameOn(p);
      inGame = 1;
      alarm(durata * 60);
    } else {
      printf("Not enough players to start a new game. Waiting for more players...\n");
      alarm(30);
    }
  }
}

void *handle_client(void *arg) {
  Client *client = (Client *)arg;
  Message *msg, *res = malloc(sizeof(Message));
  while(1) {
    msg = receiveMessage(client->fd);
    if (msg == NULL) {
      break;
    }
    switch (msg->type) {
      case MSG_REGISTRA_UTENTE:
        if(msg->length > 10){
          res->type = MSG_ERR;
          res->data = "Nickname too long";
          res->length = strlen(res->data);
          sendMessage(msg, client->fd);
        } else if (userExists(msg->data, p)) {
          res->type = MSG_ERR;
          res->data = "Nickname already in use";
          res->length = strlen(res->data);
          sendMessage(res, client->fd);
        } else {
          client->nickname = msg->data;
          res->type = MSG_OK;
          res->data = "User registered";
          res->length = strlen(res->data);
          sendMessage(res, client->fd);
        }
        break;
      case MSG_MATRICE:
        if(strcmp(client->nickname, "user") == 0) {
          res->type = MSG_ERR;
          res->data = "User not registered";
          res->length = strlen(res->data);
          sendMessage(res, client->fd);
        } else {
          if(inGame && client->inGame) {
            res->type = MSG_MATRICE;
            res->data = matrToStr(matrix, ROWS, COLS);
            res->length = strlen(res->data);
            sendMessage(res, client->fd);
          }
          if(inGame) {
            res->type = MSG_TEMPO_PARTITA;
          } else {
            res->type = MSG_TEMPO_ATTESA;
          }
          char *leftTime = malloc(10 * sizeof(char));
          sprintf(leftTime, "%d", getTime());
          res->data = leftTime;
          res->length = strlen(res->data);
          sendMessage(res, client->fd);
        }
        break;
      case MSG_PAROLA:
        if(strcmp(client->nickname, "user") == 0) {
          res->type = MSG_ERR;
          res->data = "User not registered";
          res->length = strlen(res->data);
          sendMessage(res, client->fd);
        } else if(!client->inGame) {
          res->type = MSG_ERR;
          res->data = "You can't partecipate to this game";
          res->length = strlen(res->data);
          sendMessage(res, client->fd);
        }else if (inGame) {
          if (checkWordInMatrix(msg->data, matrix, ROWS, COLS) && checkWord(msg->data, dictionary)) {
            client->score += strlen(msg->data);
            res->type = MSG_PUNTI_PAROLA;
            char *score = malloc(10 * sizeof(char));
            sprintf(score, "%d", client->score);
            res->data = score;
            res->length = strlen(res->data);
            sendMessage(res, client->fd);
          } else {
            res->type = MSG_ERR;
            res->data = "Invalid word";
            res->length = strlen(res->data);
            sendMessage(res, client->fd);
          }
        } else {
          res->type = MSG_ERR;
          res->data = "Game not started";
          res->length = strlen(res->data);
          sendMessage(res, client->fd);
        }
        break;
      case MSG_USCITA:
        removeUser(p, client->nickname);
        break;
      case MSG_HELP:
        res->type = MSG_HELP;
        res->data = "commands: registra_utente nickname - matrice - punteggio - tempo - p parola - aiuto - fine";
        res->length = strlen(res->data);
        sendMessage(res, client->fd);
        break;
      default:
        break;
    }
  }
}

int main(int argc, char* argv[]) {
  int optopt, longindex, port, seed = 42, ch, retvalue, matrixFd, dictionaryFd, stringsCount = 0;
  char *nome_server, *matrici = NULL, *dizionario = "dictionary_ita.txt", **strings = NULL;
  struct stat statbuf;

  static struct option long_options[] = {
    {"matrici", required_argument, NULL, 'm'},
    {"durata", required_argument, NULL, 'd'},
    {"seed", required_argument, NULL, 's'},
    {"diz", required_argument, NULL, 'z'},
    {0, no_argument, NULL, 0}
  };

  if (argc < 3) {
    printf("Errore: parametri obbligatori mancanti\n");
    printf("Sintassi: %s nome_server port [--matrici data_filename] [--durata durata_in_minuti] [--seed rnd_seed] [--diz dizionario]\n", argv[0]);
    exit(1);
  }

  nome_server = argv[1];
  port = atoi(argv[2]);

  while ((ch = getopt_long(argc, argv, "md:s:z:", long_options, &longindex)) != -1) {
    switch (ch) {
      case 'm':
        matrici = optarg;
        break;
      case 'd':
        durata = atoi(optarg);
        break;
      case 's':
        seed = atoi(optarg);
        break;
      case 'z':
        dizionario = optarg;
        break;
      case '?':
        printf("Parametro non valido: %c\n", optopt);
        exit(EXIT_FAILURE);
      default:
        printf("Errore interno: parametro sconosciuto\n");
        exit(EXIT_FAILURE);
    }
  }

  signal(SIGINT, handle_sigint);
  signal(SIGALRM, handle_alarm);

  srand(seed);

  if(matrici == NULL) {
    matrix = randomMatrix(ROWS, COLS);
  } else { // controllo che il file delle matrici sia regolare e ne genero la matrice
    SYSC(retvalue, stat(matrici, &statbuf), matrici);
    if(!S_ISREG(statbuf.st_mode)) {
      printf("%s is not a regular file.\n", matrici);
      exit(EXIT_FAILURE);
    }
    SYSC(matrixFd, open(matrici, O_RDONLY), "nella open matrix");
    ssize_t bytes_read;
    int i = 0;
    char buffer[BUFFERSIZE];

    while ((bytes_read = read(matrixFd, buffer + i, 1)) > 0) {
      if (buffer[i] == '\n') {
        buffer[i] = '\0';
        if (i > 0) {
          char *line = malloc(i + 1);
          strncpy(line, buffer, i + 1);
          char **new_lines = realloc(strings, (stringsCount + 1) * sizeof(char *));
          strings = new_lines;
          strings[stringsCount] = line;
          stringsCount++;
        }
        i = 0;
      } else {
        i++;
      }
    }

    if (i > 0) {
      buffer[i] = '\0';
      char *line = malloc(i + 1);
      strncpy(line, buffer, i + 1);
      char **new_lines = realloc(strings, (stringsCount + 1) * sizeof(char *));
      strings = new_lines;
      strings[stringsCount] = line;
      stringsCount++;
    }


    close(matrixFd);

    matrix = strToMatr(strings[stringsCount - 1], ROWS, COLS);
  }

  if(dizionario != NULL) { // controllo che il file delle matrici sia regolare e genero il dizionario
    SYSC(retvalue, stat(dizionario, &statbuf), dizionario);
    if(!S_ISREG(statbuf.st_mode)) {
      printf("%s is not a regular file.\n", dizionario);
      exit(EXIT_FAILURE);
    }
    SYSC(dictionaryFd, open(dizionario, O_RDONLY), "nella open dictionary");
    dictionary = loadDictionary(dictionaryFd);
  }

  p = malloc(sizeof(Players));
  p->head = NULL;
  p->tail = NULL;
  p->count = 0;

  struct sockaddr_in address;
  int client_fd, opt = 1, addrlen = sizeof(address);

  SYSC(server_fd, socket(AF_INET, SOCK_STREAM, 0), "socket error");

  SYSC(retvalue, setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)), "setsockopt error");
  SYSC(retvalue, setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)), "setsockopt error");

  address.sin_family = AF_INET;
  if (inet_pton(AF_INET, nome_server, &address.sin_addr) <= 0) {
    printf("Indirizzo IP non valido: %s\n", nome_server);
    exit(EXIT_FAILURE);
  }
  address.sin_port = htons(port);

  SYSC(retvalue, bind(server_fd, (struct sockaddr *)&address, sizeof(address)), "bind error");

  SYSC(retvalue, listen(server_fd, 0), "listen error");

  printf("Server in ascolto su %s:%d...\n", nome_server, port);

  printMatrix(matrix, ROWS, COLS);
  alarm(60);

  while ((client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) != -1) {
    Client *client = malloc(sizeof(Client));
    client->fd = client_fd;
    client->nickname = "user";
    client->inGame = 0;
    client->score = 0;
    client->globalScore = 0;
    pthread_mutex_lock(&pmutex);
    addUser(p, client);
    pthread_mutex_unlock(&pmutex);
    pthread_create(&client->tid, NULL, handle_client, client);
    pthread_detach(client->tid);
  }
  
  close(server_fd);
  return 0;
}