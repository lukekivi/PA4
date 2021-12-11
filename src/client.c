#include "../include/client.h"
double clientCash = START_CASH;

void printSyntax(){
    printf("incorrect usage syntax! \n");
    printf("usage: $ ./client input_filename server_addr server_port\n");
}

void func(int sockfd, int message_type, int account_number, int nameLen, char* name, int usernameLen, char* username, long birthday, float amount, int num_transactions) {

    char buf[MSG_BUFFER_SIZE];
    sprintf(buf, "%d", message_type);


    if (write(sockfd, buf, MSG_BUFFER_SIZE) < 0) {
        perror("Cannot write");
        exit(1);
    }


    while (1) {
        char rcv[MSG_BUFFER_SIZE];
        int results = read(sockfd, rcv, MSG_BUFFER_SIZE);
        float rcvAmount;

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
                  int rcvAccount_number = atoi(rcv);
                  results = pread(sockfd, rcv, MSG_BUFFER_SIZE, 8);
                  rcvAmount = atof(rcv);
                  // balance(account_number, amount);

                  break;
              case CASH:
                    // need to read in float cash
                    results = pread(sockfd, rcv, MSG_BUFFER_SIZE, 4);
                    rcvAmount = atof(rcv);
                    clientCash += rcvAmount;

                    // cash(amount);

                    break;
            }
        }
    }

}

void transact (int sockfd, int account_number, float amount) {
  // send a GET_BALANCE message to the server to ensure
  // that the account will not go negative
  msg_enum msg_type = GET_BALANCE;
  int amt = 0;

  if ((amt =write(sockfd, &msg_type, sizeof(msg_enum))) != sizeof(msg_enum)) {
      perror("Cannot write");
      exit(1);
  }
  if ((amt =write(sockfd, &account_number, sizeof(int))) != sizeof(int)) {
      perror("Cannot write");
      exit(1);
  }

  msg_enum rcvMessage_type;
  int rcvAccount_number;
  float rcvBalance;
  if ((amt = read(sockfd, &rcvMessage_type, sizeof(msg_enum))) != sizeof(msg_enum)) {
    perror("Cannot read message type.");
    exit(1);
  }
  if ((amt = read(sockfd, &rcvAccount_number, sizeof(int))) != sizeof(msg_enum)) {
    perror("Cannot read account number.");
    exit(1);
  }
  if ((amt = read(sockfd, &rcvBalance, sizeof(float))) != sizeof(float)) {
    perror("Cannot read balance.");
    exit(1);
  }

  if ((rcvBalance - amount) < 0) {
    return; // if account can go negative, ignore transact
  }

  // send GET_CASH to server until the cash variable will not go negative.
  while (clientCash - amount < 0) {
    char getCashBuf[MAX_STR];
    msg_type = REQUEST_CASH;

    if ((amt = write(sockfd, &msg_type, sizeof(msg_enum))) != sizeof(msg_enum)) {
        perror("Cannot write");
        exit(1);
    }

    if ((amt = write(sockfd, &amount, sizeof(float))) != sizeof(float)) {
        perror("Cannot write");
        exit(1);
    }

    if ((amt = read(sockfd, &rcvMessage_type, sizeof(msg_enum))) != sizeof(msg_enum)) {
      perror("Cannot read message type.");
      exit(1);
    } else if (rcvMessage_type != CASH) {
      printf("transact recieved wrong response type\n");
      exit(1);
    }

    float rcvCash;
    if ((amt = read(sockfd, &rcvCash, sizeof(float))) != sizeof(float)) {
      perror("Cannot read message type.");
      exit(1);
    }

    clientCash += rcvCash;
  }

  // send TRANSACTION to the server
  msg_type = TRANSACT;
  if ((amt =write(sockfd, &msg_type, sizeof(msg_enum))) != sizeof(msg_enum)) {
      perror("Cannot write");
      exit(1);
  }
  if ((amt =write(sockfd, &account_number, sizeof(int))) != sizeof(int)) {
      perror("Cannot write");
      exit(1);
  }
  if ((amt =write(sockfd, &amount, sizeof(float))) != sizeof(float)) {
      perror("Cannot write");
      exit(1);
  }

  // add the value of the transaction to the cash variable
  clientCash += amount;

  // balance is returned in diagram in the document.
  if ((amt = read(sockfd, &rcvMessage_type, sizeof(msg_enum))) != sizeof(msg_enum)) {
    perror("Cannot read message type.");
    exit(1);
  }
  if ((amt = read(sockfd, &rcvAccount_number, sizeof(int))) != sizeof(msg_enum)) {
    perror("Cannot read account number.");
    exit(1);
  }
  if ((amt = read(sockfd, &rcvBalance, sizeof(float))) != sizeof(float)) {
    perror("Cannot read balance.");
    exit(1);
  }
}

void get_account_info (int sock_fd, int acc_num) {
  int amt = 0;

  msg_enum msg_type = GET_ACCOUNT_INFO;

  msg_enum rsp_type;
  char name[MAX_STR];
  char username[MAX_STR];
  time_t birthday;

  // write message type
  if ((amt=write(sock_fd, &msg_type, sizeof(msg_enum))) != sizeof(msg_enum)) {
    printf("get_account_info failed to write msg_type\n.");
    printf("It wrote %d bytes\n.", amt);
    exit(1);
  }
  // write account number
  if ((amt=write(sock_fd, &acc_num, sizeof(int))) != sizeof(int)) {
    printf("get_account_info failed to write msg_type\n.");
    printf("It wrote %d bytes\n.", amt);
    exit(1);
  }

  // read message type
  if ((amt = read(sock_fd, &rsp_type, sizeof(msg_enum))) != sizeof(msg_enum)) {
    printf("get_account_info failed to read rsp_type\n.");
    printf("It read %d bytes\n.", amt);
    exit(1);
  }
  // make sure message type is account info
  else if (rsp_type != ACCOUNT_INFO) {
    printf("get_account_info recieved the wrong rsp_type\n");
    exit(1);
  }
  // read username
  if ((amt = read(sock_fd, &username, sizeof(char)*MAX_STR)) < 1) {
    printf("get_account_info failed to read username\n.");
    printf("It read %d bytes\n.", amt);
    exit(1);
  }
  // read name
  if ((amt = read(sock_fd, &name, sizeof(char)*MAX_STR)) < 1) {
    printf("get_account_info failed to read name\n.");
    printf("It read %d bytes\n.", amt);
    exit(1);
  }
  // read birthday
  if ((amt = read(sock_fd, &birthday, sizeof(time_t))) != sizeof(time_t)) {
    printf("get_account_info failed to read birthday\n.");
    printf("It read %d bytes.\n", amt);
    exit(1);
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


    // Variable declaration
    int message_type, account_number, num_transactions;
    char *line = (char*)malloc(sizeof(char)*MAX_STR);
    char* name = (char*)malloc(sizeof(char)*MAX_STR);
    char* username = (char*)malloc(sizeof(char)*MAX_STR);
    int nameLen, usernameLen;
    long birthday;
    float amount;

    // Necessary for reading in variables
    char *token, *saveptr;
    char *pattern = ",";


    FILE *fp = fopen(file, "r");
    if (fp == NULL) {
      fprintf(stderr, "ERROR: failed to open file %s\n", file);
      exit(EXIT_FAILURE);
    }

    while (fscanf(fp, "%d,%d,%s\n", &message_type, &account_number, line) != EOF) {
			sscanf(line, "%64[^,],%64[^,],%ld,%f,%d\n", name, username, &birthday, &amount, &num_transactions);

      func(sockfd, message_type, account_number, nameLen, name, usernameLen, username, birthday, amount, num_transactions);
    }

    end = clock();
    cpu_time = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Elapsed Time: %.2f\n", cpu_time);

    return 0;
}
