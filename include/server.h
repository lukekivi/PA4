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
 * @return account number for success, -1 for failure
 */
 int handleRegister(int sockfd);

/**
 * Write response to REGISTER back to client
 * @param sockfd
 * @param accountNumber
 * @return 1 for success, 0 for failure
 */
 int respondRegister(int sockfd, int accountNumber);

/**
 * Update account with transaction data
 * @param accountNumber index of the account
 * @param transaction   the value change in the account total
 * @return new balance on success, -1 on failure
 */
 int addTransaction(int accountNumber, float transaction);

 /**
  * Get the transactions from an account.
  * @param accountNumber    index of the account
  * @param numTransactions how many transactions you want, 0 for all
  * @param arr             array to fill 
  * @return how many transactions were added to arr, -1 for failure
  */
  int getTransactions(int accountNumber, int numTransactions, float** arr);

/**
 * Read in how much cash the client wants and send it back.
 * @param sockfd
 * @return 1 for success, -1 for error
 */
 int cashRequest(int sockfd);

 /**
  * Read in account number and send back balance.
  * @param sockfd
  * @return 1 for success, 0 for non existant account, -1 for error   
  */
  int getBalance(int sockfd);

  /**
   * Sends balance response back to client
   * @param sockf
   * @param accNum account number
   * @param balance
   * @return 1 for succes, 0 for error
   */
   int respondBalance(int sockfd, int accNum, float balance);

 /**
   * Read in transaction infromation and then respond to it.
   * @param sockfd
   * @return 1 for success, 0 for non existant account, -1 for error   
   */
  int transact(int sockfd);

   /**
   * Read in account number and then respond with account details.
   * @param sockfd
   * @return 1 for success, 0 for non existant account, -1 for error   
   */
  int getAccountInfo(int sockfd);

  /**
   * Read in account number, and numTransactions then respond with the transactions.
   * @param sockfd
   * @return 1 for success, 0 for non existant account, -1 for error
   */
  int getHistory(int sockfd);

  /**
   * Check if the account exists already
   * @param  accountNumber
   * @return 0 for no, 1 for yes
   */
   int validAccount(int accountNumber);

  /**
   * Respond with an error proceeded by the causal message. If the proceeding message is also ERROR that means
   * that the client requested an account number that doesn't exist.
   * @param sockfd
   * @param msg
   * @return 1 for success, -1 for error
   */
   int sendError(int sockfd, msg_enum msg);


  /**
   * Initialize balances to all NULLS
   */ 
  void initBalances();

  /**
   * Free balances array
   */
  void freeBalances();

#endif

