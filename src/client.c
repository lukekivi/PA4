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
    else
        printf("Socket successfully created..\n");

    // connect the client socket to server socket
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
      perror("ERROR: Connection with server failed.\n");
      close(sockfd);
      exit(EXIT_FAILURE);
    }
    else
        printf("Connected to the server..\n");

    isConnected = 1;

    return sockfd;
}

// FUNCTION: ERROR
// Generic error message to be sent when the enumerated message type does not match with any in the protocol
void error (int sock_fd, int message_type) {
    int msg_type = htonl(ERROR);
    int wrong_msg_type = htonl(message_type);

    if (write(sock_fd, &msg_type, sizeof(int)) != sizeof(int)) {
      perror("ERROR: Error could not write message type.\n");
      close(sock_fd);
      exit(EXIT_FAILURE);
    }

    if (write(sock_fd, &wrong_msg_type, sizeof(int)) != sizeof(int)) {
      perror("ERROR: Error could not write the incorrect message type recieved.\n");
      close(sock_fd);
      exit(EXIT_FAILURE);
    }
}

// FUNCTION: TERMINATE
// Alert the server of termination, once response is received
// Terminate connection
void terminate(int sockfd) {
    int convMsg = htonl(TERMINATE);

    if (write(sockfd, &convMsg, sizeof(int)) < 0) {
        perror("ERROR: terminate - Cannot write to sockfd\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    close(sockfd);
    isConnected = 0;
    return;
}

// FUNCTION: REGISTER
// Register a user
void register_user(int sockfd, char* name, char* username, time_t birthday) {
    msg_enum convMsg = htonl(REGISTER);
    int results;
    if (write(sockfd, &convMsg, sizeof(msg_enum)) != sizeof(msg_enum)) {
        perror("ERROR: register_user - Cannot write msg to sockfd\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (writeStringToSocket(sockfd, username) == 0) {
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (writeStringToSocket(sockfd, name) == 0) {
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (write(sockfd, &birthday, sizeof(time_t)) != sizeof(time_t)) {
        perror("ERROR: register_user - Cannot write birthday to sockfd\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    msg_enum tempMsg;

    if (read(sockfd, &tempMsg, sizeof(msg_enum)) < 0) {
        perror("ERROR: failed to read from sockfd\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    msg_enum msg = ntohl(tempMsg);

    if (msg != BALANCE) {
        perror("ERROR: failed to follow protocol\n");
        error(sockfd, msg);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    int tempAccountNumber;

    if (read(sockfd, &tempAccountNumber, sizeof(int)) != sizeof(int)) {
        perror("ERROR: failed to read from sockfd\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    int accountNumber = ntohl(tempAccountNumber);

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
    msg_enum msg = htonl(REQUEST_CASH);

    if (write(sockfd, &msg, sizeof(msg_enum)) != sizeof(msg_enum)) {
        perror("ERROR: Cannot write to socket\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (write(sockfd, &request, sizeof(float)) != sizeof(float)) {
        perror("ERROR: Cannot write to socket\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    msg_enum rcvMsgType;
    if (read(sockfd, &rcvMsgType, sizeof(msg_enum)) != sizeof(msg_enum)) {
        perror("ERROR: Cannot read account number\n.");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    int translatedMessage = ntohl(rcvMsgType);
    if (translatedMessage != CASH) {
        printf("Request cash recieved wrong response type.\n");
        error(sockfd, translatedMessage);
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
    int msg = htonl(GET_BALANCE);

    if (write(sockfd, &msg, sizeof(int)) != sizeof(int)) {
        perror("ERROR: Cannot write to socket\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    int accNum = htonl(accountNumber);
    if (write(sockfd, &accNum, sizeof(int)) != sizeof(int)) {
        perror("ERROR: Cannot write to socket\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    msg_enum rcvMsg;

    if (read(sockfd, &rcvMsg, sizeof(msg_enum)) != sizeof(msg_enum)) {
        perror("ERROR: Failed to read message type from socket\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    rcvMsg = ntohl(rcvMsg);
    if (rcvMsg != BALANCE) {
        perror("ERROR: Did not receive BALANCE back from sever after sending GET_BALANCE\n");
        error(sockfd, rcvMsg);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    int rcvAccount_number;
    if (read(sockfd, &rcvAccount_number, sizeof(int)) != sizeof(int)) {
        perror("ERROR: Failed to read account number from socket\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    rcvAccount_number = ntohl(rcvAccount_number);
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

    int rcvAccount_number;
    msg_enum msg_type, rcvMessage_type;

    // send TRANSACTION to the server
    msg_type = htonl(TRANSACT);
    if (write(sockfd, &msg_type, sizeof(msg_enum)) != sizeof(msg_enum)) {
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

    // balance is returned
    if (read(sockfd, &rcvMessage_type, sizeof(msg_enum)) != sizeof(msg_enum)) {
        perror("ERROR: Transact could not read message type.\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    rcvMessage_type = ntohl(rcvMessage_type);
    if (rcvMessage_type != BALANCE) {
        perror("ERROR: Did not receive BALANCE back from sever after sending GET_BALANCE\n");
        error(sockfd, rcvMessage_type);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (read(sockfd, &rcvAccount_number, sizeof(int)) != sizeof(msg_enum)) {
      perror("ERROR: Transact could not read account number.\n");
      close(sockfd);
      exit(EXIT_FAILURE);
    }

    rcvAccount_number = ntohl(rcvAccount_number);

    if (read(sockfd, &rcvBalance, sizeof(float)) != sizeof(float)) {
      perror("ERROR: Transact could not read balance.\n");
      close(sockfd);
      exit(EXIT_FAILURE);
    }
}

// FUNCTION: GET_ACCOUNT_INFO
// request the information for a specific account
void get_account_info (int sockfd, int acc_num) {
    msg_enum msg_type = htonl(GET_ACCOUNT_INFO);
    msg_enum rsp_type;

    char* name = (char*)malloc(sizeof(char)*MAX_STR);
    char* username = (char*)malloc(sizeof(char)*MAX_STR);
    time_t birthday;

    // write message type
    if (write(sockfd, &msg_type, sizeof(msg_enum)) != sizeof(msg_enum)) {
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


    // read message type
    if (read(sockfd, &rsp_type, sizeof(msg_enum)) != sizeof(msg_enum)) {
        perror("ERROR: get_account_info failed to read rsp_type\n.");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // make sure message type is account info
    if (rsp_type = ntohl(rsp_type) != ACCOUNT_INFO) {
        perror("ERROR: get_account_info recieved the wrong rsp_type\n");
        error(sockfd, rsp_type);
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
    msg_enum msgType = htonl(REQUEST_HISTORY);
    int msgAccount = htonl(accountNum);
    int msgNumTransactions = htonl(numTransactions);

    if (write(sockfd, &msgType, sizeof(msg_enum)) != sizeof(msg_enum)) {
      perror("ERROR: Request_history could not write message type.\n");
      close(sockfd);
      exit(EXIT_FAILURE);
    }

    if (write(sockfd, &msgAccount, sizeof(int)) != sizeof(int)) {
      perror("ERROR: Request_history could not write account number.\n");
      close(sockfd);
      exit(EXIT_FAILURE);
    }

    if (write(sockfd, &msgNumTransactions, sizeof(int)) != sizeof(int)) {
      perror("ERROR: Request_history could not write numTransactions.\n");
      close(sockfd);
      exit(EXIT_FAILURE);
    }

    int rcvAccNum;
    int rcvNumTransactions;

    if (read(sockfd, &rcvAccNum, sizeof(int)) != sizeof(int)) {
        perror("ERROR: request_history failed to read account number\n.");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    rcvAccNum = ntohl(rcvAccNum);
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

    rcvNumTransactions = ntohl(rcvNumTransactions);
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

    // Variable declaration
    int message_type, account_number, num_transactions;
    char *line = (char*)malloc(sizeof(char)*MAX_STR);
    char* name = (char*)malloc(sizeof(char)*MAX_STR);
    char* username = (char*)malloc(sizeof(char)*MAX_STR);
    int nameLen, usernameLen;
    long birthday;
    float amount;

    FILE *fp = fopen(file, "r");
    if (fp == NULL) {
      fprintf(stderr, "ERROR: failed to open file %s\n", file);
      exit(EXIT_FAILURE);
    }

    int i = 0;
    while (fscanf(fp, "%d,%d,%64[^,],%64[^,],%ld,%f,%d\n",
            &message_type, &account_number, name, username, &birthday, &amount, &num_transactions) != EOF) {
        // printf("i: %d \n", i++);
        if (isConnected == 0) {
            sockfd = connectSocket(servaddr);
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

            case ERROR:
                error(sockfd, message_type);
                break;

            case TERMINATE:
                terminate(sockfd);
                break;

            case REQUEST_HISTORY:
                request_history (sockfd, account_number, num_transactions);
                break;

            default:
                perror("ERROR: Invalid message type read from input file\n");
                free(line); free(name); free(username); close(sockfd); fclose(fp);
                exit(EXIT_FAILURE);
        }
    }

    free(line); free(name); free(username); close(sockfd); fclose(fp);

    end = clock();
    cpu_time = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Elapsed Time: %.2f\n", cpu_time);

    return 0;
}
