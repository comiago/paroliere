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
char ***matrix = NULL;

int main(int argc, char *argv[]) {
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
        exit(1);
      default:
        printf("Errore interno: parametro sconosciuto\n");
        exit(1);
    }
  }

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
  // Array per memorizzare le stringhe
    char **strings = NULL;
    size_t strings_count = 0;
        ssize_t bytes_read;


    while ((bytes_read = read(fd, buffer, MAX_STRING_LENGTH)) > 0) {
        // Aggiungi una stringa alla fine dell'array
        strings = realloc(strings, (strings_count + 1) * sizeof(char *));
        if (strings == NULL) {
            perror("realloc");
            exit(EXIT_FAILURE);
        }

        // Alloca la memoria per la nuova stringa
        strings[strings_count] = malloc(bytes_read + 1); // +1 per il terminatore null
        if (strings[strings_count] == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }

        // Copia la stringa letta nel buffer nell'array
        memcpy(strings[strings_count], buffer, bytes_read);
        strings[strings_count][bytes_read] = '\0'; // Aggiungi il terminatore null

        strings_count++;
    }

    // Controllo errori di lettura
    if (bytes_read == -1) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    // Stampa le stringhe lette
    for (size_t i = 0; i < strings_count; i++) {
        printf("String %zu: %s", i+1, strings[i]);
    }

    // Libera la memoria allocata per le stringhe
    for (size_t i = 0; i < strings_count; i++) {
        free(strings[i]);
    }

    matrix = strToMatr(strings[0], ROWS, COLS);

    printMatrix(matrix, ROWS, COLS);
  }
  return 0;
}