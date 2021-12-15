#include "../include/client.h"

float clientCash = START_CASH;
int isConnected = 0;

void printSyntax(){
    printf("incorrect usage syntax! \n");
    printf("usage: $ ./client input_filename server_addr server_port\n");
}

int connectSocket(struct sockaddr_in servaddr) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1) {
      perror("ERROR: Socket creation failed.\n");
      close(sockfd);
      exit(EXIT_FAILURE);
    }

    // connect the client socket to server socket
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
      perror("ERROR: Connection with server failed.\n");
      close(sockfd);
      exit(EXIT_FAILURE);
    }

    isConnected = 1;
    return sockfd;
}

// FUNCTION: ERROR
// Generic error message to be sent when the enumerated message type does not match with any in the protocol
void error (int sockfd, msg_enum message_type) {
    msg_enum error_msg = ERROR;

    if (writeEnum(sockfd, error_msg) == -1) {
        perror("ERROR: error - failed to write error_msg\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (writeEnum(sockfd, message_type) == -1) {
      perror("ERROR: error - could not write the incorrect message type recieved.\n");
      close(sockfd);
      exit(EXIT_FAILURE);
    }
}

// FUNCTION: TERMINATE
// Alert the server of termination, once response is received
// Terminate connection
void terminate(int sockfd) {
    msg_enum terminate_msg = TERMINATE;
    if (writeEnum(sockfd, terminate_msg) == -1) {
        perror("ERROR: terminate - cannot write msg to sockfd\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    close(sockfd);
    isConnected = 0;
}

// FUNCTION: REGISTER
// Register a user
void register_user(int sockfd, char* name, char* username, time_t birthday) {
    msg_enum register_msg = REGISTER;
    
    if (writeEnum(sockfd, register_msg) == -1) {
        perror("ERROR: register_user - Cannot write msg to sockfd\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (writeStringToSocket(sockfd, username) == -1) {
        close(sockfd);
        exit(EXIT_FAILURE);
    }


    if (writeStringToSocket(sockfd, name) == -1) {
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (write(sockfd, &birthday, sizeof(time_t)) != sizeof(time_t)) {
        perror("ERROR: register_user - Cannot write birthday to sockfd\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    msg_enum returned_msg;

    if ((returned_msg = readEnum(sockfd)) == -1) {
        perror("ERROR: failed to read from sockfd\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (returned_msg != BALANCE) {
        if (returned_msg == ERROR) {
            readEnum(sockfd); // burn extra message, the account didn't exist
            perror("ERROR: transaction invalid, account didn't exist");
            return;
        }
        perror("ERROR: failed to follow protocol\n");
        error(sockfd, returned_msg);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    int accountNumber;

    if (read(sockfd, &accountNumber, sizeof(int)) != sizeof(int)) {
        perror("ERROR: failed to read from sockfd\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    float balance;
    if (read(sockfd, &balance, sizeof(float)) != sizeof(float)) {
        perror("ERROR: failed to read from sockfd\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
}

// FUNCTION: REQUEST_CASH
// Request that the recipient is sent cash
void request_cash (int sockfd, float request) {
    msg_enum request_cash_msg = REQUEST_CASH;

    if (writeEnum(sockfd, request_cash_msg) == -1) {
        perror("ERROR: Cannot write to socket\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (write(sockfd, &request, sizeof(float)) != sizeof(float)) {
        perror("ERROR: Cannot write to socket\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    msg_enum returned_msg;
    if ((returned_msg = readEnum(sockfd)) == -1) {
        perror("ERROR: Cannot read account number\n.");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (returned_msg != CASH) {
        if (returned_msg == ERROR) {
            readEnum(sockfd); // burn extra message, the account didn't exist
            perror("ERROR: transaction invalid, account didn't exist");
            return;
        }
        perror("ERROR: Request cash recieved wrong response type.\n");
        error(sockfd, returned_msg);
        return;
    }

    float rcvCash;
    if (read(sockfd, &rcvCash, sizeof(float)) != sizeof(float)) {
        perror("ERROR: Cannot read balance.\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    clientCash += rcvCash;
}

// FUNCTION: GET_BALANCE
// Get the balance of a specific account
float get_balance (int sockfd, int accountNumber) {
    msg_enum get_balance_msg = GET_BALANCE;

    if (writeEnum(sockfd, get_balance_msg) == -1) {
        perror("ERROR: Cannot write to socket\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (write(sockfd, &accountNumber, sizeof(int)) != sizeof(int)) {
        perror("ERROR: Cannot write to socket\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    msg_enum returned_msg;

    if ((returned_msg = readEnum(sockfd)) == -1) {
        perror("ERROR: Failed to read message type from socket\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (returned_msg != BALANCE) {
        if (returned_msg == ERROR) {
            readEnum(sockfd); // burn extra message, the account didn't exist
            perror("ERROR: transaction invalid, account didn't exist");
            return -1;
        }
        perror("ERROR: Did not receive BALANCE back from sever after sending GET_BALANCE\n");
        error(sockfd, returned_msg);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    int rcvAccount_number;
    if (read(sockfd, &rcvAccount_number, sizeof(int)) != sizeof(int)) {
        perror("ERROR: Failed to read account number from socket\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (rcvAccount_number != accountNumber) {
        perror("ERROR: get_balance recieved the wrong account number.\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    float rcvBalance;
    if (read(sockfd, &rcvBalance, sizeof(float)) != sizeof(float)) {
        perror("ERROR: Failed to read balance from socket.\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    return rcvBalance;
}

// FUNCTION: TRANSACT
// Send a transaction for an account
void transact (int sockfd, int account_number, float amount) {
    // send a GET_BALANCE message to the server to ensure
    // that the account will not go negative
    float rcvBalance = get_balance(sockfd, account_number);

    if ((rcvBalance + amount) < 0) {
        return; // if account can go negative, ignore transact
    }

    // send GET_CASH to server until the cash variable will not go negative.
    while (clientCash + amount < 0) {
        request_cash(sockfd, CASH_AMOUNT);
    }

    msg_enum transact_msg = TRANSACT;

    // send TRANSACTION to the server
    if (writeEnum(sockfd, transact_msg) == -1) {
        perror("ERROR: Transact could not write msg_enum TRANSACT to server.\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (write(sockfd, &account_number, sizeof(int)) != sizeof(int)) {
        perror("ERROR: Transact could not write account_number to server.\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (write(sockfd, &amount, sizeof(float)) != sizeof(float)) {
        perror("ERROR: Transact could not write balance to server.\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // add the value of the transaction to the cash variable
    clientCash += amount;

    msg_enum returned_msg;

    // balance is returned
    if ((returned_msg = readEnum(sockfd)) == -1) {
        perror("ERROR: Transact could not read message type.\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (returned_msg != BALANCE) {
        if (returned_msg == ERROR) {
            readEnum(sockfd); // burn extra message, the account didn't exist
            perror("ERROR: transaction invalid, account didn't exist");
            return;
        }
        perror("ERROR: Did not receive BALANCE back from sever after sending GET_BALANCE\n");
        error(sockfd, returned_msg);
        close(sockfd);
        exit(EXIT_FAILURE);
    }


    int rcvAccount_number;
    if (read(sockfd, &rcvAccount_number, sizeof(int)) != sizeof(int)) {
      perror("ERROR: Transact could not read account number.\n");
      close(sockfd);
      exit(EXIT_FAILURE);
    }

    if (read(sockfd, &rcvBalance, sizeof(float)) != sizeof(float)) {
      perror("ERROR: Transact could not read balance.\n");
      close(sockfd);
      exit(EXIT_FAILURE);
    }
}

// FUNCTION: GET_ACCOUNT_INFO
// request the information for a specific account
void get_account_info (int sockfd, int acc_num) {
    msg_enum get_account_info_msg = GET_ACCOUNT_INFO;

    char* name = (char*)malloc(sizeof(char)*MAX_STR);
    char* username = (char*)malloc(sizeof(char)*MAX_STR);
    time_t birthday;

    // write message type
    if (writeEnum(sockfd, get_account_info_msg) == -1) {
        perror("ERROR: get_account_info failed to write msg_type\n.");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // write account number
    if (write(sockfd, &acc_num, sizeof(int)) != sizeof(int)) {
        perror("ERROR: get_account_info failed to write msg_type\n.");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    msg_enum returned_msg;
    // read message type
    if ((returned_msg = readEnum(sockfd)) == -1) {
        perror("ERROR: get_account_info failed to read rsp_type\n.");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // make sure message type is account info
    if (returned_msg != ACCOUNT_INFO) {
        if (returned_msg == ERROR) {
            readEnum(sockfd); // burn extra message, the account didn't exist
            perror("ERROR: transaction invalid, account didn't exist");
            return;
        }
        perror("ERROR: get_account_info recieved the wrong rsp_type\n");
        error(sockfd, returned_msg);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // read username
    if ((username = readStringFromSocket(sockfd)) == NULL) {
        perror("ERROR: get_account_info failed to read username\n.");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // read name
    if ((name = readStringFromSocket(sockfd)) == NULL) {
        perror("ERROR: get_account_info failed to read name\n.");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // read birthday
    if (read(sockfd, &birthday, sizeof(time_t)) != sizeof(time_t)) {
        perror("ERROR: get_account_info failed to read birthday\n.");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    free(name);
    free(username);
}


// FUNCTION: REQUEST_HISTORY
// Request history of account
void request_history (int sockfd, int accountNum, int numTransactions) {
    msg_enum request_history_msg = REQUEST_HISTORY;

    if (writeEnum(sockfd, request_history_msg) == -1) {
        perror("ERROR: Request_history could not write message type.\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (write(sockfd, &accountNum, sizeof(int)) != sizeof(int)) {
        perror("ERROR: Request_history could not write account number.\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (write(sockfd, &numTransactions, sizeof(int)) != sizeof(int)) {
        perror("ERROR: Request_history could not write numTransactions.\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    msg_enum returned_msg;
    int rcvAccNum, rcvNumTransactions;

    if ((returned_msg = readEnum(sockfd)) == -1) {
        perror("ERROR: get_history failed to read msg\n.");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // make sure message type is account info
    if (returned_msg != HISTORY) {
        if (returned_msg == ERROR) {
            readEnum(sockfd); // burn extra message, the account didn't exist
            perror("ERROR: transaction invalid, account didn't exist");
            return;
        }
        perror("ERROR: get_history recieved the wrong msg\n");
        error(sockfd, returned_msg);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (read(sockfd, &rcvAccNum, sizeof(int)) != sizeof(int)) {
        perror("ERROR: request_history failed to read account number\n.");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (rcvAccNum != accountNum) {
        perror("ERROR: request_history returned the wrong account number\n.");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (read(sockfd, &rcvNumTransactions, sizeof(int)) != sizeof(int)) {
        perror("ERROR: request_history failed to read num transactions\n.");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    float transactions[rcvNumTransactions];
    for (int i = 0; i < rcvNumTransactions; i++) {
        if (read(sockfd, &transactions[i], sizeof(float)) != sizeof(float)) {
            perror("ERROR: request_history failed to read array of transactions\n.");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char *argv[]){
    /**
     * args
     *  - 1: input file
     *  - 2: local address
     *  - 3: port number
     */

    // Cash variable initialization
    // double cash = START_CASH;

    // argument handling
    if (argc != 4) {
        printSyntax();
        return 0;
    }

    int sockfd;
    struct sockaddr_in servaddr;

    // Get the correct filepath
    char file[MAX_STR] = "input/";
    strcat(file, argv[1]);

    // Get the server address and port number
    char *server_addr = argv[2];
    bzero(&servaddr, sizeof(servaddr));
    int port = atoi(argv[3]);

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(server_addr);
    servaddr.sin_port = htons(port);

    // Time complexity, necessary for assignment
    double cpu_time;
    clock_t begin, end;

    begin = clock();

    FILE *fp = fopen(file, "r");
    if (fp == NULL) {
      fprintf(stderr, "ERROR: failed to open file %s\n", file);
      exit(EXIT_FAILURE);
    }

    int results;
    msg_enum prev_msg;

    char* line = NULL; 
    size_t len = 0;

    while (getline(&line, &len, fp) != -1) {
        if (isConnected == 0) {
            sockfd = connectSocket(servaddr);
        }


        int message_type, account_number, num_transactions;
        char name[MAX_STR];
        char username[MAX_STR];
        time_t birthday;
        float amount;

        // scape the enum
        if (sscanf(line, "%d", &message_type) != 1) {
            // incomplete line
            perror("ERROR: main 1 - read in an incomplete line");
        }

        if (message_type < 0 || message_type > MSG_ENUM_SIZE) {
            continue;
        } else if (message_type == TERMINATE) {
            terminate(sockfd);
            prev_msg = message_type;
            continue;
        }

        int results = 0;
        if ((results = sscanf(line, "%d,%d,%64[^,],%64[^,],%ld,%f,%d\n", &message_type, &account_number, name, username, &birthday, &amount, &num_transactions)) != 7) {
            // incomplete line
            perror("ERROR: main 2 - read in an incomplete line");
            prev_msg = message_type;
            continue;
        }

        switch(message_type) {
            case REGISTER:
                register_user(sockfd, name, username, birthday);
                break;

            case GET_ACCOUNT_INFO:
                get_account_info(sockfd, account_number);
                break;

            case TRANSACT:
                transact(sockfd, account_number, amount);
                break;

            case GET_BALANCE:
                get_balance(sockfd, account_number);
                break;

            case REQUEST_HISTORY:
                request_history (sockfd, account_number, num_transactions);
                break;

            default:;
                // do nothing -- invalid msg -- keep reading
        }
        prev_msg = message_type;
    }

    if (prev_msg != TERMINATE) {
        terminate(sockfd);
    }

    if (line) {
        free(line);
    }

    end = clock();
    cpu_time = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Elapsed Time: %.2f\n", cpu_time);

    return 0;
}
