#include "utils.h"

void _removeOutputDir() {
    pid_t pid = fork();
    if(pid == 0) {
        char *argv[] = {"rm", "-rf", "output", NULL};
        if (execvp(*argv, argv) < 0) {
            printf("ERROR: exec failed\n");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    } else {
        wait(NULL);
    }
}

void _createOutputDir() {
    mkdir("output", ACCESSPERMS);
}

void bookeepingCode() {
    _removeOutputDir();
    sleep(1);
    _createOutputDir();
}

// writes a string length and then the string itself to the socket fd
int writeStringToSocket(int sockfd, char* str) {
    if (write(sockfd, str, MAX_STR) <= 0) {
        perror("ERROR: failed to write stringto sockfd\n");
        return -1;
    }
    return 1;
}


// reads string from the socket and returns it or null for an error val
char* readStringFromSocket(int sockfd) {
    char* str = (char*) malloc(sizeof(char) * MAX_STR);

    if(read(sockfd, str, MAX_STR) <= 0) {
        perror("ERROR: failed to read string from socket\n");
        free(str);
        return NULL;        
    }

    return str;
}

// writes enum to socket, returns 1 for success, -1 for failure
int writeEnum(int sockfd , msg_enum msg) {
    if (write(sockfd, &msg, sizeof(msg_enum)) != sizeof(msg_enum)) {
        perror("ERROR: failed to write msg to socket\n");
        return -1;
    }
    return 1;
}

// reads enum from socket, returns msg or -1 for failure
msg_enum readEnum(int sockfd) {
    msg_enum msg;
    if (read(sockfd, &msg, sizeof(msg_enum)) != sizeof(msg_enum)) {
        perror("ERROR: failed to write msg to socket\n");
        return -1;
    }

    return msg;
}

/* Return a pointer to an initialized queue */
struct Queue* initQueue() {
    struct Queue* q = (struct Queue*) malloc(sizeof(struct Queue));
    q->head = (struct Node*) malloc(sizeof(struct Node));
    q->head->next = NULL;

    q->tail = NULL;

    return q;
}

/* initialize a node */ 
struct Node* initNode(int sockfd) {
    // printf("Got here 3.1\n");
    struct Node* node = (struct Node*) malloc(sizeof(struct Node));
    // printf("Got here 3.2\n");
    node->next = NULL;
    node->sockfd = sockfd;
    // printf("Got here 3.3\n");
    return node;
}

/* Add node to the bottom of a queue */
void enqueue(struct Queue* q, struct Node* node) {

    if (q->tail == NULL) {
        // case where q is empty
        q->tail = node;
        q->head->next = node;
    } else {
        // case where q has been started
        struct Node *temp = q->tail;
        temp->next = node;
        q->tail = node;
    }
}

/* Pop node from queue - just returns the sockfd */
int dequeue(struct Queue* q) {
    if (q->head->next == NULL) {
        // q is empty
        return -1;
    }

    struct Node *temp = q->head->next;
    if (q->head->next == q->tail) {
        // last element removed
        q->tail = NULL;
    }

    q->head->next = q->head->next->next;
    temp->next = NULL;

    int sockfd = temp->sockfd;
    freeNode(temp);

    return sockfd;
}

/* Deallocate a node */
void freeNode(struct Node* node) {
    if (node == NULL) {
        return;
    } else {
        node->next = NULL;        
        free(node);
    }
}

/* free entire queue */
void freeQueue(struct Queue* q) {
    struct Node* curNode = q->head;
    while(curNode != NULL) {
        struct Node* temp = curNode->next;
        freeNode(curNode);
        curNode = temp;
    }
    q->head = NULL;
    q->tail = NULL;

    free(q);
}

void printEnumName(msg_enum msg) {
    switch (msg) {
        case REGISTER:         printf("REGISTER : %d\n", REGISTER); return;
        case GET_ACCOUNT_INFO: printf("GET_ACCOUNT_INFO : %d\n", GET_ACCOUNT_INFO); return;
        case TRANSACT:         printf("TRANSACT : %d\n", TRANSACT); return;
        case GET_BALANCE:      printf("GET_BALANCE : %d\n", GET_BALANCE); return;
        case ACCOUNT_INFO:     printf("ACCOUNT_INFO : %d\n", ACCOUNT_INFO); return;
        case BALANCE:          printf("BALANCE : %d\n", BALANCE); return;
        case REQUEST_CASH:     printf("REQUEST_CASH : %d\n", REQUEST_CASH); return;
        case CASH:             printf("CASH : %d\n", CASH); return;
        case ERROR:            printf("ERROR : %d\n", ERROR); return;
        case REQUEST_HISTORY:  printf("REQUEST_HISTORY : %d\n", REQUEST_HISTORY); return;
        case TERMINATE:        printf("TERMINATE : %d\n", TERMINATE); return;
        case HISTORY:          printf("HISTORY : %d\n", HISTORY);return;
    }
}