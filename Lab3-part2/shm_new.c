//Collaborated with Onyinyechukwu Ogbunaya

#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

enum AccountOrTurn {BANK_ACCOUNT = 0, TURN =  1};

void ClientProcess(int[]);

int main(int argc, char * argv[]) {
  int ShmID;
  int *ShmPTR;
  pid_t pid;
  int status;

  time_t t;
  srandom((unsigned) time( & t));

  ShmID = shmget(IPC_PRIVATE, 2 * sizeof(int), IPC_CREAT | 0666);
  if (ShmID < 0) {
    printf("*** shmget error (server) ***\n");
    exit(1);
  }

  ShmPTR = (int * ) shmat(ShmID, NULL, 0);
  if ( * ShmPTR == -1) {
    printf("*** shmat error (server) ***\n");
    exit(1);
  }
  

  ShmPTR[BANK_ACCOUNT] = 0;
  ShmPTR[TURN] = 0;

  pid = fork();
  if (pid < 0) {
    exit(1);
  } else if (pid == 0) {
    ClientProcess(ShmPTR);
    exit(0);
  }
  
  // parent process continues
  int account;
  int balance;
  int i;

  for (i = 0; i < 25; ++i) {

    sleep(random() % 6);

    account = ShmPTR[BANK_ACCOUNT];
    while (ShmPTR[TURN] != 0) {}

    if (account <= 100) {
    
      balance = random() % 101;

      if (balance % 2 == 0) {
        account += balance;
        printf("Dear old Dad: Deposits $%d / Balance = $%d\n", balance, account);
      } else {
        printf("Dear old Dad: Doesn't have any money to give\n");
      }
    } else {
      printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
    }
    
    ShmPTR[BANK_ACCOUNT] = account;
    ShmPTR[TURN] = 1;
  }

  wait( & status);
  printf("Server has detected the completion of its child...\n");
  shmdt((void * ) ShmPTR);
  printf("Server has detached its shared memory...\n");
  shmctl(ShmID, IPC_RMID, NULL);
  printf("Server has removed its shared memory...\n");
  printf("Server exits...\n");
  exit(0);
}

void ClientProcess(int SharedMem[]) {
  int account;
  int balance;
  int i;

  for (i = 0; i < 25; ++i) {
    sleep(random() % 6);
    
    account = SharedMem[BANK_ACCOUNT];
    while (SharedMem[TURN] != 1) {}
    
    balance = random() % 51;
    
    printf("Poor Student needs $%d\n", balance);
    
    if (balance <= account) {
      account -= balance;
      printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance, account);
    } else {
      printf("Poor Student: Not Enough Cash ($%d)\n", account );
    }
    
    SharedMem[BANK_ACCOUNT] = account;
    SharedMem[TURN] = 0;
  }
}