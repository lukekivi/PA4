#include "../include/client.h"

void printSyntax(){
    printf("incorrect usage syntax! \n");
    printf("usage: $ ./client input_filename server_addr server_port\n");
}

void func(int sockfd, int msg) {
    int convMsg = htonl(msg);

    if (write(sockfd, &convMsg, sizeof(int)) < 0) {
        perror("Cannot write");
        exit(1);
    }
    
    while (1) {
        int temp;
        int results = read(sockfd, &temp, sizeof(int));
        
        int rcv = ntohl(temp);
        
        if (results < 0) {
            perror("cannot read");
            exit(1);
        } else if (results > 0) {
            msg_enum msg = rcv;
            printEnumName(msg);
            if (msg == TERMINATE) {
                close(sockfd);
                exit(EXIT_SUCCESS);
            }
            break;
        }
    }
}

void funcRegister(int sockfd) {
    int convMsg = htonl(REGISTER);

    if (write(sockfd, &convMsg, sizeof(int)) < 0) {
        perror("Cannot write");
        exit(1);
    }

    char* name = "Lucas Kivi";
    char* username = "kivix019";
    time_t birthday = 753131776;

    if (writeStringToSocket(sockfd, username) == 0) {
        exit(1);
    }
    
    if (writeStringToSocket(sockfd, name) == 0) {
        exit(1);
    }

    if (write(sockfd, &birthday, sizeof(time_t)) < 0) {
        perror("Cannot write");
        exit(1);
    }
    
    while (1) {
        msg_enum temp;
        int results = read(sockfd, &temp, sizeof(msg_enum));
        
        if (results < 0) {
            perror("ERROR: failed to read from sockfd\n");
            exit(EXIT_FAILURE);
        } else if (results > 0) {
            
            msg_enum msg = ntohl(temp);;
            
            if (msg != BALANCE) {
                perror("ERROR: failed to follow protocol\n");
                exit(EXIT_FAILURE);
            } else {
                printEnumName(msg);
                break;
            }
        }
    }

    while (1) {
        int temp;
        int results = read(sockfd, &temp, sizeof(int));
        
        if (results < 0) {
            perror("ERROR: failed to read from sockfd\n");
            exit(EXIT_FAILURE);
        } else if (results > 0) {
            
            int accountNumber = ntohl(temp);;
            
            printf("Account Number: %d\n", accountNumber);
            break;
        }
    }

    while (1) {
        float balance;
        int results = read(sockfd, &balance, sizeof(float));
        
        if (results < 0) {
            perror("ERROR: failed to read from sockfd\n");
            exit(EXIT_FAILURE);
        } else if (results > 0) {        
            printf("Balance: %.2f\n", balance);
            break;
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
        exit(0);
    }
    else
        printf("Connected to the server..\n");

    // Time complexity, necessary for assignment
    double cpu_time;
    clock_t begin, end;

    begin = clock();


    funcRegister(sockfd);
    // func(sockfd, GET_ACCOUNT_INFO);
    // func(sockfd, TRANSACT);
    // func(sockfd, REGISTER);
    // func(sockfd, GET_BALANCE);
    // func(sockfd, REQUEST_CASH);
    // func(sockfd, REQUEST_HISTORY);
    func(sockfd, TERMINATE);
    
    close(sockfd);

    end = clock();
    cpu_time = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Elapsed Time: %.2f\n", cpu_time);

    return 0; 
}
