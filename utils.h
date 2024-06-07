#define ROWS 4
#define COLS 4
char ***strToMatr(char *str, int rows, int cols);

char *matrToStr(char ***matrix, int rows, int cols);

char ***randomMatrix(int rows, int cols);

void printMatrix(char ***matrix, int rows, int cols);

int hash(char *word, int m, int i);

Dictionary *loadDictionary(int fd);

void printDictionary(Dictionary *dictionary);

int userExists(char *nickname, Players *players);

void addUser(Players *players, Client *client);

void removeUser(Players *players, char *nickname);

void removeAllUsers(Players *players);

void printUsers(Players *players);

int checkWord(char *word, Dictionary *dictionary);

int checkWordInMatrix(char *word, char ***matrix, int rows, int cols);

void printShell();

char *toLower(char *str);

void *sendMessage(Message *message, int fd);

Message *receiveMessage(int fd);

void printMessage(Message *message);

void gameOn(Players *players);

void gameOff(Players *players);