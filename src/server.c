#include "server.h"
#define STARTING_TRANSACTIONS_SIZE 50

struct account** balances;
int numAccounts = 0;

// shared queue of nodes containing sockfds
struct Queue* q;

// sempahores 
sem_t mutexBalances[MAX_ACC];
sem_t mutexQueue;             
sem_t staged; 
sem_t numAccountsMutex;

void initBalances() {
    balances = (struct account **) malloc(sizeof(struct account*) * MAX_ACC);

    for (int i = 0; i < STARTING_TRANSACTIONS_SIZE; i++) {
        balances[i] = NULL;
    }
}


// Returns 1 if the username is already registered in balances, and 0 otherwise
int isUsernameInBalances(char* username) {
    sem_wait(&numAccountsMutex);
    for (int i = 0; i < numAccounts; i++) {
        if (balances[i]->username == username) {
            perror("ERROR: account already exists.");
            return 0;
        }
    }
    return 1;
}

// return 1 for success, -1 for error
int registerAccount(int sockfd) {
    char* username;

    if ((username = readStringFromSocket(sockfd)) == NULL) {
        return -1;
    }

    if (isUsernameInBalances(username) == 0) {
        return -1;
    } 

    char* name;

    if ((name = readStringFromSocket(sockfd)) == NULL) {
        return -1;
    }

    time_t birthday;
    // TODO: HOW???

    struct account* acc = (struct account*) malloc(sizeof(struct account) * STARTING_TRANSACTIONS_SIZE);
    acc->username = username;
    acc->name = name;
    acc->birthday = birthday;
    acc->balance = 0.0;
    acc->transactions = (float*) malloc(sizeof(float));
    acc->numTransactions = 0;
    acc->transactionsSize = STARTING_TRANSACTIONS_SIZE;

    balances[numAccounts] = acc;
    numAccounts += 1;
    sem_post(&numAccountsMutex);

    return 0;
}


int addTransaction(int accountNumber, float transaction) {
    struct account* acc = balances[accountNumber];

    if (acc == NULL) {
        return 0;
    } else if (acc->balance + transaction < 0) {
        return -1;
    } else if (acc->numTransactions == acc->transactionsSize) {
        acc->transactions = 
            (float *) realloc(acc->transactions, sizeof(float) * 2 * acc->transactionsSize);
        acc->transactionsSize *= 2;

        acc->transactions[acc->numTransactions] = transaction;
        acc->numTransactions += 1;
        acc->balance += transaction;

        balances[accountNumber] = acc;

        return 1;
    }
}


 void freeBalances() {
     sem_wait(&numAccountsMutex);
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
     sem_post(&numAccountsMutex);

     free(balances);
 }


void printSyntax() {
    printf("incorrect usage syntax! \n");
    printf("usage: $ ./server server_addr server_port num_workers\n");
}


void* writeLog() {
    char file[MAX_STR] = "output/balances.csv";
    FILE *fp = fopen(file, "w");
    if (fp == NULL) {
        fprintf(stderr, "ERROR: failed to open file %s\n", file);
        exit(EXIT_FAILURE);            
    }

    while(1) {
        sleep(LOGGER_SLEEP);
        sem_wait(&numAccountsMutex);
        int curNumAccounts = numAccounts;
        sem_post(&numAccountsMutex);

        for (int i=0; i < curNumAccounts; i++) {
            sem_wait(&mutexBalances[i]);
            struct account* acc = balances[i];
            fprintf(fp, "%d,%.2f,%s,%s,%ld\n", i, acc->balance, acc->name, acc->username,acc->birthday);
            sem_post(&mutexBalances[i]);
        }
    }
}


/************
 *  Worker  *
 ************/
void* worker(void* arg) {

    while(1) {

        sem_wait(&staged);
        sem_wait(&mutexQueue);
        int sockfd = dequeue(q);
        sem_post(&mutexQueue);

        if (sockfd < 0) {
            perror("tried to dequeue from an empty queue");
            exit(1);
        }


        // when server receives TERMINATE, re-transmit to client, close the connection and wait
        
        while (1) {
            int temp;
            int results = read(sockfd, &temp, sizeof(int));
            msg_enum recv = ntohl(temp);

            int account_number;
            float amount;

            if (results < 0 ) {
                perror("cannot read");
                exit(1);
            } else if (results > 0) {

                if (recv == TERMINATE) {
                    printf("BREAK\n");
                    int response = htonl(recv);

                    if (write(sockfd, &response, sizeof(int)) < 0) {
                        perror("Cannot write");
                        exit(1);
                    }

                    break;
                }

                switch (recv) {
                    case REGISTER:
                        printf("REGISTER\n");
                        registerAccount(sockfd);

                        // write back BALANCE
                        break;
                    case GET_ACCOUNT_INFO:
                        // // need to read in int account_number
                        // results = read(sockfd, buf, MSG_BUFFER_SIZE);
                        // account_number = atoi(buf);
                        // getAccountInfo(account_number);
                        printf("GET_ACCOUNT_INFO\n");

                        // write back ACCOUNT_INFO
                        break;
                    case TRANSACT:
                        // need to read in int account_number and float amount
                        // results = read(sockfd, buf, MSG_BUFFER_SIZE);
                        // account_number = atoi(buf);
                        // results = read(sockfd, buf,MSG_BUFFER_SIZE);
                        // amount = atof(buf);
                        // transact(account_number, amount);
                        printf("TRANSACT\n");

                        // write back BALANCE
                        break;
                    case GET_BALANCE:
                        // need to read in int account_number and float amount
                        // results = read(sockfd, buf, MSG_BUFFER_SIZE);
                        // account_number = atoi(buf);
                        // results = read(sockfd, buf, MSG_BUFFER_SIZE);
                        // amount = atof(buf);
                        // get_balance(account_number, amount);
                        printf("GET_BALANCE\n");

                        // write back BALANCE
                        break;
                    case REQUEST_CASH:
                        // need to read in float amount
                        // results = read(sockfd, buf, MSG_BUFFER_SIZE);
                        // amount = atof(buf);
                        // requestCash(amount);
                        printf("REQUEST_CASH\n");

                        // write back CASH
                        break;
                    case REQUEST_HISTORY:
                        printf("REQUEST_HISTORY\n");
                        break;
                    case ERROR:
                        printf("ERROR\n");
                        freeBalances();
                        exit(EXIT_FAILURE);
                        break;
                    default:
                        fprintf(stderr, "ERROR: Bad recv argument.\n");
                        freeBalances();
                        exit(0);
                }

                int response = htonl(selectResponse(recv));

                if (write(sockfd, &response, sizeof(int)) < 0) {
                    perror("Cannot write");
                    exit(1);
                }
            }
        }

        printf("Socket closed\n");

        close(sockfd);
    }
}



/**********
 *  MAIN  *
 **********/

int main(int argc, char *argv[]) {
         /**
     * args
     *   - 1: local address
     *   - 2: port number
     *   - 3: num workers
     */

    // argument handling
    if (argc != 4) {
        printSyntax();
        return 0;
    }

    // create empty output folder
    bookeepingCode();

    // Start log thead
    pthread_t logThread;

    if (pthread_create(&logThread, NULL, writeLog, NULL) != 0) {
         fprintf(stderr, "ERROR: Failed to start log thread\n");
         exit(EXIT_FAILURE);
    }

    // create socket
    int sockfd, len;
    struct sockaddr_in servaddr, cli; 

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Socket creation failed...\n");
        exit(0);
    }
    else {
        printf("Socket successfully created...\n");
    }
    bzero(&servaddr, sizeof(servaddr));

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


    /** Data structure and semaphore initializations **/
    q = initQueue();
    initBalances();

    for (int i = 0; i < MAX_ACC; i++) {
        sem_init(&mutexBalances[i], 0, 1);
    }

    sem_init(&mutexQueue, 0, 1);
    sem_init(&staged, 0, 0);
    sem_init(&numAccountsMutex, 0, 1);

    int nWorkers = atoi(argv[3]); 

    // create worker threads
    pthread_t workerThreads[nWorkers];
    for (int i = 0; i < nWorkers; i++) {
        if (pthread_create(&workerThreads[i], NULL, worker, NULL) != 0) {
            fprintf(stderr, "ERROR: Failed to start worker thread\n");
            exit(EXIT_FAILURE);
        }
    }

    while (1) {
         // Accept the data packet from client and verification
        int newSockfd = accept(sockfd, (struct sockaddr *)&cli, &len);
        if (sockfd < 0) {
            printf("Server accept failed...\n");
            exit(0);
        } else {
            printf("Server accept the client...\n");

            struct Node* node = initNode(newSockfd);
            sem_wait(&mutexQueue);
            enqueue(q, node);
            sem_post(&mutexQueue);
        
            sem_post(&staged);
        }
    }
}
