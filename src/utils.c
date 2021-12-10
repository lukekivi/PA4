#include "utils.h"

void _removeOutputDir()
{
    pid_t pid = fork();
    if(pid == 0)
    {
        char *argv[] = {"rm", "-rf", "output", NULL};
        if (execvp(*argv, argv) < 0)
        {
            printf("ERROR: exec failed\n");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    } else{
        wait(NULL);
    }
}

void _createOutputDir()
{
    mkdir("output", ACCESSPERMS);
}

void bookeepingCode()
{
    _removeOutputDir();
    sleep(1);
    _createOutputDir();
}

msg_enum selectResponse(msg_enum recv) {
    switch (recv) {
        case REGISTER:         return ACCOUNT_INFO;
        case GET_ACCOUNT_INFO: return ACCOUNT_INFO;
        case TRANSACT:         return BALANCE;
        case GET_BALANCE:      return BALANCE;
        case REQUEST_CASH:     return CASH;
        case REQUEST_HISTORY:  return HISTORY;
        default: fprintf(stderr, "ERROR: Bad recv argument."); exit(0);
    }
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
    struct Node* node = (struct Node*) malloc(sizeof(struct Node));
    node->next = NULL;
    node->sockfd = sockfd;

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

/* Pop node from queue */
struct Node* dequeue(struct Queue* q) {
    if (q->head->next == NULL) {
        // q is empty
        return NULL;
    }

    struct Node *temp = q->head->next;
    if (q->head->next == q->tail) {
        // last element removed
        q->tail = NULL;
    }

    q->head->next = q->head->next->next;
    temp->next = NULL;

    return temp;
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


/* Check a string to see if it is strictly composed of digits */
int isDigits(char* str) {
    int len = strlen(str);

    for (int i = 0; i < len; i++) {
        if (str[i] < '0' || str[i] > '9') {
            return -1;
        }
    }

    return 1;
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