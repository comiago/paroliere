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

int main(int argc, char* argv[]) {
  srand(480);
  Players *p = malloc(sizeof(Players));
  p->head = NULL;
  p->tail = NULL;
  p->count = 0;
  Client *client = malloc(sizeof(Client));
  client->nickname = "user";
  client->inGame = 0;
  client->score = 0;
  client->globalScore = 0;
  client->next = NULL;
  client->prev = NULL;
  for(int i = 0; i < 10; i++) {
    Client *client = malloc(sizeof(Client));
    client->nickname = "user";
    client->inGame = 0;
    client->score = 0;
    client->globalScore = 0;
    client->next = NULL;
    client->prev = NULL;
    addUser(p, client);
  }
  char *str = listToStr(p);
  printf("%s\n", str);
  return 0;
}