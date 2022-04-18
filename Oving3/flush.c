#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_SINGLE_ARG_LENGTH 32
#define MAX_ARG_COUNT 16
#define MAX_INP_LENGTH 512
#define MAX_NUM_BG 16

char backgroundArgsSize[MAX_NUM_BG][MAX_SINGLE_ARG_LENGTH];
int bgPID[MAX_NUM_BG];
char **bgArgs = backgroundArgsSize;


void printExitStatus(char **args, int exitStatus) {
    int i;
    printf("Exited with status [");
    for (i = 0; args[i] != 0; i++) {
        printf("%s ", args[i]);
    }
    printf("]: %d\n", exitStatus);
}

int cd(char **args) {
// if input is cd then change directory
  if (strcmp(args[0], "cd") == 0) {
    if (chdir(args[1]) != 0) {
      perror("chdir() error");
      return 1;
    }
  }
  return 0;
}

void collectZombies() {
  int status;
  int pid = waitpid (-1, &status, WNOHANG);
  while (pid > 0) {
    for (int i = 0; i < MAX_NUM_BG; i++) {
      if (bgPID[i] == pid) {
        printf("Collected zombie with args [%s] and exit status %d\n", bgArgs[i], WEXITSTATUS(status));
        bgPID[i] = 0;
        bgArgs[i] = NULL;
        break;
      }
    }
    pid = waitpid (-1, &status, WNOHANG);
  }
}

int jobs(char **args) {
  collectZombies(); //first collects zombies in order to show the proper list of background jobs
  //for each element in bgPID, print out the PID and the element at the same index in bgArgs if not NULL
  int i;
  int foundCommand = 0;
  for(i = 0; i < MAX_NUM_BG; i++) {
    
    if(bgPID[i] != 0) {
      if(!foundCommand) {
        printf("All background processes:\n");
        printf("PID\t\tCommand\n");
        foundCommand = 1;
      }
      printf("%d\t\t%s\n", bgPID[i], bgArgs[i]);
    }
  }
  if(!foundCommand) {
    printf("No background processes\n");
  }
}

//return 1 if args contains > or <, 0 otherwise
int checkIfIoRedir(char **args) {
  for(int i = 0; args[i] != NULL; i++) {
    if(strcmp(args[i], "<") == 0 || strcmp(args[i], ">") == 0) {
      return 1;
    }
  }
  return 0;
}

// exec args with io redirection "<" ">"
int execWithIoRedir(char **args) {

  // args without "<" ">"
  char argsSize[MAX_ARG_COUNT][MAX_SINGLE_ARG_LENGTH];
  char **argsClean = argsSize;
  int argsCleanIndex = 0;
  int infd, outfd;

  for (int j = 0; args[j] != NULL; j++) {
    if (!strcmp(args[j], "<")) {
      ++j;
      if ((infd = open(args[j], O_RDONLY)) < 0) {
        fprintf(stderr, "error opening file\n");
      }
      dup2(infd, STDIN_FILENO);
      close(infd);
      continue;
    }
    else if (!strcmp(args[j], ">")) {
      ++j;
      outfd = creat(args[j], 0644);
      dup2(outfd, STDOUT_FILENO);
      close(outfd);
      continue;
    } 
    //if args[j] is not &, add to argsClean
    else if(strcmp(args[j], "&") != 0) {
      argsClean[argsCleanIndex++] = args[j];
    }
  }                                  

  argsClean[argsCleanIndex] = NULL;

  execvp(argsClean[0], argsClean);
  fprintf(stderr, "error in child exec \n");
  
}

// returns 1 if last arg is '&' and removes it from args
int isBackgroundJob(char **args) {
  int i = 0;
  while(args[i] != NULL && i+1 < MAX_ARG_COUNT) {
    if(strcmp(args[i], "&") == 0 && args[i+1] == NULL){
      args[i] = NULL; // remove & from args
      return 1;
    }
    i++;
  }
  return 0;
}


int forkAndExec(char **args) {
  int bgJob = isBackgroundJob(args);
  pid_t pid = fork();

  if (pid == 0) {
    if(checkIfIoRedir(args)) {
      execWithIoRedir(args);
      exit(EXIT_FAILURE);  //execWithIoRedir should exit on its own, if not exit with failiure
    } else { 
      execvp(args[0], args);
      printf("Could not find command: %s\n", args[0]);
      exit(EXIT_FAILURE);  //execvp should exit on its own, if not exit with failiure
    }
  } else if (pid < 0) {
    perror("fork() error");
    return 1;
  } else {
    
    if(bgJob == 0) {
      int status;
      waitpid ( pid, &status, WUNTRACED);
      printExitStatus(args, WEXITSTATUS(status));
    } else {
      //find first element of bgPid that is NULL and set it to pid
      for (int i = 0; i < MAX_NUM_BG; i++) {
        //printf("%d ", bgPID[i]);
        if (bgPID[i] == 0) {
          bgPID[i] = pid;
          //copy args 0 into bgArgs[i]
          char *arg;
          strcpy(arg, args[0]);
          
          for (int j = 1; j < MAX_ARG_COUNT; j++) {
            if (args[j] == NULL) {
              break;
            }
            strcat(arg, " ");
            strcat(arg, args[j]);
          } 
          bgArgs[i] = arg;
          break;
        }
      }
    }
  }
  return 0;
}



int main() {
  //set all elements in bgPID to NULL
  printf("\033[1;31m Welcome to FLUSH ;)\n");
  for (int i = 0; i < MAX_NUM_BG; i++) {
    bgPID[i] = 0;
  }
  
  while (1) {

    // print the current path
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
      printf("\033[1;36m%s: \033[0m", cwd);
    } else {
      perror("getcwd() error");
      return 1;
    }

    // get input from user
    char input[MAX_INP_LENGTH]; 
    input[0] = 0;
    fgets(input, MAX_INP_LENGTH, stdin);

    // If user inputs ctrl + d: break
    if(input[0] == 0) {
      printf("\n");
      collectZombies();
      exit(EXIT_SUCCESS);
    } 
    // If userinput is empty, continue
    if (input[0] == '\n') {
      continue;
    }

    input[strcspn(input, "\n")] = 0;

    // get space-separated arguments from input
    char argsSize[MAX_ARG_COUNT][MAX_SINGLE_ARG_LENGTH];
    char **args = argsSize;
    for(int i = 0; i < MAX_ARG_COUNT; i++)
      args[i] = 0;

    int i = 0;
    char *token = strtok(input, "\t ");
    while (token != 0) {
      args[i] = token;
      i++;
      token = strtok(0, "\t ");
    }

   
    int exitStat = 0;
    if(args[0] != 0) {
      if (strcmp(args[0], "exit") == 0) {
        exitStat = 0;
      } else if (strcmp(args[0], "cd") == 0) {
        exitStat = cd(args);
      } else if (strcmp(args[0], "jobs") == 0) {
        exitStat = jobs(args);
      } else {
        exitStat = forkAndExec(args);
      }

      if(exitStat == 1) {
        perror("Command error");
      }
    }

    if (args[0] == 0 || strcmp(args[0], "exit") == 0) {
      collectZombies();
      exit(EXIT_SUCCESS);
    }
    //collects any potential zombies before next input prompt
    collectZombies();
  }
  
  collectZombies(); // for insurance
}