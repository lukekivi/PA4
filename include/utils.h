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

#define MAX_STR 64
#define MAX_ACC 1023
#define CASH_AMOUNT 100000
#define START_CASH 10000
#define LOGGER_SLEEP 5
#define NCLIENTS 8
#define MSG_BUFFER_SIZE 4
#define MSG_ENUM_SIZE 12

// number of worker threads
int nWorkers;

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

typedef struct {
	char* username;
	char* name;
	time_t birthday;
	int account_number;
	float balance;
}account_info;

/**
 *
 */
void printEnumName(msg_enum msg);

/**
 * 
 */
msg_enum selectResponse(msg_enum recv);

/**
 * @brief Check if a string is composed strictly of digits
 * @param str a string
 * @return -1 if a non-digit character was found, 1 if str is composed of digits
 */
int isDigits(char* str);

#endif

