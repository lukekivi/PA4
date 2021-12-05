#include "../include/client.h"

void printSyntax(){
    printf("incorrect usage syntax! \n");
    printf("usage: $ ./client input_filename server_addr server_port\n");
}

int main(int argc, char *argv[]){
    // Cash variable initialization
    double cash = START_CASH;

    // argument handling
    if(argc != 4) {
        printSyntax();
        return 0;
    }

    // Get the correct filepath
    char file[MAX_STR] = "input/";
    strcat(file, argv[1]);
    printf("FILE: %s\n", file);

    // Temporary
    char *serverAddress = argv[2];
    printf("SERVER_ADDR: %s\n", serverAddress);

    // Temporary
    char *serverPort = argv[3];
    printf("SERVER PORT: %s\n", serverPort);

    // Network stuff. Not touching until later

    // struct sockaddr_in servaddr;
    //
    // servaddr.sin_family = AF_INET;
    // servaddr.sin_addr.s_addr = htonl(argv[2]);
    // servaddr.sin_port = htons(argv[3]);

    // Time complexity, necessary for assignment
    double cpu_time;
    clock_t begin, end;

    begin = clock();

    // Variable declaration
    int message_type, account_number, num_transactions;
    char *line = (char *)malloc(sizeof(char) * MAX_STR);
    char *name = (char *)malloc(sizeof(char) * MAX_STR);
    char *username = (char *)malloc(sizeof(char) * MAX_STR);
    long birthday;
    float amount;

    FILE *fp = fopen(file, "r");
    if (fp == NULL) {
        fprintf(stderr, "ERROR: failed to open file %s\n", file);
        exit(EXIT_FAILURE);
    }

    // Read in variables. Currently there is a bug that first string isn't delimited by
    // the commas, so it takes the whole rest of the line. Still trying to figure out clean solution.
    while (fscanf(fp, "%d,%d,%s,%s,%ld,%f,%d\n", &message_type, &account_number,
                  name, username, &birthday, &amount, &num_transactions) != EOF) {

        printf("%d,%d,%s,%s,%ld,%f,%d\n", message_type, account_number,
               name, username, birthday, amount, num_transactions); // testing

        printf("%d\n", message_type);     // testing
        printf("%d\n", account_number);   // testing
        printf("%s\n", name);             // testing
        printf("%s\n", username);         // testing
        printf("%ld\n", birthday);        // testing
        printf("%f\n", amount);           // testing
        printf("%d\n", num_transactions); // testing
    }

    free(name);
    free(username);

    end = clock();
    cpu_time = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Elapsed Time: %.2f\n", cpu_time);

    return 0; 
}

