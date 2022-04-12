#include "kernel/types.h"
#include "user/user.h"
#include "kernel/flags.h"
//this test involves multiple faults
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
    producer();
    printf("Total consumed = %d\n",consumer());
    exit(0);
}
    


