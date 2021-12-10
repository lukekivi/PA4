#include "server.h"
#define STARTING_TRANSACTION_SIZE 50

struct account* balances[MAX_ACC];
int numAccounts = 0;

// shared queue of nodes containing sockfds
struct Queue* q;

// sempahores 
sem_t mutexBalances[acctsNum];
sem_t mutexQueue;             
sem_t staged; 

void initBalances() {
    balances = (struct account **) malloc(sizeof(struct account*) * MAX_ACC);

    for (int i = 0; i < STARTING_TRANSACTION_SIZE; i++) {
        balances[i] = NULL;
    }
}

void registerAccount(char* username, char* name, time_t birthday) {
    struct account* newAccount = (struct account*) malloc(sizeof(struct account) * STARTING_TRANSACTION_SIZE);
    account->username = username;
    account->name = name;
    account->birthday = birthday;
    account->balance = 0.0;
    account->transactions = (float*) malloc(sizeof(float));
    account->numTransactions = 0;
    account->transctionSize = STARTING_TRANSACTION_SIZE;

    balances[numAccounts] = account;
    numAccounts += 1;
}

int addTransaction(int accountNumber, float transaction) {
    if (balances[accountNumber] == NULL) {
        return 0;
    } else if (balances[accountNumber]->balance + transaction < 0) {
        return -1;
    } else if (balances[accountNumber]->numTransactions == balances[accountNumber]->transactionSize) {
        balances[accountNumber]->transactions = 
            (float *) realloc(balances[accountNumber]->transctions, sizeof(float) * 2 * balances[accountNumber]->transactionSize);
        balances[accountNumber]->tansactionSize *= 2;

        balances[accountNumber]->transactions[numTransactions] = transaction;
        balances[accountNumber]->numTransactions += 1;
        balances[accountNumber]->balance += transaction;
    }
}


 void freeBalances() {
     for (int i = 0; i < numAccounts; i++) {
         free(balances[i]->username);
         free(balances[i]->name);
         free(balances[i]->transactions);
         balances[i]->username = NULL;
         balances[i]->name = NULL;
         balances[i]->transactions = NULL;
         free(balances[i]);
         balances[i] = NULL;
     }

     free(balances);
 }


void printSyntax() {
    printf("incorrect usage syntax! \n");
    printf("usage: $ ./server server_addr server_port num_workers\n");
}

// Commented out because it's not for interim
void log() {
    // Get the correct filepath
    char file[MAX_STR] = "output/";
    printf("FILE: %s\n", file);

    sleep(LOGGER_SLEEP);
    FILE *fp = fopen(file, "w");
    if (fp == NULL) {
        fprintf(stderr, "ERROR: failed to open file %s\n", file);
        exit(EXIT_FAILURE);
    }
}

void* worker(void* arg) {
     /**
     * args
     *   - 1: local address
     *   - 2: port number
     *   - 3: num workers
     */

    int sockfd = *((int *) arg);

    // re-transmit enumerated value to client
    // when server receives TERMINATE, re-transmit to client, close the connection and return
    char* buf = NULL;
    while (1) {
        buf = (char* ) malloc(sizeof(char) * MSG_BUFFER_SIZE);

        int results = read(sockfd, buf, MSG_BUFFER_SIZE);

        if (results < 0 ) {
            perror("cannot read");
            exit(1);
        } else if (results > 0) {
            msg_enum recv = atoi(buf); // get

            switch (recv) {
              case REGISTER:
                // needs to read in a string name, string username, and long birthday

                // write back BALANCE
                break;
              case GET_ACCOUNT_INFO:
                // need to read in int account_number
                results = read(sockfd, buf, MSG_BUFFER_SIZE, 4);
                int account_number = atoi(buf);
                // getAccountInfo(account_number);

                // write back ACCOUNT_INFO
                break;
              case TRANSACT:
                // need to read in int account_number and float amount
                results = read(sockfd, buf, MSG_BUFFER_SIZE, 4);
                int account_number = atoi(buf);
                results = read(sockfd, buf, MSG_BUFFER_SIZE, 8);
                float amount = atof(buf);
                // transact(account_number, amount);

                // write back BALANCE
                break;
              case GET_BALANCE:
                // need to read in int account_number and float amount
                results = read(sockfd, buf, MSG_BUFFER_SIZE, 4);
                int account_number = atoi(buf);
                results = read(sockfd, buf, MSG_BUFFER_SIZE, 8);
                float amount = atof(buf);
                // get_balance(account_number, amount);

                // write back BALANCE
                break;
              case REQUEST_CASH:
                // need to read in float amount
                results = read(sockfd, buf, MSG_BUFFER_SIZE, 4);
                float amount = atof(buf);
                // requestCash(amount);

                // write back CASH
                break;
              case ERROR:
                // dont need to read in anything

                break;
              case TERMINATE:
                // dont need to read in anything
                break;
              default:
                // another error
                fprintf(stderr, "ERROR: Bad recv argument.");
                exit(0);
            }

            if (write(sockfd, buf, MSG_BUFFER_SIZE) < 0) {
                perror("Cannot write");
                exit(1);
            }
        }

        free(buf);
    }

    // close socket
    close(sockfd);
}

int main(int argc, char *argv[]) {

    // argument handling
    if (argc != 4) {
        printSyntax();
        return 0;
    }

    // create empty output folder
    bookeepingCode();

    // Start log thead
    pthread_t logThread;

    if (pthread_create(&logThread, NULL, log, NULL) != 0) {
         fprintf(stderr, "ERROR: Failed to start log thread\n");
         exit(EXIT_FAILURE);
    }

    // create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Socket creation failed...\n");
        exit(0);
    }
    else {
        printf("Socket successfully created...\n");
    }
    bzero(&servaddr, sizeof(servaddr));

    int nWorkers = argv[3]; 

    int sockfd, len, connfd;
    struct sockaddr_in servaddr, cli;     

    q = initQueue();

    initBalances();

    for (int i = 0; i < acctsNum; i++) {
        sem_init(&mutexBalances[i], 0, 1);
    }
    sem_init(&mutexQueue, 0, 1);
    sem_init(&staged, 0, 0);

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(atoi(argv[2]));

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) {
        printf("Socket bind failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully binded..\n");

    // Now server is ready to listen and verification
    if ((listen(sockfd, NCLIENTS)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    }
    else
        printf("Server listening..\n");
    len = sizeof(cli);

    int count = 0;
    pthread_t tid;
    int connfd_arr[NCLIENTS];

    // create worker threads
    int* workerThreads[nWorkers];
    for (int i = 0; i < nWorkers; i++) {
        if (pthread_create(&workerThreads[i], NULL, worker, NULL) != 0) {
            fprintf(stderr, "ERROR: Failed to start worker thread\n");
            exit(EXIT_FAILURE);
        }
    }

    while (1) {
         // Accept the data packet from client and verification
        connfd_arr[count % NCLIENTS] = accept(sockfd, (struct sockaddr *)&cli, &len);
        if (connfd_arr[count % NCLIENTS] < 0) {
            printf("Server accept failed...\n");
            exit(0);
        } else
            printf("Server accept the client...\n");

        pthread_create(&tid, NULL, worker, (void *)&connfd_arr[count % NCLIENTS]);
    }
}
