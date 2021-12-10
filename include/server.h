#ifndef SERVER_H
#define SERVER_H

#include "utils.h"

void printSyntax();

// array of structs to store account information
extern struct account* balances[MAX_ACC];

// how many active accounts in balances
extern int numAccounts;

// shared queue of nodes containing data packets 
extern struct Queue* q;

// sempahores 
extern sem_t mutexBalances[MAX_ACC];         // Balance is being modified    
extern sem_t mutexQueue;			         // Queue is being modified
extern sem_t staged;			             // A producer signals when a package was staged in the queue

/**
 * Sets up an account and adds it to the global array balances.
 * @param username      
 * @param name
 * @param birthday
 */
void registerAccount(char* username, char* name, time_t birthday);

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

