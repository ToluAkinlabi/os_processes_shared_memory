#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>

/**
 * Executes the command "cat scores | grep Lakers".  In this quick-and-dirty
 * implementation the parent doesn't wait for the child to finish and
 * so the command prompt may reappear before the child terminates.
 *
 */

int main(int argc, char **argv) 
{ 
  int pid, status, i;
  if (argc < 2) {
    exit(0);
  }
  // arguments for commands; parser would be responsible
  // for setting up arrays like these
  char *cat_args[] = {"cat", "scores", NULL};
  char *grep_args[] = {"grep", argv[1], NULL};
  char *sort_args[] = {"sort", NULL};

  // make 2 pipes (cat => grep | grep => sort)
  // each has 2 fds
  int pipefd[4];
  pipe(pipefd); // sets up the 1st pipe
  pipe(pipefd + 2); // sets up 2nd pipe

  // we now have 4 fds:
  // pipefd[0] = read end of cat => grep pipe (read by grep)
  // pipefd[1] = write end of cat => grep pipe (write by cat)
  // pipefd[2] = read end of grep => sort pipe (read by sort)
  // pipefd[3] = write end of grep => sort pipe (write by grep)

  // fork the 1st child (to execute cat)
  pid = fork();

  if (pid == 0)
    {
      // child gets here and handles "cat scores"

      // replace cat's stdout with write end of 1st pipe

      dup2(pipefd[1], 1);

      // close all pipes (very important!)

      close(pipefd[0]);
      close(pipefd[1]);
      close(pipefd[2]);
      close(pipefd[3]);      

      // execute cat
      execvp(*cat_args, cat_args);
    }
  else 
  {
    // fork 2nd child (to execute grep)
    pid = fork();

    if (pid == 0) 
    {
      // child gets here and handles "grep Lakers"

      // replace grep's stdin with read end of 1st pipe

      dup2(pipefd[0], 0);

      // replace grep's stdout with write end of 2nd pipe

      dup2(pipefd[3], 1);

      // close all ends of pipes 

      close(pipefd[0]);
      close(pipefd[1]);
      close(pipefd[2]);
      close(pipefd[3]);      

      // execute grep
      // execvp(*grep_args, grep_args);
      execvp(*grep_args, grep_args);
    }
    else
    {
      // fork third child (to execute sort)
      pid = fork();

      if (pid == 0)
      {
        // replace sort's stdin with input read of 2nd pipe
        dup2(pipefd[2], 0);

        // close all ends of pipes 

        close(pipefd[0]);
        close(pipefd[1]);
        close(pipefd[2]);
        close(pipefd[3]);

        // execute sort
        execvp(*sort_args, sort_args);
      }
    }
  }

  // only parent gets here and waits for 3 children to finish

  close(pipefd[0]);
  close(pipefd[1]);
  close(pipefd[2]);
  close(pipefd[3]);

  for (i = 0; i < 3; i++) {
    wait(&status);
  }
}
