#include "../include/client.h"

void printSyntax(){
    printf("incorrect usage syntax! \n");
    printf("usage: $ ./client input_filename server_addr server_port\n");
}

void func(int sockfd, int msg) {
    printf("Emitting: %d\n", msg);
    char buf[MSG_ENUM_SIZE];
    sprintf(buf, "%d", (int) msg);

    if (write(sockfd, buf, sizeof(buf)) < 0) {
        perror("Cannot write");
        exit(1);
    }

    // char recv[MAX_ENUM_LENGTH];
    // memset(recv, 0, MAX_ENUM_LENGTH);
    // if (read(sockfd, recv, MAX_ENUM_LENGTH) < 0) {
    //     perror("cannot read");
    //     exit(1);
    // }
    // printf("Messaged received from server: %s\n", recv);
    // printEnumName(*((msg_enum*) msg));
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
    printf("FILE: %s\n", file);

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


    // // Variable declaration
    // int message_type, account_number, num_transactions;
    // char *line = (char *)malloc(sizeof(char) * MAX_STR);
    // char *name = (char *)malloc(sizeof(char) * MAX_STR);
    // char *username = (char *)malloc(sizeof(char) * MAX_STR);
    // long birthday;
    // float amount;

    // FILE *fp = fopen(file, "r");
    // if (fp == NULL) {
    //     fprintf(stderr, "ERROR: failed to open file %s\n", file);
    //     exit(EXIT_FAILURE);
    // }

    // // Read in variables. Currently there is a bug that first string isn't delimited by
    // // the commas, so it takes the whole rest of the line. Still trying to figure out clean solution.
    // while (fscanf(fp, "%d,%d,%s,%s,%ld,%f,%d\n", &message_type, &account_number,
    //               name, username, &birthday, &amount, &num_transactions) != EOF) {

    //     printf("%d,%d,%s,%s,%ld,%f,%d\n", message_type, account_number,
    //            name, username, birthday, amount, num_transactions); // testing

    //     printf("%d\n", message_type);     // testing
    //     printf("%d\n", account_number);   // testing
    //     printf("%s\n", name);             // testing
    //     printf("%s\n", username);         // testing
    //     printf("%ld\n", birthday);        // testing
    //     printf("%f\n", amount);           // testing
    //     printf("%d\n", num_transactions); // testing
    // }

    // free(name);
    // free(username);
    
    for (int i = 0; i < MSG_ENUM_SIZE; i++) {
        func(sockfd, i);
    }
    

    end = clock();
    cpu_time = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Elapsed Time: %.2f\n", cpu_time);

    return 0; 
}

