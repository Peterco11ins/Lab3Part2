// Lab 3 (Part 2): Processes and Shared Memory
// Parent = "Dear Old Dad"   (deposits)
// Child  = "Poor Student"   (withdraws)
// Uses shared memory with two ints: BankAccount and Turn
// Turn: 0 = Dad's turn, 1 = Student's turn

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <time.h>

int main() {
    int   ShmID;
    int  *ShmPTR;
    pid_t pid;

    // create shared memory for 2 integers
    ShmID = shmget(IPC_PRIVATE, 2 * sizeof(int), IPC_CREAT | 0666);
    if (ShmID < 0) {
        perror("shmget");
        exit(1);
    }

    ShmPTR = (int *) shmat(ShmID, NULL, 0);
    if ((void *) ShmPTR == (void *) -1) {
        perror("shmat");
        exit(1);
    }

    int *BankAccount = &ShmPTR[0];
    int *Turn        = &ShmPTR[1];

    *BankAccount = 0;     // starting balance
    *Turn        = 0;     // Dad goes first

    srand(time(NULL));

    pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    /* ===================== PARENT: Dear Old Dad ===================== */
    if (pid > 0) {
        int i;
        for (i = 0; i < 25; i++) {
            sleep(rand() % 6);          // 0–5 seconds

            int account = *BankAccount;

            // strict alternation: wait while it's not Dad's turn
            while (*Turn != 0) {
                ; // no-op
            }

            if (account <= 100) {
                int balance = rand() % 101;   // 0–100

                if (balance % 2 == 0) {
                    account += balance;
                    printf("Dear old Dad: Deposits $%d / Balance = $%d\n",
                           balance, account);
                } else {
                    printf("Dear old Dad: Doesn't have any money to give\n");
                }

                *BankAccount = account;
            } else {
                printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n",
                       account);
                *BankAccount = account;
            }

            *Turn = 1;   // now it's Student's turn
        }

        // wait for child, then clean up shared memory
        wait(NULL);

        if (shmdt((void *) ShmPTR) == -1) {
            perror("shmdt");
        }
        if (shmctl(ShmID, IPC_RMID, NULL) == -1) {
            perror("shmctl");
        }

        return 0;
    }

    /* ===================== CHILD: Poor Student ===================== */
    else {
        // different seed than parent
        srand(time(NULL) ^ getpid());

        int i;
        for (i = 0; i < 25; i++) {
            sleep(rand() % 6);          // 0–5 seconds

            int account = *BankAccount;

            // strict alternation: wait while it's not Student's turn
            while (*Turn != 1) {
                ; // no-op
            }

            int balance = rand() % 51;  // 0–50
            printf("Poor Student needs $%d\n", balance);

            if (balance <= account) {
                account -= balance;
                printf("Poor Student: Withdraws $%d / Balance = $%d\n",
                       balance, account);
            } else {
                printf("Poor Student: Not Enough Cash ($%d)\n", account);
            }

            *BankAccount = account;
            *Turn        = 0;           // now it's Dad's turn
        }

        if (shmdt((void *) ShmPTR) == -1) {
            perror("shmdt");
        }
        return 0;
    }
}