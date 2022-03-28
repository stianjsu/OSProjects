#include <stdio.h>
#include <stdlib.h>
#include "bbuffer.h"
#include "sem.h"
#include <pthread.h>

typedef struct BNDBUF {
  struct SEM *empty;
  struct SEM *full;
  int readPos;
  int writePos;
  int size;
  int bufferArr[];
} BNDBUF;

pthread_mutex_t getMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t addMutex = PTHREAD_MUTEX_INITIALIZER;

BNDBUF *bb_init(unsigned int size) {
  struct BNDBUF* buffer = (BNDBUF*) malloc(sizeof(BNDBUF)+ size * sizeof(int));
  buffer->size = size;
  buffer->readPos = 0;
  buffer->writePos = 0;
  buffer->empty = sem_init(0);
  buffer->full = sem_init(size);
  return buffer;
}



int bb_get(BNDBUF *bb) {
  
  P(bb->empty); //wait if empty
  pthread_mutex_lock( &getMutex );
    int value = bb->bufferArr[bb->readPos % bb->size];
    bb->bufferArr[bb->readPos % bb->size] = 0;
    bb->readPos++;
  pthread_mutex_unlock( &getMutex );
  V(bb->full);//buffer is no longer full
  
  return value;
}


void bb_add(BNDBUF *bb, int fd) {
  P(bb->full); //wait if full
  pthread_mutex_lock( &addMutex );
    bb->bufferArr[bb->writePos % bb->size] = fd;
    bb->writePos++;
  pthread_mutex_unlock( &addMutex );
  V(bb->empty); //buffer is no longer empty 

}

void bb_del(BNDBUF* bb) {
  sem_del(bb->empty);
  sem_del(bb->full);
  free(bb);
}
