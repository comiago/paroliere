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
  str[i + 1] = '\0';
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

int hash(char *word, int m, int i) {
  unsigned long long k = 0;
  size_t len = strlen(word);

  for (size_t i = 0; i < len; ++i) {
    k = k * 128 + (unsigned char)word[i];
  }
  
  int h1 = k % m;
  int h2 = (k >> 5) % (m - 1) + 1;

  return (h1 + i * h2) % m;
}

Dictionary *loadDictionary(int fd) {
  int initialSize = 337499;
  Dictionary *dict = malloc(sizeof(Dictionary));
  dict->dimension = initialSize;
  dict->words = 0;
  dict->loadingFactor = dict->words / dict->dimension;
  dict->hashTable = malloc(initialSize * sizeof(char *));
  dict->conflicts = 0;
  for (int i = 0; i < initialSize; i++) {
    dict->hashTable[i] = NULL;
  }

  char buffer[BUFFERSIZE];
  ssize_t bytesRead;

  while ((bytesRead = read(fd, buffer, BUFFERSIZE)) > 0) {
    if(dict->loadingFactor > 0.7){
      int newSize = dict->dimension * 2;
      char **newHashTable = malloc(newSize * sizeof(char *));
      for (int i = 0; i < newSize; i++) {
        newHashTable[i] = NULL;
      }
      for (int i = 0; i < dict->dimension; i++) {
        if(dict->hashTable[i]){
          dict->conflicts++;
          int conflict = 0;
          int index = hash(dict->hashTable[i], newSize, conflict);
          while(newHashTable[index]){
            conflict++;
            index = hash(dict->hashTable[i], newSize, conflict);
          }
          newHashTable[index] = dict->hashTable[i];
        }
      }
      free(dict->hashTable);
      dict->hashTable = newHashTable;
      dict->dimension = newSize;
    }
    
    int start = 0;
    for (int i = 0; i < bytesRead; i++) {
      if (buffer[i] == '\n') {
        char *parola = malloc(i - start + 1);
        strncpy(parola, buffer + start, i - start);
        parola[i - start] = '\0';
        int conflict = 0;
        int index = hash(parola, dict->dimension, conflict);
        while(dict->hashTable[index]){
          dict->conflicts++;
          conflict++;
          index = hash(parola, dict->dimension, conflict);
        }
        dict->hashTable[index] = parola;
        dict->words++;
        start = i + 1;
      }
    }
    dict->loadingFactor = (double) dict->words / dict->dimension;
  }

  return dict;
}

void printDictionary(Dictionary *dictionary) {
  printf("Dimension: %d\n", dictionary->dimension);
  printf("words: %d\n", dictionary->words);
  printf("Loading Factor: %f\n", dictionary->loadingFactor);
  printf("Conflicts: %d\n", dictionary->conflicts);
}

int userExists(char *nickname, Players *players) {
  Client *current = players->head;
  while (current) {
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
  players->count++;
}

void removeUser(Players *players, char *nickname) {
  if(players->count == 0) return;
  Client *cur = players->head;
  while(cur) {
    if(cur->nickname == nickname) {
      if(cur == players->head) {
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
    printf("%s\n", c->nickname);
    c = c->next;
  }
}

int checkWord(char *word, Dictionary *dictionary) {
  int conflict = 0;
  int index = hash(word, dictionary->dimension, conflict);
  while(dictionary->hashTable[index]){
    if(strcmp(dictionary->hashTable[index], word) == 0){
      return 1;
    }
    conflict++;
    index = hash(word, dictionary->dimension, conflict);
  }
  return 0;
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
  return message;
}

void printMessage(Message *message) {
  printf("%c %d %s\n", message->type, message->length, message->data);
}

void gameOn(Players *players) {
  Client *current = players->head;
  while(current) {
    if(strcmp(current->nickname, "user") == 0) {
      current->inGame = 1;
    }
    current = current->next;
  }
}

void gameOff(Players *players) {
  Client *current = players->head;
  while(current) {
    current->inGame = 0;
    current = current->next;
  }
}