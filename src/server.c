#include "server.h"


void printSyntax() {
    printf("incorrect usage syntax! \n");
    printf("usage: $ ./server server_addr server_port num_workers\n");
}

// void log() {
//     // Get the correct filepath
//     char file[MAX_STR] = "output/";
//     printf("FILE: %s\n", file);

//     sleep(LOGGER_SLEEP);
//     FILE *fp = fopen(file, "w");
//     if (fp == NULL) {
//         fprintf(stderr, "ERROR: failed to open file %s\n", file);
//         exit(EXIT_FAILURE);
//     }
// }

void* worker(void *arg) {
    int sockfd = *(int *) arg;
    char clientid[MAX_STR];
    memset(clientid, 0, MAX_STR);
    int size = read(sockfd, clientid, sizeof(clientid));
    if (size < 0) {
        perror("read error");
        exit(1);
    }
    printf("\t CLient (client ID: %s) has logged in.\n", clientid);

    // receive enumerated value from client and print it to standard output
    char recv[MAX_STR];
    memset(recv, 0, MAX_STR);
    if (read(sockfd, recv, strlen(recv)) < 0) {
        perror("read error");
        exit(1);
    }
    printf("\tReceived \"%s\" from client.\n", recv);

    // re-transmit enumerated value to client
    // when server receives TERMINATE, re-transmit to client, close the connection and return
    do {
        if (write(sockfd, recv, strlen(recv)) < 0) {
            perror("Cannot write");
            exit(1);
        }
        printf("\tRe-transmit %s to client.\n", recv);
    } while (strcmp(recv, "TERMINATE") != 0);
}

int main(int argc, char *argv[]) {
    int sockfd, len;
    struct sockaddr_in servaddr, cli;

    // argument handling
    if (argc != 4) {
        printSyntax();
        return 0;
    }

    // create empty output folder
    bookeepingCode();

    // // Start log thead
    // pthread_t logThread;

    // if (pthread_create(&logThread, NULL, log, NULL) != 0) {
    //     fprintf(stderr, "ERROR: Failed to start log thread\n");
    //     exit(EXIT_FAILURE);
    // }

    // create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Socket creation failed...\n");
        exit(0);
    }
    else {
        printf("Socket successfully created...\n");
    }

    bzero(&servaddr, sizeof(servaddr)); // ???

    // Local host
    char* LOCAL_HOST = argv[1];
    // Port
    char* PORT = argv[2];

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(LOCAL_HOST);
    servaddr.sin_port = htons(inet_addr(PORT));

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0) {
        printf("Socket bind failed...\n");
        exit(0);
    } else
        printf("Socket successfully binded..\n");

    // Now server is ready to listen and verification
    if ((listen(sockfd, NCLIENTS)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    }
    else
        printf("Server listening..\n");
    len = sizeof(cli);

    int count = 0;
    pthread_t tid;
    int connfd_arr[NCLIENTS];

    // Num workers
    nWorkers = atoi(argv[3]);

    while (1) {
        // Accept the data packet from client and verification
        connfd_arr[count % NCLIENTS] = accept(sockfd, (SA *)&cli, &len);
        if (connfd_arr[count % NCLIENTS] < 0) {
            printf("Server accept failed...\n");
            exit(0);
        } else
            printf("Server accept the client...\n");

        // TODO: pthread_create and pass client socket (connfd_arr[]) to the thread
        int* workerThreads[nWorkers];
        for (int i = 0; i < nWorkers; i++) {
            if ((pthread_create(&workerThreads[i], NULL, worker, (void *)&connfd_arr[count % NCLIENTS])) != 0) {
                fprintf(stderr, "ERROR: Failed to start worker thread\n");
                exit(EXIT_FAILURE);
            }
        }  
        
        count++;
    }

    // Server never shut down
    close(sockfd);
}
