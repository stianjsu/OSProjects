#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>
#include "alarmClock.h"
#define MAXALARMS 10

struct tm;

typedef struct _Alarm {
  int isActive;
  pid_t pid;
  time_t timestamp;

} Alarm;

Alarm alarms[MAXALARMS];

Alarm initAlarmStruct() {
  Alarm newAlarm = {  .isActive = 0,
                      .pid = 0,
                      .timestamp = 0};
  return newAlarm;
}

void setAlarmUnactiveAtPid(pid_t pid) {
  for (int i = 0; i < 10; i++){
    if(alarms[i].pid == pid){
      alarms[i].isActive = 0;
    }
  }
}

void menu() {
  time_t timeNow = time(NULL);
  printf("Welcome to the alarm clock! It is currently %s \n", ctime(&timeNow));
  printf("Please enter \"s\" (schedule), \"l\" (list), \"c\" (cancel), \"x\" (exit) \n>");

  char inputMenu;

  do {

    scanf("%c", &inputMenu);

    if (inputMenu == 's') {
      schedule();
      printf("> ");
    }
    else if (inputMenu == 'l') {
      list();
      printf("> ");
    }
    else if (inputMenu == 'c') {
      cancelAlarm();
      printf("> ");
    }
    else if (inputMenu != '\n') {
      putchar(inputMenu);
      printf(" is not a valid command\n> ");
    }

    pid_t pid = waitpid(-1, 0, WNOHANG);
    if (pid > 0) {
      setAlarmUnactiveAtPid(pid);
    }

  } while (inputMenu != 'x');
  printf("Goodbye!\n");

}


void schedule() {
  int freeSpace = -1;
  for (int i = 0; i < MAXALARMS; i++)
  {
    if (!alarms[i].isActive) {
      freeSpace = i;
      break;
    }
  }
  if (freeSpace < 0) {
    printf("No more space for alarms. Please cancel an alarm\n");
    return;
  }

  char inputTime[20]; 
  time_t now = time(NULL);
  time_t alarmTime = 0;
  struct tm alarmTimeStruct;
  alarmTimeStruct.tm_isdst=0;

  printf("Schedule alarm at which date and time?\n");
  printf("Format -> 'YYYY-MM-DD_hh:mm:ss'. Type 'c' to cancel\n> ");
  while (difftime (now, alarmTime) > 0) {
    if (alarmTime) {
      printf("Alarm has to be scheduled in the future, please try again\n> ");
    }

    scanf("%19s", &inputTime);

    if(inputTime[0] == 'c') {
      printf("Exiting\n");
      return;
    }

    strptime(inputTime, "%Y-%m-%d_%H:%M:%S", &alarmTimeStruct);
    alarmTime = mktime(&alarmTimeStruct);
    
  };
  
  now = time(NULL);
  pid_t pid = fork();

  if (!pid) {//child
    
    sleep(alarmTime-now);
    printf("Ringeling tingting\n");
    //None of us has a proper unix system installed except for wsl
    //Hard to test wether the alarm sound will work
    if(CURR_OS == 1) {
      char *file = "afplay";
      char *program = "afplay";
      char *arg1 = "./ringRingWnohang.mp3";
      execlp(file, program, arg1, NULL);
      
    } else if (CURR_OS == 2) {
      char *file = "mpg123";
      char *program = "mpg123";
      char *arg1 = "./ringRingWnohang.mp3";
      execlp(file, program, arg1, NULL);
    }
    printf("No no unix sound mediaplayer detected");
    
    exit(0);
    
  } else {//parent

    alarms[freeSpace].isActive = 1;
    alarms[freeSpace].pid = pid;
    alarms[freeSpace].timestamp = alarmTime;
    printf("Successfully created an alarm at %s \n", ctime(&alarmTime));
    
    printf("Will RingeDing in %f seconds\n", difftime(alarmTime, now));
  }
}

void list() {
  for (int i = 0; i < MAXALARMS; i++){
    if(alarms[i].isActive){
      time_t alTime = alarms[i].timestamp;
      printf("Alarm %d at %s", i, ctime(&alTime));
    }
  }
}

void cancelAlarm() {
  char inp;
  int alarmNr = -1;
  
  
  while(alarmNr < 0 || alarmNr > 9){ 
    if(inp != '\n'){
      printf("Cancel which alarm?\n> ");
    }
    scanf("%c", &inp);
    alarmNr = inp - '0';
  };
  
  if(alarms[alarmNr].isActive){
    alarms[alarmNr].isActive = 0;
    kill(alarms[alarmNr].pid, 0);
  } else {
    printf("There is no alarm nr.%i\n", alarmNr);
  }

}

int main() {
  for (int i = 0; i < MAXALARMS; i++) {
    alarms[i] = initAlarmStruct();
  }

  menu();
  
  //Cancel unfinished alarms before exiting
  int numChild = 0;
  for (int i = 0; i < MAXALARMS; i++) {
    if (alarms[i].isActive){
      kill(alarms[i].pid, SIGKILL);
      numChild++;
    }
  }
  printf("Killed %d children\n", numChild);
  sleep(3);
  numChild = 0;
  while(waitpid(-1, 0, WNOHANG) > 0 ){
    numChild++;
  }
  printf("Burried %d Zombiez\n", numChild);


  return 0;
}