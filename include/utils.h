#ifndef UTILS_H
#define UTILS_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>


#define MAX_STR 64
#define MAX_ACC 1023
#define CASH_AMOUNT 100000
#define START_CASH 10000
#define LOGGER_SLEEP 5
#define NCLIENTS 8
#define MSG_ENUM_SIZE 12

/*  DO NOT MODIFY */
typedef enum {
	REGISTER,
	GET_ACCOUNT_INFO,
	TRANSACT,
	GET_BALANCE,
	ACCOUNT_INFO,
	BALANCE,
	REQUEST_CASH,
	CASH,
	ERROR,
	TERMINATE,
	/* extra credit messages */
	REQUEST_HISTORY,
	HISTORY,
}msg_enum;

void bookeepingCode();

// Struct to hold account information
struct account {
	char* username;
	char* name;
	time_t birthday;
	float balance;
    float* transactions;
	int numTransactions;
    int transactionsSize;
};

/**
 * Selects the response enum. For use in server. If the enum doesn't have a response it triggers 
 * an error and exits
 * @param recv received enum
 * @return     the correct response enum
 */
msg_enum selectResponse(msg_enum recv);

/**
 * Read a string size, allocate space for it, and then the string and return it.
 * @param sockfd the socket file descriptor to read from
 * @return a pointer to a character array, or NULL for an error value
 */
 char* readStringFromSocket(int sockfd);

/**** Queue Code ****/

/**
 * Node for shared queue linked list
 * @param next   pointer to the next node in the linked list
 * @param sockfd fd for socket
 */
struct Node {
    struct Node* next;
    int sockfd;
};

/** Queue Stuff **/

/**
 * Queue for nodes that carry packets
 * @param tail where nodes are dequeued
 * @param head dummy node. Head->next is what is dequeued.
 */
struct Queue {
    struct Node* head;
    struct Node* tail;
};

/**
 * Initialize a queue
 * @returns a pointer to an initialized queue
 */
struct Queue* initQueue();

/**
 * Allocate space for a node and set its fields
 * @param sockfd the socket fd
 * @returns      pointer to the malloc'd node
 */
 struct Node* initNode(int sockfd);

/**
 * Add node to a queue. If the queue is empty to start, tail should be a node with it's fields set to NULL.
 * @param q    the queue
 * @param node node to be added. node->next should be NULL
 */
void enqueue(struct Queue* q, struct Node* node);

/**
 * Pop the head node off of the queue
 * @param q    the queue
 * @returns    the sockfd from popped node, -1 if error
 */
int dequeue(struct Queue* q);

/**
 * Deallocate a node
 * @param node the node to free
 */
void freeNode(struct Node* node);

/**
 * Free entire queue
 * @param q    the queue
 */
void freeQueue(struct Queue* q);

/**** Debugging Functions ****/

void printEnumName(msg_enum msg);

#endif

