#include "../include/client.h"

float clientCash = START_CASH;

void printSyntax(){
    printf("incorrect usage syntax! \n");
    printf("usage: $ ./client input_filename server_addr server_port\n");
}

void terminate(int sockfd) {
    int convMsg = htonl(TERMINATE);

    if (write(sockfd, &convMsg, sizeof(int)) < 0) {
        perror("ERROR: Cannot write to sockfd\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    int temp;
    if (read(sockfd, &temp, sizeof(int)) != sizeof(int)) {
        perror("ERROR: Cannot read from sockfd\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
        
    msg_enum rcv = ntohl(temp);

    if (rcv != TERMINATE) {
        perror("ERROR: Didn't receive TERMINATE back from server\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
        
    printEnumName(rcv);
    close(sockfd);
    exit(EXIT_SUCCESS);
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
void request_cash (int sockfd) {
    float request = CASH_AMOUNT;
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

    printf("Starting cash: %f\n", clientCash);
    request_cash(sockfd);
    printf("Ending cash: %f\n", clientCash);

    terminate(sockfd);
    
    close(sockfd);

    end = clock();
    cpu_time = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Elapsed Time: %.2f\n", cpu_time);

    return 0; 
}
