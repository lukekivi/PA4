#include "../include/client.h"

float clientCash = START_CASH;

void printSyntax(){
    printf("incorrect usage syntax! \n");
    printf("usage: $ ./client input_filename server_addr server_port\n");
}

// FUNCTION: TERMINATE
// Alert the server of termination, once response is received
// Terminate connection
void terminate(int sockfd) {
    int convMsg = htonl(TERMINATE);

    if (write(sockfd, &convMsg, sizeof(int)) < 0) {
        perror("ERROR: Cannot write to sockfd\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
        
    close(sockfd);
    return;
}

// FUNCTION: REGISTER
// Register a user
void register_user(int sockfd, char* name, char* username, time_t birthday) {
    int convMsg = htonl(REGISTER);

    if (write(sockfd, &convMsg, sizeof(int)) != sizeof(int)) {
        perror("ERROR: Cannot write to sockfd\n");
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
        perror("ERROR: Cannot write to sockfd\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    
    msg_enum tempMsg;
        
    if (read(sockfd, &tempMsg, sizeof(msg_enum)) < 0) {
        perror("ERROR: failed to read from sockfd\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
            
    msg_enum msg = ntohl(tempMsg);;
            
    if (msg != BALANCE) {
        perror("ERROR: failed to follow protocol\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    } else {
        printEnumName(msg);
    }

    int tempAccountNumber;
        
    if (read(sockfd, &tempAccountNumber, sizeof(int)) != sizeof(int)) {
        perror("ERROR: failed to read from sockfd\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    } 
            
    int accountNumber = ntohl(tempAccountNumber);           
    printf("Account Number: %d\n", accountNumber);

    float balance;        

    if (read(sockfd, &balance, sizeof(float)) != sizeof(float)) {
        perror("ERROR: failed to read from sockfd\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    
    printf("Balance: %.2f\n", balance);
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

    printf("Msg: %d\n", translatedMessage);
    
    if (translatedMessage != CASH) {
        printf("Request cash recieved wrong response type.\n");
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
        printf("ERROR: get_balance recieved the wrong account number.\n");
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
        perror("Cannot write");
        exit(1);
    }

    if (write(sockfd, &account_number, sizeof(int)) != sizeof(int)) {
        perror("Cannot write");
        exit(1);
    }

    if (write(sockfd, &amount, sizeof(float)) != sizeof(float)) {
        perror("Cannot write");
        exit(1);
    }

    // add the value of the transaction to the cash variable
    clientCash += amount;

    // balance is returned
    if (read(sockfd, &rcvMessage_type, sizeof(msg_enum)) != sizeof(msg_enum)) {
        perror("Cannot read message type.");
        exit(1);
    } 
      
    rcvMessage_type = ntohl(rcvMessage_type);
    printf("TRANSACT: msg : %d\n", rcvMessage_type);
  
    if (read(sockfd, &rcvAccount_number, sizeof(int)) != sizeof(msg_enum)) {
        perror("Cannot read account number.");
        exit(1);
    } 
    
    rcvAccount_number = ntohl(rcvAccount_number);
      
    if (read(sockfd, &rcvBalance, sizeof(float)) != sizeof(float)) {
      perror("Cannot read balance.");
      exit(1);
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
        perror("get_account_info failed to write msg_type\n.");
        close(sockfd);
        exit(1);
    }

    // write account number
    if (write(sockfd, &acc_num, sizeof(int)) != sizeof(int)) {
        perror("get_account_info failed to write msg_type\n.");
        close(sockfd);
        exit(1);
    }


    // read message type
    if (read(sockfd, &rsp_type, sizeof(msg_enum)) != sizeof(msg_enum)) {
        perror("get_account_info failed to read rsp_type\n.");
        close(sockfd);
        exit(1);
    }
    
    // make sure message type is account info
    if (rsp_type = ntohl(rsp_type) != ACCOUNT_INFO) {
        perror("get_account_info recieved the wrong rsp_type\n");
        close(sockfd);
        exit(1);
    }

    // read username
    if ((username = readStringFromSocket(sockfd)) == NULL) {
        perror("get_account_info failed to read username\n.");
        close(sockfd);
        exit(1);
    }
  
    // read name
    if ((name = readStringFromSocket(sockfd)) == NULL) {
        perror("get_account_info failed to read name\n.");
        close(sockfd);
        exit(1);
    }
  
    // read birthday
    if (read(sockfd, &birthday, sizeof(time_t)) != sizeof(time_t)) {
        perror("get_account_info failed to read birthday\n.");
        close(sockfd);
        exit(1);
    }

    free(name);
    free(username);
}

// FUNCTION: ERROR
// Generic error message to be sent when the enumerated message type does not match with any in the protocol
void error (int sock_fd, int message_type) {
    int msg_type = htonl(ERROR);
    int wrong_msg_type = htonl(message_type);

    if (write(sock_fd, &msg_type, sizeof(int)) != sizeof(int)) {
        perror("error failed to write msg_type\n.");
        exit(1);
    }

    if (write(sock_fd, &wrong_msg_type, sizeof(int)) != sizeof(int)) {
        perror("error failed to write wrong_msg_type\n.");
        exit(1);
    }
}

// FUNCTION: REQUEST_HISTORY
// Request history of account
void request_history (int sockfd, int accountNum, int numTransactions) {
    msg_enum msgType = htonl(REQUEST_HISTORY);
    int msgAccount = htonl(accountNum);
    int msgNumTransactions = htonl(numTransactions);
    
    if (write(sockfd, &msgType, sizeof(msg_enum)) != sizeof(msg_enum)) {
        printf("request_history failed to write msg_type\n.");
        exit(1);
    }

    if (write(sockfd, &msgAccount, sizeof(int)) != sizeof(int)) {
        perror("request_history failed to write account_num\n.");
        exit(1);
    }

    if (write(sockfd, &msgNumTransactions, sizeof(int)) != sizeof(int)) {
        perror("request_history failed to write msgNumTransactions\n.");
        exit(1);
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
    printf("Numtransactions: %d\n", rcvNumTransactions);
    for (int i = 0; i < rcvNumTransactions; i++) {
        if (read(sockfd, &transactions[i], sizeof(float)) != sizeof(float)) {
            perror("ERROR: request_history failed to read array of transactions\n.");
            close(sockfd);
            exit(EXIT_FAILURE);
        }   
        printf("Arr[%d] = %f\n", i, transactions[i]);
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

    // Socket create and verification
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");

    // Get the correct filepath
    char file[MAX_STR] = "input/";
    strcat(file, argv[1]);

    // Get the server address
    char *server_addr = argv[2];
    bzero(&servaddr, sizeof(servaddr));


    int port = atoi(argv[3]);
    
    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(server_addr);
    servaddr.sin_port = htons(port);

    // connect the client socket to server socket
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
        printf("Connection with the server failed...\n");
        exit(EXIT_FAILURE);
    }
    else
        printf("Connected to the server..\n");

    // Time complexity, necessary for assignment
    double cpu_time;
    clock_t begin, end;

    begin = clock();

    char* name = "Lucas Kivi";
    char* username = "kivix019";
    time_t birthday = 753131776;


    register_user(sockfd, name, username, birthday);

    printf("Received balance: %f\n", get_balance(sockfd, 0));
    transact(sockfd, 0, 20000.00);
    printf("Received balance: %f\n", get_balance(sockfd, 0));

    printf("Received balance: %f\n", get_balance(sockfd, 0));
    transact(sockfd, 0, 20000.00);
    printf("Received balance: %f\n", get_balance(sockfd, 0));

    printf("Received balance: %f\n", get_balance(sockfd, 0));
    transact(sockfd, 0, -900.00);
    printf("Received balance: %f\n", get_balance(sockfd, 0));

    get_account_info(sockfd, 0);

    request_history(sockfd, 0, 1);

    terminate(sockfd);
    
    close(sockfd);

    end = clock();
    cpu_time = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Elapsed Time: %.2f\n", cpu_time);

    return 0; 
}
