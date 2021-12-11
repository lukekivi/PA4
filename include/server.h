#ifndef SERVER_H
#define SERVER_H

#include "utils.h"

void printSyntax();

// array of structs to store account information
extern struct account** balances;

// how many active accounts in balances
extern int numAccounts;

// shared queue of nodes containing data packets 
extern struct Queue* q;

// sempahores 
extern sem_t mutexBalances[MAX_ACC];        // Balance is being modified    
extern sem_t mutexQueue;			              // Queue is being modified
extern sem_t staged;			                  // A producer signals when a package was staged in the queue
extern sem_t numAccountsMutex;              // Mutex for accessing numAccounts

/**
 * Handle reading in after REGISTER is received
 * @param sockfd
 * @return 1 for success, -1 for error
 */
 int registerAccount(int sockfd);

/**
 * Update account with transaction data
 * @param accountNumber index of the account
 * @param transaction   the value change in the account total
 * @return 1 on success, 0 on unitialized, -1 on insufficient funds
 */
 int addTransaction(int accountNumber, float transaction);

 /**
  * Initialize balances to all NULLS
  */ 
  void initBalances();

/**
 * Free balances array
 */
 void freeBalances();

#endif

