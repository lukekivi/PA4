#include "../include/client.h"

void printSyntax(){
    printf("incorrect usage syntax! \n");
    printf("usage: $ ./client input_filename server_addr server_port\n");
}

void func(int sockfd, int message_type, int account_number, char* name, char* username, long birthday, float amount, int num_transactions) {

    char buf[MSG_BUFFER_SIZE];
    sprintf(buf, "%d", msg);


    if (write(sockfd, buf, MSG_BUFFER_SIZE) < 0) {
        perror("Cannot write");
        exit(1);
    }


    while (1) {
        char rcv[MSG_BUFFER_SIZE];
        int results = read(sockfd, rcv, MSG_BUFFER_SIZE);

        if (results < 0) {
            perror("cannot read");
            exit(1);
        } else if (results > 0) {
            msg_enum msg = atoi(rcv);

            switch (msg) {
              case ACCOUNT_INFO:
                // need to read in string username, string name, and long birthday

                break;
              case BALANCE:
                  // need to read in int account_number and float balance
                  results = pread(sockfd, rcv, MSG_BUFFER_SIZE, 4);
                  int account_number = atoi(rcv);
                  results = pread(sockfd, rcv, MSG_BUFFER_SIZE, 8);
                  float amount = atof(rcv);
                  // balance(account_number, amount);

                  break;
              case CASH:
                    // need to read in float cash
                    results = pread(sockfd, rcv, MSG_BUFFER_SIZE, 4);
                    float amount = atof(rcv);
                    // cash(amount);

                    break;
            }
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

    for (int i = 0; i < MSG_ENUM_SIZE-2; i++)
        func(sockfd, i);


    end = clock();
    cpu_time = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Elapsed Time: %.2f\n", cpu_time);

    return 0;
}
