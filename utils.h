#define ROWS 4
#define COLS 4
char ***strToMatr(char *str, int rows, int cols);

char *matrToStr(char ***matrix, int rows, int cols);

char ***randomMatrix(int rows, int cols);

void printMatrix(char ***matrix, int rows, int cols);

TrieNode *loadDictionary(const char *filename);

void freeTrie(TrieNode *node);

int checkWordInDictionary(TrieNode *root, const char *word);

int userExists(Players *players, const char *nickname);

void addUser(Players *players, Client *client);

void removeUser(Players *players, char *nickname);

void removeAllUsers(Players *players);

void printUsers(Players *players);

int checkWordInMatrix(char *word, char ***matrix, int rows, int cols);

void printShell();

char *toLower(char *str);

void *sendMessage(Message *message, int fd);

Message *receiveMessage(int fd);

void printMessage(Message *message, char *sender);

void insertionSort(Players *p);

int wordPlayed(char **array, int elements, char *word, Client *client);

char *listToStr(Players *players);