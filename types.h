#include <pthread.h>

// Definizioni di macro
#define MSG_OK 'K'
#define MSG_ERR 'E'
#define MSG_REGISTRA_UTENTE 'R'
#define MSG_MATRICE 'M'
#define MSG_TEMPO_PARTITA 'T'
#define MSG_TEMPO_ATTESA 'A'
#define MSG_PAROLA 'W'
#define MSG_PUNTI_FINALI 'F'
#define MSG_PUNTI_PAROLA 'P'
#define MSG_USCITA 'U'
#define MSG_HELP 'H'

#define BUFFERSIZE 1024

// Dichiarazione preliminare della struttura Client
struct Client;

// Definizione della struttura Message che indica come deve essere fatto un messaggio da inviare
typedef struct {
  char type;
  unsigned int length;
  char *data;
} Message;

// Definizione della struttura Dictionary
typedef struct {
  char **hashTable;
  int words, dimension, conflicts;
  double loadingFactor;
} Dictionary;

// Definizione della struttura Client contenente tutte le informazioni di un giocatore
typedef struct Client {
  int fd, score, globalScore, inGame;
  char *nickname;
  struct Client *next, *prev;
  pthread_t tid;
} Client;

// Definizione della struttura Players che è la lista dei giocatori collegati
typedef struct {
  Client *head, *tail;
  int count;
} Players;