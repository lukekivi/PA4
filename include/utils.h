#ifndef UTILS_H
#define UTILS_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>

#define MAX_STR 64
#define MAX_ACC 1023
#define CASH_AMOUNT 100000 		// cash amount for REQUEST_CASH and CASH messages
#define START_CASH 10000        // starting cash value for client.c
#define LOGGER_SLEEP 5          // time in between logging for the log thread (I reduced this since 30 seconds was too long to wait to log)

/*  DO NOT MODIFY */
typedef enum
{
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

#endif

