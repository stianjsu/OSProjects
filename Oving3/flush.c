#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SINGLE_ARG_LENGTH 32
#define MAX_ARG_COUNT 16
#define MAX_INP_LENGTH 512
#define MAX_NUM_BG 16

char backgroundArgsSize[MAX_NUM_BG][MAX_SINGLE_ARG_LENGTH];
int bgPID[MAX_NUM_BG];
char **bgArgs = backgroundArgsSize;




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

int ls(char **args) {
  // if input is ls then list directory
  printf("arg[0]: %s, arg[1]: %s\n", args[0], args[1]);
  if(args[1] == NULL || strcmp(args[1], "''")) {
    args[1] = "./";
  }
  return forkAndExec(args);
}

// list all files in directory, if no directory is specified, list current directory


int jobs(char **args) {
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
/* 
//redirect input from arg before > to arg after >
int redirect(char **args) {
  int i;
  for(i = 0; args[i] != NULL; i++) {
    if(strcmp(args[i], ">") == 0) {
      int fd = open(args[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if(fd < 0) {
        perror("open() error");
        return 1;
      }
      if(dup2(fd, STDOUT_FILENO) < 0) {
        perror("dup2() error");
        return 1;
      }
      args[i] = NULL;
      args[i+1] = NULL;
      return 0;
    }
  }
  return 0;
}

// pipe input from arg before | to arg after |
int pipe(char **args) {
  int i;
  for(i = 0; args[i] != NULL; i++) {
    if(strcmp(args[i], "|") == 0) {
      int fd[2];
      if(pipe(fd) < 0) {
        perror("pipe() error");
        return 1;
      }
      int pid = fork();
      if(pid < 0) {
        perror("fork() error");
        return 1;
      }
      if(pid == 0) {
        if(dup2(fd[1], STDOUT_FILENO) < 0) {
          perror("dup2() error");
          return 1;
        }
        args[i] = NULL;
        args[i+1] = NULL;
        return execvp(args[0], args);
      }
      else {
        if(dup2(fd[0], STDIN_FILENO) < 0) {
          perror("dup2() error");
          return 1;
        }
        args[i] = NULL;
        args[i+1] = NULL;
        return execvp(args[i+1], args);
      }
    }
  }
  return 0;
} */

// returns 1 if last arg is '&'
int isBackgroundJob(char **args) {
  int i = 0;
  while(args[i] != NULL && i+1 < MAX_ARG_COUNT) {
    if(strcmp(args[i], "&") == 0 && args[i+1] == NULL){
      return 1;
    }
    i++;
  }
  return 0;
}

int forkAndExec(char **args) {
  
  //fork and exec
  pid_t pid = fork();
  if (pid == 0) {
    execvp(args[0], args);
    printf("Could not find command: %s\n", args[0]);
    exit(EXIT_FAILURE);  //execvp should exit on its own, if not exit with failiure
  } else if (pid < 0) {
    perror("fork() error");
    return 1;
  } else {
    
    if(!isBackgroundJob(args)) {
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
    




int main(/* int argc, char *argv[] */) {
  //set all elements in bgPID to NULL
  for (int i = 0; i < MAX_NUM_BG; i++) {
    bgPID[i] = 0;
  }
  
  while (1) {

    // print the current path
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
      printf("%s : ", cwd);
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

    input[strcspn(input, "\n")] = 0;

    if(strstr(input, "<") != NULL || strstr(input, ">") != NULL) {
      //do special parseing for redirection
      
    }


    // get space-separated arguments from input
    char argsSize[MAX_ARG_COUNT][MAX_SINGLE_ARG_LENGTH];
    char **args = argsSize;
    for(int i = 0; i < MAX_ARG_COUNT; i++)
      args[i] = NULL;

    int i = 0;
    char *token = strtok(input, " ");
    while (token != NULL) {
      args[i] = token;
      i++;
      token = strtok(NULL, " ");
    }

    // print the arguments
    for (int j = 0; j < i; j++) {
      printf("Arg %d: |%s| \n", j, args[j]);
    }

    char term = 0x04;
    char *ter = &term;
    int f = 0;
    //terminate when user enters 0x04
    //terminate when user enters control + D
    //actually terminate when user enters "exit"
    if(args[0] != NULL) {
      if (strcmp(args[0], "exit") == 0) {
        f = 0;
      } else if (strcmp(args[0], "cd") == 0) {
        f = cd(args);
      } else if (strcmp(args[0], "ls") == 0) {
        f = ls(args);
      } else if (strcmp(args[0], "jobs") == 0) {
        f = jobs(args);
      } else {
        f = forkAndExec(args);
      }
      if(f == 1) {
        perror("Command error");
      }
    }

    //collects any potential zombies before next input prompt
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

    if (strcmp(args[0], "exit") == 0) {
      exit(EXIT_SUCCESS);
    }
  }
  
  return EXIT_SUCCESS;

}