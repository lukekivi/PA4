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

void printEnumName(msg_enum msg) {
    switch (msg) {
        case REGISTER:         printf("REGISTER\n"); return;
        case GET_ACCOUNT_INFO: printf("GET_ACCOUNT_INFO\n"); return;
        case TRANSACT:         printf("TRANSACT\n"); return;
        case GET_BALANCE:      printf("GET_BALANCE\n"); return;
        case ACCOUNT_INFO:     printf("ACCOUNT_INFO\n"); return;
        case BALANCE:          printf("BALANCE\n"); return;
        case REQUEST_CASH:     printf("REQUEST_CASH\n"); return;
        case CASH:             printf("CASH\n"); return;
        case ERROR:            printf("ERROR\n"); return;
        case REQUEST_HISTORY:  printf("REQUEST_HISTORY\n"); return;
        case HISTORY:          printf("HISTORY\n");return;
    }
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