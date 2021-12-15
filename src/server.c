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

// FUNCTION: INITBALANCES
// Initialize balances of accounts in global array.
void initBalances() {
    balances = (struct account **) malloc(sizeof(struct account*) * MAX_ACC);

    for (int i = 0; i < STARTING_TRANSACTIONS_SIZE; i++) {
        balances[i] = NULL;
    }
}

// FUNCTION: HANDLEREGISTER
// Response to client REGISTER by registering new account in account array.
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

// FUNCTION: RESPONDREGISTER
// Write to client the BALANCE of the new account in response to Client REGISTER
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

// FUNCTION: CASHREQUEST
// Write to client in response to GET_CASH with amount of cash given to client.
int cashRequest(int sockfd) {
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

// FUNCTION: GETBALANCE
// Retrieves the balance of an account
int getBalance(int sockfd) {
    int accountNumber;

    if (read(sockfd, &accountNumber, sizeof(int)) != sizeof(int)) {
        perror("ERROR: failed to read account number from socket.\n");
        return 0;
    }

    accountNumber = ntohl(accountNumber);

    sem_wait(&mutexBalances[accountNumber]);
    float balance = balances[accountNumber]->balance;
    sem_post(&mutexBalances[accountNumber]);

    if (respondBalance(sockfd, accountNumber, balance) == 0) {
        perror("ERROR: from getBalance, occurred within respondBalance\n");
        return 0;
    }

    return 1;
}

// FUNCTION: RESPONDBALANCE
// Writes to client GET_BALANCE with balance of an account.
int respondBalance(int sockfd, int accNum, float balance) {
    int returnMsg = htonl(BALANCE);
    if (write(sockfd, &returnMsg, sizeof(int)) != sizeof(int)) {
        perror("ERROR: failed to write return message to socket.\n");
        return 0;
    }

    accNum = htonl(accNum);
    if (write(sockfd, &accNum, sizeof(int)) != sizeof(int)) {
        perror("ERROR: failed to write account number to socket.\n");
        return 0;
    }

    if (write(sockfd, &balance, sizeof(float)) != sizeof(float)) {
        perror("ERROR: failed to write balance to socket.\n");
        return 0;
    }

    return 1;
}

// FUNCTION: ADDTRANSACTION
// Adds to an accounts balance.
int addTransaction(int accountNumber, float transaction) {

    sem_wait(&mutexBalances[accountNumber]);
    struct account* acc = balances[accountNumber];

    if (acc == NULL) {
        sem_post(&mutexBalances[accountNumber]);
        perror("ERROR: account doesn't exist\n.");
        return -1;

    } else if (acc->balance + transaction < 0) {
        sem_post(&mutexBalances[accountNumber]);
        perror("ERROR: this transaction will put the account's balance negative\n.");
        return -1;

    } else if (acc->numTransactions == acc->transactionsSize) {
        acc->transactionsSize *= 2;
        acc->transactions = (float *) realloc(acc->transactions, sizeof(float) * acc->transactionsSize);

        if (acc->transactions == NULL) {
            sem_post(&mutexBalances[accountNumber]);
            perror("ERROR: realloc failed\n.");
            return -1;
        }
    }

    acc->transactions[acc->numTransactions] = transaction;
    acc->numTransactions += 1;
    acc->balance += transaction;

    balances[accountNumber] = acc;

    float balance = acc->balance;
    sem_post(&mutexBalances[accountNumber]);

    return balance;
}

// FUNCTION: GETTRANSACTIONS
// Helper function to get an array of transactions from a given account
int getTransactions(int accountNumber, int numTransactions, float** arr) {
    sem_wait(&mutexBalances[accountNumber]);
    struct account* acc = balances[accountNumber];
    if (acc == NULL) {
        perror("ERROR: Account number indexed a NULL account\n");
        return -1;
    }

    int size = numTransactions;
    if (numTransactions == 0 || acc->numTransactions < numTransactions) {
        size = acc->numTransactions;
    }

    *arr = (float *) malloc(sizeof(float) * size);

    for (int i = 0; i < size; i++) {
        (*arr)[i] = acc->transactions[i];
    }
    sem_post(&mutexBalances[accountNumber]);
    return size;
}

// FUNCTION: TRANSACT
// Facilitates a transaction to deposit to or wirthdrawal from an account.
int transact(int sockfd) {

    int accNum;
    if (read(sockfd, &accNum, sizeof(int)) != sizeof(int)) {
        perror("ERROR: failed to read account number from socket.\n");
        return 0;
    }

    accNum = ntohl(accNum);
    float amount;
    if (read(sockfd, &amount, sizeof(float)) != sizeof(float)) {
        perror("ERROR: failed to read transaction amount from socket.\n");
        return 0;
    }

    float balance;
    if ((balance = addTransaction(accNum, amount)) < 0) {
        perror("ERROR: failure within addTransaction\n");
        return 0;
    }

    if (respondBalance(sockfd, accNum, balance) == 0) {
        perror("ERROR: in transact, occurred within respondBalance.\n");
        return 0;
    }
    return 1;
}

// FUNCTION: GETACCOUNTINFO
// Writes to client the username, name, and birthday of the requested account number.
int getAccountInfo(int sockfd) {
    int accNum;
    if (read(sockfd, &accNum, sizeof(int)) != sizeof(int)) {
        perror("ERROR: failed to read account number from socket.\n");
        return 0;
    }

    accNum = ntohl(accNum);

    sem_wait(&mutexBalances[accNum]);
    struct account* acc = balances[accNum];

    if (acc == NULL) {
        perror("ERROR: requested account is null\n");
        return 0;
    }

    msg_enum rspMsg = htonl(ACCOUNT_INFO);

    if (write(sockfd, &rspMsg, sizeof(msg_enum)) != sizeof(msg_enum)) {
        perror("ERROR: failed to write response message to socket\n");
        return 0;
    }

    if (writeStringToSocket(sockfd, acc->username) == 0) {
        perror("ERROR: from getAccountInfo, occurred in writeStringToSocket, failed to write username\n");
        return 0;
    }

    if (writeStringToSocket(sockfd, acc->name) == 0) {
        perror("ERROR: from getAccountInfo, occurred in writeStringToSocket, failed to write name\n");
        return 0;
    }

    if (write(sockfd, &acc->birthday, sizeof(time_t)) != sizeof(time_t)) {
        perror("ERROR: failed to write birthday to socket\n");
        return 0;
    }

    sem_post(&mutexBalances[accNum]);

    return 1;
}

// FUNCTION: GETHISTORY
// Writes to client the desired number of recent transactions from a specific account.
int getHistory(int sockfd) {
    int accNum, numTrans;

    if (read(sockfd, &accNum, sizeof(int)) != sizeof(int)) {
        perror("ERROR: getHistory - failed to read the account number\n");
        return 0;
    }

    accNum = ntohl(accNum);

    if (read(sockfd, &numTrans, sizeof(int)) != sizeof(int)) {
        perror("ERROR: getHistory - failed to read the number of transactions\n");
        return 0;
    }

    numTrans = ntohl(numTrans);

    float* transactions = NULL;

    if ((numTrans = getTransactions(accNum, numTrans, &transactions)) == -1) {
        perror("ERROR: getHistory - error within getTransactions\n");
        return 0;
    }

    accNum = htonl(accNum);
    if (write(sockfd, &accNum, sizeof(int)) != sizeof(int)) {
        perror("ERROR: getHistory - failed to write the account number\n");
        return 0;
    }

    int replyNumTrans = htonl(numTrans);
    if (write(sockfd, &replyNumTrans, sizeof(int)) != sizeof(int)) {
        perror("ERROR: getHistory - failed to write the number of transactions\n");
        return 0;
    }

    for (int i = 0; i < numTrans; i++) {
        if (write(sockfd, &transactions[i], sizeof(float)) != sizeof(float)) {
            perror("ERROR: getHistory - failed to write transactions\n");
            return 0;
        }
    }

    return 1;
}

// FUNCTION: FREEBALANCES
// Free the balances of the array.
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


// FUNCTION: WRITELOG
// Writes to the log file.
void* writeLog() {
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
            msg_enum recv;

            if (read(sockfd, &recv, sizeof(int)) != sizeof(int)) {
                perror("ERROR: Cannot read from sockfd\n.");
                close(sockfd);
                freeBalances();
                exit(EXIT_FAILURE);
            }

            recv = ntohl(recv);

            int accountNumber;
            float amount;
            msg_enum replyMsg;

            // when TERMINATE is received close the connection and wait
            if (recv == TERMINATE) {
                break;
            }

            switch (recv) {
                case REGISTER:
                    accountNumber = handleRegister(sockfd);
                    if (accountNumber < 0) {
                        freeBalances();
                        close(sockfd);
                        exit(EXIT_FAILURE);
                    }

                    if (respondRegister(sockfd, accountNumber) == 0) {
                        freeBalances();
                        close(sockfd);
                        exit(EXIT_FAILURE);
                    }

                    break;
                case GET_ACCOUNT_INFO:
                    if (getAccountInfo(sockfd) == 0) {
                        freeBalances();
                        close(sockfd);
                        exit(EXIT_FAILURE);
                    }

                    break;
                case TRANSACT:
                    if (transact(sockfd) == 0) {
                        freeBalances();
                        close(sockfd);
                        exit(EXIT_FAILURE);
                    }

                    break;
                case GET_BALANCE:
                    if (getBalance(sockfd) == 0) {
                        freeBalances();
                        close(sockfd);
                        exit(EXIT_FAILURE);
                    }

                    break;
                case REQUEST_CASH:
                    if (cashRequest(sockfd) == 0) {
                        freeBalances();
                        close(sockfd);
                        exit(EXIT_FAILURE);
                    }

                    break;
                case REQUEST_HISTORY:
                    if (getHistory(sockfd) == 0) {
                        freeBalances();
                        close(sockfd);
                        exit(EXIT_FAILURE);
                    }

                    break;
                case ERROR:
                    // Have to receive the message that caused the error
                    if (read(sockfd, &replyMsg, sizeof(msg_enum)) != sizeof(msg_enum)) {
                        perror("ERROR: Cannot read from sockfd\n.");
                        close(sockfd);
                        freeBalances();
                        exit(EXIT_FAILURE);
                    }

                    break;
                default:
                    replyMsg = htonl(ERROR);

                    if (write(sockfd, &replyMsg, sizeof(msg_enum)) != sizeof(msg_enum)) {
                        perror("ERROR: failed to return ERROR to client.\n");
                    }

                    recv = htonl(recv);
                    if (write(sockfd, &recv, sizeof(msg_enum)) != sizeof(msg_enum)) {
                        perror("ERROR: failed to return MSG that caused ERROR to client.\n");
                    }

                    perror("ERROR: Bad recv argument.\n");
                    freeBalances();
                    close(sockfd);
                    exit(0);
            }
        }

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
      perror("ERROR: Socket creation failed.\n");
      close(sockfd);
      exit(EXIT_FAILURE);
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
      perror("ERROR: Socket bind failed.\n");
      close(sockfd);
      exit(EXIT_FAILURE);
    }
    else
        printf("Socket successfully binded..\n");

    // Now server is ready to listen and verification
    if ((listen(sockfd, NCLIENTS)) != 0) {
      perror("ERROR: Listen failed.\n");
      close(sockfd);
      exit(EXIT_FAILURE);
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
          perror("ERROR: Server accept failed.\n");
          close(sockfd);
          exit(EXIT_FAILURE);
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
