#include "../include/client.h"

void printSyntax(){
    printf("incorrect usage syntax! \n");
    printf("usage: $ ./client input_filename server_addr server_port\n");
}

// If I understand interim correctly, we are just doing message_type for now?
void func(int sockfd, int message_type) {
    int ENUMLENGTH = 2; // digits 0-11, so message_type length max is 2
    char message[ENUMLENGTH];
    memset(message, 0, ENUMLENGTH);
    sprintf(message, "%d", message_type);

    if (write(sockfd, message, strlen(message)) < 0) {
        perror("Cannot write");
        exit(1);
    }

    char recv[ENUMLENGTH];
    memset(recv, 0, ENUMLENGTH);
    if (read(sockfd, recv, ENUMLENGTH) < 0) {
        perror("cannot read");
        exit(1);
    }
    printf("Messaged received from server: %s\n", recv);
}

int main(int argc, char *argv[]){
    // Cash variable initialization
    // double cash = START_CASH;

    // argument handling
    if(argc != 4) {
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
    bzero(&servaddr, sizeof(servaddr));

    // Get the correct filepath
    char file[MAX_STR] = "input/";
    strcat(file, argv[1]);
    printf("FILE: %s\n", file);

    // Get the server address
    char *server_addr = argv[2];
    int intServerAddr = atoi(server_addr);

    // In the design document, client only has 2 arguments input_filename and server_addr
    // At the top of this document, there is a comment with 3. For now, I will assume the
    // document was correct in that there was 2. Obviously can be changed.
    char *port = "0000";

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

    end = clock();
    cpu_time = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Elapsed Time: %.2f\n", cpu_time);

    return 0; 
}

