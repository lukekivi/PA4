#include "client.h"

void printSyntax() {
    printf("incorrect usage syntax! \n");
    printf("usage: $ ./client input_filename server_addr server_port\n");
}

// If I understand interim correctly, we are just doing message_type for now?
void func(int sockfd, int message_type){
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

int main(int argc, char *argv[]) {
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

    // In the design document, client only has 2 arguments input_filename and server_addr
    // At the top of this document, there is a comment with 3. For now, I will assume the
    // document was correct in that there was 2. Obviously can be changed.
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

    for (int i = REGISTER; i < TERMINATE; i++) {
        func(sockfd, i);
    }

    // SECTION BELOW IS COMMENTED OUT BECAUSE IT DOES NOT APPLY TO INTERIM

    // // Variable declaration
    // int message_type, account_number, num_transactions;
    // char *line = (char*)malloc(sizeof(char)*MAX_STR);
    // char* name = (char*)malloc(sizeof(char)*MAX_STR);
    // char* username = (char*)malloc(sizeof(char)*MAX_STR);
    // long birthday;
    // float amount;
    //
    // // Necessary for reading in variables
    // char *token, *saveptr;
    // char *pattern = ",";
    //
    // FILE *fp = fopen(file, "r");
    // if (fp == NULL) {
    //   fprintf(stderr, "ERROR: failed to open file %s\n", file);
    //   exit(EXIT_FAILURE);
    // }
    //
    // while (fscanf(fp, "%d,%d,%s\n", &message_type, &account_number, line) != EOF) {
    //   // I hate this solution but its what working at the moment. fscanf and sscanf do not
    //   // separate with commas, so the string will just be the end of the line. Used viet's
    //   // parse to get the correct fields.
    //   token = strtok_r(line, pattern, &saveptr);
    //   sscanf(token, "%s", name);
    //   token = strtok_r(NULL, pattern, &saveptr);
    //   sscanf(token, "%s", username);
    //   token = strtok_r(NULL, pattern, &saveptr);
    //   sscanf(token, "%ld,%f,%d\n", &birthday, &amount, &num_transactions);
    //
    //   printf("%d,%d,%s,%s,%ld,%f,%d\n", message_type, account_number,
    //   name, username, birthday, amount, num_transactions); // testing
    //
    //   func(sockfd, message_type);
    // }
    //
    // func(sockfd, TERMINATE);
    // free(name);
    // free(username);

    end = clock();
    cpu_time = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Elapsed Time: %.2f\n", cpu_time);

    return 0;
}