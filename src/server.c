#include "server.h"

void printSyntax(){
    printf("incorrect usage syntax! \n");
    printf("usage: $ ./server server_addr server_port num_workers\n");
}

void log() {
    // wait 30 seconds

    // write log to file
    // iterate over each account in global balance (thread safe)

    // output to balances.csv
    // account number,balance,name,username,birthday
    // "%d,%.2f,%s,%s,%f\n"

}

void worker() {
    // Parse each query received

    // Respond with appropriate message

    // If it modifies the global balance it should signal log thread's condition variable

    // Continue until terminate query is received from client

    // CLose connection and return
}

int main(int argc, char *argv[]){
    // argument handling
    if(argc != 4)
    {
        printSyntax();
        return 0;
    }

    // Check account number

    // Check port number

    // Check number of workers

    // create empty output folder
    bookeepingCode();

    // Start log thead
    pthread_t logThread;

    if (pthread_create(&logThread, NULL, , NULL) != 0) {
        fprintf(stderr, "ERROR: Failed to start log thread\n");
        exit(EXIT_FAILURE);
    }

    // Create a socket

    // Begin listening

    /*  
        For each incoming connection 
        - Create a worker thread and pass it connections file descriptor
        - Return to listening on the socket
    */

    



    return 0; 
}

