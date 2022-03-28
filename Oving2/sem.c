#include "sem.h"
#include <pthread.h>
#include <stdlib.h>

typedef struct SEM {
  int counter;
  pthread_mutex_t condition_mutex;
  pthread_cond_t  condition_cond;
} SEM;

SEM *sem_init(int initVal) {
  pthread_mutex_t condition_mutex_const = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t  condition_cond_const = PTHREAD_COND_INITIALIZER;
  struct SEM *semmy = (SEM*) malloc(sizeof(SEM));
  semmy->counter = initVal;
  semmy->condition_mutex = condition_mutex_const;
  semmy->condition_cond  = condition_cond_const;
  
  return semmy;

}

int sem_del(SEM *sem) {
  pthread_mutex_destroy(&(sem->condition_mutex));
  free(sem);
  return 0;
}

void P(SEM *sem) { //wait
  pthread_mutex_lock( &(sem->condition_mutex) );
    while(sem->counter == 0) {
      //blocked; wait for counter to be incremented
      pthread_cond_wait( &(sem->condition_cond), &(sem->condition_mutex) );
    }
    sem->counter--;
  pthread_mutex_unlock( &(sem->condition_mutex) );
}

void V(SEM *sem) { //signal
  pthread_mutex_lock( &(sem->condition_mutex) );
    sem->counter++;
    pthread_cond_signal( &(sem->condition_cond) );
  pthread_mutex_unlock( &(sem->condition_mutex) );
}