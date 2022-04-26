#include "kernel/types.h"
#include "user/user.h"
#include "kernel/flags.h"
#include "kernel/spinlock.h"
/*
  this test is used to test multiple producers and only a single consumer

*/
#define BSIZE 10
#define MAX 20
int status = 0;

typedef struct {
    int buf[BSIZE];
    int nextin;
    int nextout;
    int num_produced;
    int num_consumed;
    int total;
    sem_t occupied;
    sem_t free;
    sem_t lock;
} buffer_t;

buffer_t *buffer;

void producer(int arg)
{
  while(1) {
    sem_wait(&buffer->free);
    sem_wait(&buffer->lock);
    if (buffer->num_produced >= MAX) {
        sem_post(&buffer->free);
        sem_post(&buffer->occupied);
        sem_post(&buffer->lock);
        exit(0);
    }
    
    buffer->num_produced++;
    printf("producer %d producing %d\n", arg, buffer->num_produced);
    buffer->buf[buffer->nextin++] = buffer->num_produced;

    buffer->nextin %= BSIZE;
    sem_post(&buffer->occupied);

    /* Now either buffer->occupied < BSIZE and buffer->nextin is the index
       of the next empty slot in the buffer, or
       buffer->occupied == BSIZE and buffer->nextin is the index of the
       next (occupied) slot that will be emptied by a consumer
       (such as buffer->nextin == buffer->nextout) */
  
    sem_post(&buffer->lock);
  }
}

void consumer(int arg)
{ 
  while(1) {
    sem_wait(&buffer->occupied);
    sem_wait(&buffer->lock);
    if (buffer->num_consumed >= MAX) {
        sem_post(&buffer->occupied);
        sem_post(&buffer->free);
        sem_post(&buffer->lock);
        exit(0);
   }
    printf("consumer %d consuming %d\n", arg, buffer->buf[buffer->nextout]);
    buffer->total += buffer->buf[buffer->nextout++];
    buffer->nextout %= BSIZE;
    buffer->num_consumed++;
    sem_post(&buffer->free);

    /* now: either b->occupied > 0 and b->nextout is the index
       of the next occupied slot in the buffer, or
       b->occupied == 0 and b->nextout is the index of the next
       (empty) slot that will be filled by a producer (such as
       b->nextout == b->nextin) */

    sem_post(&buffer->lock);

  }
}

int main(int argc, char *argv[])
{
    long long int i;
  buffer = (buffer_t *)mmap(0,sizeof(buffer_t *),PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1,0);
  buffer->nextin = 0;
  buffer->nextout = 0;
  buffer->num_produced = 0;
  buffer->num_consumed = 0;
  buffer->total = 0;
  printf("sizeof(buffer->lock): %d\n", sizeof(buffer->lock));
  sem_init(&buffer->occupied, 0);
  sem_init(&buffer->free, BSIZE);
  sem_init(&buffer->lock, 1); 
  //created the child producers
  for (i = 0; i < 5; i++){
    int rt = fork();
    if(rt == 0){
      producer(i);
      exit(0);
    }
  }
  //creates the child consumers
  for (i = 0; i < 1; i++){
    int rt = fork();
    if(rt == 0){
      consumer(i);
      exit(0);
    }
  } 
  //baisically does the join for a thread
  while (wait(&status) > 0){}
  printf("total consumed = %d\n", buffer->total);
 
  exit(0);
}
