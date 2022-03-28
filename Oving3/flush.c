#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SINGLE_ARG_LENGTH 32
#define MAX_ARG_COUNT 16
#define MAX_INP_LENGTH 512


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


/*
 void stdoout(char *args) {
  // redirect stdout to file
  if (strcmp(args[0], ">") == 0) {
    if (execvp("cat", args) != 0) {
      perror("cat() error");
      return 1;
    }
  }
} 

int stdinp(char *args) {
  // get stdin from file
  if (strcmp(args[0], "<") == 0) {
    if (execvp("cat", args) != 0) {
      perror("cat() error");
      return 1;
    }
  }
  return 0;
}


int pipe(char *args) {
  // pipe
  if (strcmp(args[0], "|") == 0) {
    if (execvp("cat", args) != 0) {
      perror("cat() error");
      return 1;
    }
  }
  return 0;
}

int background(char *args) {
  // if user enters & then background process
  if (strcmp(args[0], "&") == 0) {
    if (execvp("cat", args) != 0) {
      perror("cat() error");
      return 1;
    }
  }
  return 0;
}

int printBackground(char *args) {
  // command jobs should print all background processes
  if (strcmp(args[0], "jobs") == 0) {
    if (execvp("jobs", args) != 0) {
      perror("jobs() error");
      return 1;
    }
  }
  return 0;
} */

int forkAndExec(char **args) {
    //fork and exec
    pid_t pid = fork();
    if (pid == 0) {
      execvp(args[0], args);
      printf("Could not find command: %s", args[0]);
      exit(EXIT_FAILURE);  //execvp should exit on its won, if this does not happend we exit with failiure
    } else if (pid < 0) {
      perror("fork() error");
      return 1;
    } else {
      //printf("pid child: %d\n", pid);
      int status;
      waitpid ( pid, &status, WUNTRACED);
      printf("Exit status [%s] = %d\n", args[0], WEXITSTATUS(status));
    } 
    return 0;
}

int main(/* int argc, char *argv[] */) {
  

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
    fgets(input, MAX_INP_LENGTH, stdin);
    input[strcspn(input, "\n")] = 0;


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
    if(args[0] != NULL){
      if (strcmp(args[0], "exit") == 0) {
        exit(EXIT_SUCCESS);
      } else if (strcmp(args[0], "cd") == 0) {
        f = cd(args);
      } else if (strcmp(args[0], "ls") == 0) {
        f = ls(args);
      } else {
        f = forkAndExec(args);
      }
      if(f == 1) {
        perror("Command error");
      }
    }


  }

}