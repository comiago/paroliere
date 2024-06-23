#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "types.h"
#include "macros.h"

char *toLower(char *str) {
  char *newStr = malloc(strlen(str) * sizeof(char));
  for(int i = 0; i < strlen(str); i++) {
    if(str[i] >= 'A' && str[i] <= 'Z') {
      newStr[i] = str[i] + ('a' - 'A');
    } else {
      newStr[i] = str[i];
    }
  }
  return newStr;
}

char ***strToMatr(char *str, int rows, int cols) {
  int k = 0;
  char ***matrix = malloc(rows * sizeof(char **));

  str = toLower(str);

  for (int i = 0; i < rows; i++) {
    matrix[i] = malloc(cols * sizeof(char *));

    for (int j = 0; j < cols; j++) {
      matrix[i][j] = malloc(3 * sizeof(char));

      if (str[k] != '\0') {
        matrix[i][j][0] = str[k++];
        if (str[k] != ' ' && str[k] != '\0') {
            matrix[i][j][1] = str[k++];
        } else {
            matrix[i][j][1] = '\0';
        }
        matrix[i][j][2] = '\0';
      } else {
        matrix[i][j][0] = '\0';
        matrix[i][j][1] = '\0';
        matrix[i][j][2] = '\0';
      }

      while (str[k] == ' ') {
        k++;
      }
    }
  }
  return matrix;
}

char *matrToStr(char ***matrix, int rows, int cols) {
  char *str = malloc(rows * cols * 2 * sizeof(char));
  for(int i = 0; i < rows; i++) {
    for(int j = 0; j < cols; j++) {
      strcat(str, strcat(matrix[i][j], " "));
    }
  }
  return str;
}
 
char ***randomMatrix(int rows, int cols) {
  char *alphabet = "a v c d e f g h i j k l m n o p q qu r s t u v w x y z";
  int elNum = rows * cols, i = 0;
  char str[rows * cols * 3];
  while(elNum > 0) {
    int index = rand() % strlen(alphabet);
    if(alphabet[index] != ' ' && alphabet[index] != '\0' && (index != 0 && alphabet[index - 1] == ' ')) {
      while(alphabet[index] != ' ' && alphabet[index] != '\0') {
        str[i] = alphabet[index];
        index++;
        i++;
      }
      str[i] = ' ';
      i++;
      elNum--;
    }
  }
  str[i] = '\0';
  return strToMatr(str, rows, cols);
}

void printMatrix(char ***matrix, int rows, int cols) {
  for(int i = 0; i < rows; i++) {
    for(int j = 0; j < cols; j++) {
      printf("|%s|", matrix[i][j]);
    }
    printf("\n");
  }
}

TrieNode *createNode() {
  TrieNode *node = (TrieNode *)malloc(sizeof(TrieNode));
  if (node) {
    node->isEndOfWord = 0;
    for (int i = 0; i < 26; i++) {
      node->children[i] = NULL;
    }
  }
  return node;
}

void insertWord(TrieNode *root, const char *word) {
  TrieNode *current = root;
  for (int i = 0; i < strlen(word); i++) {
    int index = word[i] - 'a';
    if (!current->children[index]) {
      current->children[index] = createNode();
    }
    current = current->children[index];
  }
  current->isEndOfWord = 1;
}

TrieNode *loadDictionary(const char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    fprintf(stderr, "Errore nell'apertura del file %s\n", filename);
    return NULL;
  }

  TrieNode *root = createNode();
  char word[100]; // Assumi che le parole nel dizionario non superino i 100 caratteri

  while (fscanf(file, "%s", word) != EOF) {
    insertWord(root, word);
  }

  fclose(file);
  return root;
}

void freeTrie(TrieNode *node) {
  if (node == NULL) {
    return;
  }

  for (int i = 0; i < 26; i++) {
    freeTrie(node->children[i]);
  }
  free(node);
}

int checkWordInDictionary(TrieNode *root, const char *word) {
  TrieNode *current = root;
  for (int i = 0; i < strlen(word); i++) {
    int index = word[i] - 'a';
    if (!current->children[index]) {
      return 0;
    }
    current = current->children[index];
  }
  return current && current->isEndOfWord;
}

int userExists(Players *players, const char *nickname) {
  if (players->head == NULL) {
    return 0;
  }
  Client *current = players->head;
  while (current != NULL) {
    if (strcmp(current->nickname, nickname) == 0) {
        return 1;
    }
    current = current->next;
  }
  return 0;
}

void addUser(Players *players, Client *client) {
  if(players->head == NULL) {
    players->head = client;
    players->tail = client;
  } else {
    client->prev = players->tail;
    client->next = NULL;
    players->tail->next = client;
    players->tail = client;
  }
}

void removeUser(Players *players, char *nickname) {
  if(players->count == 0) return;
  Client *cur = players->head;
  while(cur) {
    if(cur->nickname == nickname) {
      if(players->count == 1) {
        players->head = NULL;
        players->tail = NULL;
      } else if(cur == players->head) {
        players->head = cur->next;
        players->head->prev = NULL;
      } else if(cur == players->tail) {
        players->tail = cur->prev;
        players->tail->next = NULL;
      } else {
        cur->prev->next = cur->next;
        cur->next->prev = cur->prev;
      }
      free(cur);
      break;
    }
    cur = cur->next;
  }
  players->count--;
}

void removeAllUsers(Players *players) {
  Client *cur = players->head;
  while(cur) {
    Client *next = cur->next;
    free(cur);
    cur = next;
  }
  players->head = NULL;
  players->tail = NULL;
  players->count = 0;
}

void printUsers(Players *players) {
  Client *c = players->head;
  while(c) {
    printf("nickname: %s - global score: %d - score: %d - inGame: %d\n", c->nickname, c->globalScore, c->score, c->inGame);
    c = c->next;
  }
}

int dfs(char ***matrix, int rows, int cols, int x, int y, const char *word, int index, int **visited) {
  int wordLen = strlen(word);
  int cellLen = strlen(matrix[x][y]);
  int remainingWordLen = wordLen - index;

  if (remainingWordLen < cellLen || strncmp(matrix[x][y], word + index, cellLen) != 0) {
    return 0;
  }

  if (remainingWordLen == cellLen) {
    return 1;
  }

  visited[x][y] = 1;

  int found = 0;
  if (x + 1 < rows && !visited[x + 1][y]) {
    found = dfs(matrix, rows, cols, x + 1, y, word, index + cellLen, visited);
  }
  if (!found && x - 1 >= 0 && !visited[x - 1][y]) {
    found = dfs(matrix, rows, cols, x - 1, y, word, index + cellLen, visited);
  }
  if (!found && y + 1 < cols && !visited[x][y + 1]) {
    found = dfs(matrix, rows, cols, x, y + 1, word, index + cellLen, visited);
  }
  if (!found && y - 1 >= 0 && !visited[x][y - 1]) {
    found = dfs(matrix, rows, cols, x, y - 1, word, index + cellLen, visited);
  }

  visited[x][y] = 0;

  return found;
}

int checkWordInMatrix(char *word, char ***matrix, int rows, int cols) {
  int **visited = (int **)malloc(rows * sizeof(int *));
  for (int i = 0; i < rows; i++) {
    visited[i] = (int *)malloc(cols * sizeof(int));
    memset(visited[i], 0, cols * sizeof(int));
  }

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      if (strncmp(matrix[i][j], word, strlen(matrix[i][j])) == 0 && dfs(matrix, rows, cols, i, j, word, 0, visited)) {
        for (int k = 0; k < rows; k++) {
          free(visited[k]);
        }
        free(visited);
        return 1;
      }
    }
  }

  for (int k = 0; k < rows; k++) {
    free(visited[k]);
  }
  free(visited);
  return 0;
}

void printShell() {
  int retvalue;
  char *shell = "[PROMPT PAROLIERE]--> ";
  SYSC(retvalue, write(STDOUT_FILENO, shell, strlen(shell)), "Write error");
}

void sendMessage(Message *message, int fd) {
  int retvalue;
  
  SYSC(retvalue, write(fd, (void *)&message->type, sizeof(char)), "Write error");
  SYSC(retvalue, write(fd, (void *)&message->length, sizeof(unsigned int)), "Write error");
  SYSC(retvalue, write(fd, (void *)toLower(message->data), sizeof(char) * message->length), "Write error");
}

Message *receiveMessage(int fd) {
  Message *message = (Message *)malloc(sizeof(Message));
  int retvalue;
  SYSC(retvalue, read(fd, (void *)&message->type, sizeof(char)), "Read error");
  SYSC(retvalue, read(fd, (void *)&message->length, sizeof(unsigned int)), "Read error");
  message->data = (char *)malloc(sizeof(char) * message->length);
  SYSC(retvalue, read(fd, (void *)message->data, sizeof(char) * message->length), "Read error");
  message->data[message->length] = '\0';
  return message;
}

void printMessage(Message *message, char *sender) {
  char *action;
  switch(message->type) {
    case MSG_REGISTRA_UTENTE:
      action = "registration";
      break;
    case MSG_MATRICE:
      action = "matrix";
      break;
    case MSG_PAROLA:
      action = "Word";
      break;
    case MSG_USCITA:
      action = "exit";
      break;
    case MSG_HELP:
      action = "help";
      break;
    case MSG_ERR:
      action = "error";
      break;
    case MSG_OK:
      action = "ok";
      break;
    case MSG_TEMPO_ATTESA:
      action = "waiting time";
      break;
    case MSG_TEMPO_PARTITA:
      action = "game time";
      break;
    case MSG_PUNTI_PAROLA:
      action = "word points";
      break;
    case MSG_PUNTI_FINALI:
      action = "total points";
      break;
  }
  printf("[%s - %s]: %s\n", sender, action, message->data);
}

void gameOn(Players *players) {
  Client *c = players->head;
  while(c) {
    if(strcmp(c->nickname, "user") != 0) {
      c->inGame = 1;
      c->score = 0;
    }
    c = c->next;
  }
}

void insertionSort(Players *p) {
  if (p->head == NULL || p->head->next == NULL) {
    return;
  }
  Client *sorted = NULL;
  Client *current = p->head;
  while (current != NULL) {
    Client *next = current->next;
    if (sorted == NULL || sorted->score <= current->score) {
      current->next = sorted;
      sorted = current;
    } else {
      Client *temp = sorted;
      while (temp->next != NULL && temp->next->score > current->score) {
        temp = temp->next;
      }
      current->next = temp->next;
      temp->next = current;
    }
    current = next;
  }
  p->head = sorted;
}

int wordPlayed(Played *array, int elements, char *word, Client *client) {
  for (int i = 0; i < elements; i++) {
    if (array[i].client == client && strcmp(array[i].word, word) == 0) {
      return 1;
    }
  }
  return 0;
}

char* listToStr(Players *players) {
  int length = 0;
  Client* current = players->head;
  while(current != NULL) {
    length += snprintf(NULL, 0, "%s,%d\n", current->nickname, current->score);
    current = current->next;
  }
  char *buf = malloc((length + 1) * sizeof(char));
  current = players->head;
  int offset = 0;
  while(current != NULL) {
    offset += sprintf(buf + offset, "%s,%d\n", current->nickname, current->score);
    current = current->next;
  }
  buf[offset - 1] = '\0';
  return buf;
}