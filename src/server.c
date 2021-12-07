#include "server.h"


void printSyntax() {
    printf("incorrect usage syntax! \n");
    printf("usage: $ ./server server_addr server_port num_workers\n");
}

// Commented out because it's not for interim
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

void* worker(void* arg) {
     /**
     * args
     *   - 1: local address
     *   - 2: port number
     *   - 3: num workers
     */

    int sockfd = *((int *) arg);

    // re-transmit enumerated value to client
    // when server receives TERMINATE, re-transmit to client, close the connection and return
    char* buf = NULL;
    while (1) {
        buf = (char* ) malloc(sizeof(char) * MSG_ENUM_SIZE);

        int results = read(sockfd, buf, MSG_ENUM_SIZE);

        if (results < 0 ) {
            perror("cannot read");
            exit(1);
        } else if (results > 0) {
            printf("Worker: %s\n", buf);
        }

        free(buf);
    }

    // close socket
    close(sockfd);
}

int main(int argc, char *argv[]) {

    int sockfd, len, connfd;
    struct sockaddr_in servaddr, cli;

    // argument handling
    if (argc != 4) {
        printSyntax();
        return 0;
    }

    // create empty output folder
    bookeepingCode();

    // Commented out because it's not for interim
    // // Start log thead
    // pthread_t logThread;

    // if (pthread_create(&logThread, NULL, log, NULL) != 0) {
    //      fprintf(stderr, "ERROR: Failed to start log thread\n");
    //      exit(EXIT_FAILURE);
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
    bzero(&servaddr, sizeof(servaddr)); 

    // int nWorkers = argv[3]; // Commented out because it's not for interim

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(atoi(argv[2]));

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) {
        printf("Socket bind failed...\n");
        exit(0);
    }
    else
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

    // create worker threads
    // int* workerThreads[nWorkers];
    // for (int i = 0; i < nWorkers; i++) {
    //     if (pthread_create(&workerThreads[i], NULL, worker, NULL) != 0) {
    //         fprintf(stderr, "ERROR: Failed to start worker thread\n");
    //         exit(EXIT_FAILURE);
    //     }
    // } 

    while (1) {
        // Accept the data packet from client and verification
        connfd_arr[count % NCLIENTS] = accept(sockfd, (struct sockaddr *)&cli, &len);
        if (connfd_arr[count % NCLIENTS] < 0) {
            printf("Server accept failed...\n");
            exit(0);
        } else
            printf("Server accept the client...\n");

        pthread_create(&tid, NULL, worker, (void *)&connfd_arr[count % NCLIENTS]);
    }

    // Server never shut down
    close(sockfd);
}
