#include "kernel/types.h"
#include "user/user.h"
#include "kernel/flags.h"
//this is the original test tht can run in xv6
#define BUF_SIZE 10
void producer();
int consumer();
int *buffer;

void producer() {
    for (int i=0; i<BUF_SIZE; i++){  
        buffer[i] = i;
    }
    return;
}

int consumer() {
    int sum = 0;
    for (int i=0; i<BUF_SIZE; i++){
	 sum += buffer[i];   
    }
    return sum;
}

int main() {
	
    buffer = (int *) mmap(0, BUF_SIZE*sizeof(int), PROT_READ | PROT_WRITE, 
                          MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    if (!buffer) {
        printf("Error: mmap() failed\n");
        exit(-1);
    }
    int rc = fork();
    if (rc == 0) {
	     producer();
    }
    else if (rc>0) {
        wait(0);
        printf("main consumed sum = %d\n",consumer());
    } else {
        printf("Error: fork() failed\n");
        exit(-1);
    }
    exit(0);
}
    


