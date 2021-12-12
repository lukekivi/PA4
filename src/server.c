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

// return the account number for success, -1 for error
int handleRegister(int sockfd) {
    char* username;

    if ((username = readStringFromSocket(sockfd)) == NULL) {
        perror("ERROR: failure to write to sockfd\n");
        return -1;
    }

    char* name;

    if ((name = readStringFromSocket(sockfd)) == NULL) {
        perror("ERROR: failure to write to sockfd\n");
        return -1;
    }

    time_t birthday;

    if (read(sockfd, &birthday, sizeof(time_t)) != sizeof(time_t)) {
        perror("ERROR: failure to read from sockfd\n");
        return -1;
    }

    struct account* acc = (struct account*) malloc(sizeof(struct account) * STARTING_TRANSACTIONS_SIZE);
    acc->username = username;
    acc->name = name;
    acc->birthday = birthday;
    acc->balance = 0.0;
    acc->transactions = (float*) malloc(sizeof(float));
    acc->numTransactions = 0;
    acc->transactionsSize = STARTING_TRANSACTIONS_SIZE;

    sem_wait(&numAccountsMutex);
    balances[numAccounts] = acc;
    int accountNumber = numAccounts;
    numAccounts += 1;
    sem_post(&numAccountsMutex);

    return accountNumber;
}

// return 1 for success, 0 for error
int respondRegister(int sockfd, int accountNumber) {
    msg_enum response = htonl(BALANCE);

    if (write(sockfd, &response, sizeof(msg_enum)) != sizeof(msg_enum)) {
        perror("ERROR: Cannot write\n");
        return 0;
    }

    int accNum = htonl(accountNumber);

    if (write(sockfd, &accNum, sizeof(int)) != sizeof(int)) {
        perror("ERROR: Cannot write\n");
        return 0;
    }

    float balance = 0.0;

    if (write(sockfd, &balance, sizeof(float)) != sizeof(float)) {
        perror("ERROR: Cannot write\n");
        return 0;
    }

    return 1;
}

// return 1 for success, 0 for error
int handleAndRespondCashRequest(int sockfd) {
    float requestedCash;

    if (read(sockfd, &requestedCash, sizeof(float)) != sizeof(float)) {
        perror("ERROR: failure to read from sockfd\n");
        return 0;
    }

    int returnMsg = htonl(CASH);

    if (write(sockfd, &returnMsg, sizeof(int)) != sizeof(int)) {
        perror("ERROR: failure to write to sockfd\n");
        return 0;
    }

    if (write(sockfd, &requestedCash, sizeof(float)) != sizeof(float)) {
        perror("ERROR: failure to write to sockfd\n");
        return 0;
    }

    return 1;
}


int addTransaction(int accountNumber, float transaction) {
    sem_wait(&mutexBalances[accountNumber]);
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
    }
    sem_post(&mutexBalances[accountNumber]);
    sem_wait(&mutexBalances[accountNumber]);
}


 void freeBalances() {
     sem_wait(&numAccountsMutex);
     for (int i = 0; i < numAccounts; i++) {
         sem_wait(&mutexBalances[i]);

         free(balances[i]->username);
         free(balances[i]->name);
         free(balances[i]->transactions);
         balances[i]->username = NULL;
         balances[i]->name = NULL;
         balances[i]->transactions = NULL;
         free(balances[i]);
         balances[i] = NULL;

         sem_post(&mutexBalances[i]);
     }
     sem_post(&numAccountsMutex);

     free(balances);
 }


void printSyntax() {
    printf("incorrect usage syntax! \n");
    printf("usage: $ ./server server_addr server_port num_workers\n");
}


void* writeLog() {
    printf("Logging started\n");
    char file[MAX_STR] = "output/balances.csv";

    int i = 0;
    while(1) {
        FILE *fp = fopen(file, "w");
        if (fp == NULL) {
            fprintf(stderr, "ERROR: failed to open file %s\n", file);
            exit(EXIT_FAILURE);            
        }

        // sem_wait(&numAccountsMutex);
        int curNumAccounts = numAccounts;
        // sem_post(&numAccountsMutex);

        printf("Logging\n");
        for (int i=0; i < curNumAccounts; i++) {
            sem_wait(&mutexBalances[i]);
            struct account* acc = balances[i];
            fprintf(fp, "%d,%.2f,%s,%s,%ld\n", i, acc->balance, acc->name, acc->username,acc->birthday);
            sem_post(&mutexBalances[i]);
        }
        fclose(fp);
        sleep(LOGGER_SLEEP);
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
        
        while (1) {
            int temp;
            if (read(sockfd, &temp, sizeof(int)) != sizeof(int)) {
                perror("ERROR: Cannot read\n");
                exit(1);
            }
            msg_enum recv = ntohl(temp);

            int account_number;
            float amount;

            // when TERMINATE is received, re-transmit to client, close the connection and wait
            if (recv == TERMINATE) {
                printf("TERMINATE\n");
                int response = htonl(recv);

                if (write(sockfd, &response, sizeof(int)) != sizeof(int)) {
                    perror("ERROR: Cannot write\n");
                    exit(1);
                }
                break;
            }

            switch (recv) {
                case REGISTER:
                    printf("REGISTER\n");
                    int accountNumber = handleRegister(sockfd);
                    if (accountNumber < 0) {
                        freeBalances();
                        exit(EXIT_FAILURE);
                    }
                        
                    if (respondRegister(sockfd, accountNumber) == 0) {
                        freeBalances();
                        exit(EXIT_FAILURE);
                    }

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
                    printf("REQUEST_CASH\n");
                    handleAndRespondCashRequest(sockfd);
                
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
