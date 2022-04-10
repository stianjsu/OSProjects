#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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


void printArgs(char **args) {
    int i;
    printf("Args: ");
    for (i = 0; args[i] != NULL; i++) {
        printf("%s ", args[i]);
    }
    printf("\n");
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

int jobs(char **args) {
  collectZombies(); //first collets zombies in order to show the proper list of background jobs
  //for each element in bgPID, print out the PID and the element at the same index in bgArgs if not NULL
  int i;
  printf("All background processes:\n");
  printf("PID\t\tCommand\n");
  for(i = 0; i < MAX_NUM_BG; i++) {
    if(bgPID[i] != 0) {
      printf("%d\t\t%s\n", bgPID[i], bgArgs[i]);
    }
  }
}

//return 1 if input contrains > or <, 0 otherwise
int checkIfIoRedir(char **args) {
  for(int i = 0; args[i] != NULL; i++) {
    if(strcmp(args[i], "<") == 0 || strcmp(args[i], ">") == 0) {
      return 1;
    }
  }
  return 0;
}




int execWithIoRedir(char **args) {
  // exec args with io redirection "<" ">"
  char argsSize[MAX_ARG_COUNT][MAX_SINGLE_ARG_LENGTH];
  char **args_clean = argsSize;
  int cleanidx = 0;
  int in, out;

  for (int j = 0; args[j] != NULL; j++) {
    if (!strcmp(args[j], "<")) {        // looking for input character
      ++j;
      if ((in = open(args[j], O_RDONLY)) < 0) {   // open file for reading
        fprintf(stderr, "error opening file\n");
      }
      dup2(in, STDIN_FILENO);         // duplicate stdin to input file
      close(in);                      // close after use
      continue;
    }                                   // end input chech

    else if (!strcmp(args[j], ">")) {        // looking for output character
      ++j;
      out = creat(args[j], 0644); // create new output file
      dup2(out, STDOUT_FILENO);       // redirect stdout to file
      close(out);                     // close after usere
      continue;
    }                                   // end output check

  
    //if args[j] is not &, add to args_clean
    else if(strcmp(args[j], "&") != 0) {
      args_clean[cleanidx++] = args[j];
    }
  }                                     // end loop

  args_clean[cleanidx] = NULL;

  execvp(args_clean[0], args_clean);                  // execute in parent
  fprintf(stderr, "error in child execi \n"); // error
  
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
      printf("Exit status [%s]: %d\n", args[0], WEXITSTATUS(status));
    }  else {
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

int pipeline(**args) {
  
  //check if args contain |
  int pipeIdx[MAX_ARG_COUNT];
  int pipeCount = 0;
  for (int i = 0; args[i] != NULL; i++) {
    if (strcmp(args[i], "|") == 0) {
      pipeIdx[pipeCount++] = i;
    }
  }
  

}

int main(/* int argc, char *argv[] */) {
  //set all elements in bgPID to NULL
  printf("\033[1;31m Welcome to FLUSH ;)\n");
  for (int i = 0; i < MAX_NUM_BG; i++) {
    bgPID[i] = 0;
  }
  
  while (1) {

    // print the current path
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
      //printf("%s : ", cwd);
      printf("\033[1;36m%s: \033[0m", cwd);
    } else {
      perror("getcwd() error");
      return 1;
    }

    // get input from user
    
    char input[MAX_INP_LENGTH]; 
    input[0] = NULL;
    fgets(input, MAX_INP_LENGTH, stdin);

    // If user inputs ctrl + d: break
    if(input[0] == NULL) {
      printf("\n");
      break;
    } 
    if (input[0] == '\n') {
      continue;
    }

    input[strcspn(input, "\n")] = 0;

    // get space-separated arguments from input
    char argsSize[MAX_ARG_COUNT][MAX_SINGLE_ARG_LENGTH];
    char **args = argsSize;
    for(int i = 0; i < MAX_ARG_COUNT; i++)
      args[i] = NULL;

    int i = 0;
    char *token = strtok(input, "\t ");
    while (token != NULL) {
      args[i] = token;
      i++;
      token = strtok(NULL, "\t ");
    }

    // print the arguments
    printArgs(args);

    char term = 0x04;
    char *ter = &term;
    int exitStat = 0;
    //terminate when user enters control + D
    if(args[0] != NULL) {
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


    if (args[0] == NULL || strcmp(args[0], "exit") == 0) {
      exit(EXIT_SUCCESS);
    }
    //collects any potential zombies before next input prompt
    collectZombies();
  }
  
  return EXIT_SUCCESS;

}