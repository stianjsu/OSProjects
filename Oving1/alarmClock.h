#ifdef __APPLE__
#define CURR_OS 1
#endif
#ifdef __linux__
#define CURR_OS 2
#endif
#ifndef CURR_OS
#define CURR_OS 0
#endif


#ifndef LARMCLOCK
#define LARMCLOCK

#include <unistd.h>
#include <sys/types.h>
#include <time.h>

void menu();

void schedule();

void list();

void cancelAlarm();
//fix rest of methods



#endif