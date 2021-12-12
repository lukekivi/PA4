#include "../include/client.h"

int clientCash = START_CASH;

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

// FUNCTION: FUNCREGISTER
// Create a new account

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

// FUNCTION: GET_BALANCE
// Get the balance of a specific account
float get_balance (int sockfd, int account_number) {
  int msg_type = htonl(account_number);
  int amt = 0;

  if ((amt =write(sockfd, &msg_type, sizeof(int))) != sizeof(int)) {
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

  while (1) {
    if ((amt = read(sockfd, &rcvMessage_type, sizeof(msg_enum))) != sizeof(msg_enum)) {
      perror("Cannot read message type.");
      exit(1);
    } else {
      break;
    }
  }
  while (1) {
    if ((amt = read(sockfd, &rcvAccount_number, sizeof(int))) != sizeof(int)) {
      perror("Cannot read account number.");
      exit(1);
    } else {
      rcvAccount_number = ntohl(rcvAccount_number);
      if (rcvAccount_number != account_number) {
        printf("get_balance recieved the wrong account number.\n");
        exit(1);
      }
      break;
    }
  }
  while (1) {
    if ((amt = read(sockfd, &rcvBalance, sizeof(float))) != sizeof(float)) {
      perror("Cannot read balance.");
      exit(1);
    } else {
      break;
    }
  }
  return rcvBalance;
}

// FUNCTION: REQUEST_CASH
// Request that the recipient is sent cash
void request_cash (int sockfd, float amount) {
  int amt = 0;
  float request = amount;
  int msg_type = htonl(REQUEST_CASH);

  if ((amt = write(sockfd, &msg_type, sizeof(int))) != sizeof(int)) {
    perror("Cannot write");
    exit(1);
  }
  if ((amt = write(sockfd, &request, sizeof(float))) != sizeof(float)) {
    perror("Cannot write");
    exit(1);
  }

  int rcvMessage_type;
  float rcvCash;
  while (1) {
    if ((amt = read(sockfd, &rcvMessage_type, sizeof(msg_enum))) != sizeof(msg_enum)) {
      perror("Cannot read account number.");
      exit(1);
    } else {
      int translatedMessage = ntohl(rcvMessage_type);
      if (translatedMessage != CASH) {
        printf("Request cash recieved wrong response type.\n");
        return;
      }
      break;
    }
  }
  while (1) {
    if ((amt = read(sockfd, &rcvCash, sizeof(float))) != sizeof(float)) {
      perror("Cannot read balance.");
      exit(1);
    } else {
      break;
    }
  }
  clientCash += rcvCash;
}

// FUNCTION: TRANSACT
// Send a transaction for an account
void transact (int sockfd, int account_number, float amount) {
  // send a GET_BALANCE message to the server to ensure
  // that the account will not go negative
  float rcvBalance = get_balance(sockfd, account_number);

  if ((rcvBalance - amount) < 0) {
    return; // if account can go negative, ignore transact
  }

  // send GET_CASH to server until the cash variable will not go negative.
  while (clientCash - amount < 0) {
    request_cash(sockfd, amount);
  }

  int amt = 0;
  int rcvAccount_number;
  msg_enum msg_type, rcvMessage_type;

  // send TRANSACTION to the server
  msg_type = htonl(TRANSACT);
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
  while (1) {
    if ((amt = read(sockfd, &rcvMessage_type, sizeof(msg_enum))) != sizeof(msg_enum)) {
      perror("Cannot read message type.");
      exit(1);
    } else {
      int temp = ntohl(rcvMessage_type);
      rcvMessage_type = temp;
      break;
    }
  }
  while (1) {
    if ((amt = read(sockfd, &rcvAccount_number, sizeof(int))) != sizeof(msg_enum)) {
      perror("Cannot read account number.");
      exit(1);
    } else {
      int tempAccount = ntohl(rcvAccount_number);
      rcvAccount_number = tempAccount;
      break;
    }
  }
  while (1) {
    if ((amt = read(sockfd, &rcvBalance, sizeof(float))) != sizeof(float)) {
      perror("Cannot read balance.");
      exit(1);
    } else {
      break;
    }
  }
}

// FUNCTION: GET_ACCOUNT_INFO
// request the information for a specific account
void get_account_info (int sock_fd, int acc_num) {
  int amt = 0;

  msg_enum msg_type = htonl(GET_ACCOUNT_INFO);

  msg_enum rsp_type;
  char* name = (char*)malloc(sizeof(char)*MAX_STR);
  char* username = (char*)malloc(sizeof(char)*MAX_STR);
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

  int rsp;
  // read message type
  while (1) {
    if ((amt = read(sock_fd, &rsp_type, sizeof(msg_enum))) != sizeof(msg_enum)) {
      printf("get_account_info failed to read rsp_type\n.");
      printf("It read %d bytes\n.", amt);
      exit(1);
    }
    // make sure message type is account info
    else if (rsp = ntohl(rsp_type) != ACCOUNT_INFO) {
      printf("get_account_info recieved the wrong rsp_type\n");
      exit(1);
    }
  }
  // read username
  while (1) {
    if ((username = readStringFromSocket(sock_fd)) == NULL) {
      printf("get_account_info failed to read username\n.");
      exit(1);
    } else {
      break;
    }
  }
  // read name
  while (1) {
    if ((name = readStringFromSocket(sock_fd)) == NULL) {
      printf("get_account_info failed to read name\n.");
      exit(1);
    } else {
      break;
    }
  }
  // read birthday
  while (1) {
    if ((amt = read(sock_fd, &birthday, sizeof(time_t))) != sizeof(time_t)) {
      printf("get_account_info failed to read birthday\n.");
      printf("It read %d bytes.\n", amt);
      exit(1);
    } else {
      break;
    }
  }
}

// FUNCTION: ERROR
// Generic error message to be sent when the enumerated message type does not match with any in the protocol
void error (int sock_fd, int message_type) {
  int msg_type = htonl(ERROR);
  int wrong_msg_type = htonl(message_type); // are we supposed to send this as well? it is one of the data values for error.
  int amt = 0;

  if ((amt=write(sock_fd, &msg_type, sizeof(int))) != sizeof(int)) {
    printf("error failed to write msg_type\n.");
    printf("It wrote %d bytes\n.", amt);
    exit(1);
  }
  if ((amt=write(sock_fd, &wrong_msg_type, sizeof(int))) != sizeof(int)) {
    printf("error failed to write wrong_msg_type\n.");
    printf("It wrote %d bytes\n.", amt);
    exit(1);
  }
}

// FUNCTION: TERMINATE
// Signal to the recipient that no further messages are to be expected
void terminate (int sock_fd) {
  int msg = htonl(TERMINATE);
  int amt = 0;

  // write message type
  if ((amt=write(sock_fd, &msg, sizeof(int))) != sizeof(int)) {
    printf("terminate failed to write msg_type\n.");
    printf("It wrote %d bytes\n.", amt);
    exit(1);
  }
}

// FUNCTION: REQUEST_HISTORY
// Request history of account
// Not finished yet, doesn't know what to recieve back yet
void request_history (int sock_fd, int account_num) {
  int msg_type = htonl(REQUEST_HISTORY);
  int msg_account = htonl(account_num);
  int amt = 0;

  if ((amt=write(sock_fd, &msg_type, sizeof(int))) != sizeof(int)) {
    printf("request_history failed to write msg_type\n.");
    printf("It wrote %d bytes\n.", amt);
    exit(1);
  }
  if ((amt=write(sock_fd, &msg_account, sizeof(int))) != sizeof(int)) {
    printf("request_history failed to write account_num\n.");
    printf("It wrote %d bytes\n.", amt);
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
      if (message_type < 0 || message_type > 11) {
        message_type = ERROR; // if message_type isn't between 0 and 11, it is invalid, so it becomes an error.
      }
    }


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
