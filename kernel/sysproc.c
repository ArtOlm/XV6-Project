#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "flags.h"
uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
uint64
sys_strace(void){
     int sarg;
     argint(0,&sarg);
     myproc()->stracemask = sarg;
     return 0;

}
uint64
sys_mmap(void){
		
	struct proc *p = myproc(); 
	int add;
	int size;
	int prot;
	int flags;
	int f;
	int offset;
	//get the parameters
	argint(0,&add);
	argint(1,&size);
	argint(2,&prot);
	argint(3,&flags);
	argint(4,&f);
	argint(5,&offset);
	if(add != 0){
		printf("Error: address must be null");
	       exit(-1);	
	}	
	//set the size of the regon to be a multiple of PGSIZE
	size = size - (size % 4096) + 4096;
	size = PGROUNDUP(size);
	//actual address
	uint64 act_add = p->cur_max - size;
	//search fir valid mmr and set the members for the structure
	for(int i = 0;i < MAX_MMR;i++){
		if(!p->mmr[i].valid){
			//set up the region only
			p->mmr[i].start_addr = act_add;
			p->mmr[i].length = size;
			p->mmr[i].prot = prot;
			p->mmr[i].flags = flags;
			p->mmr[i].valid = 1;
			p->mmr[i].file = 0;
			p->mmr[i].fd = -1;
			for(int j = 0;j < MAX_PROC;j++){
				p->mmr[i].sharedproc[j] = -1;
			}
			p->mmr[i].sharedproc[0] = p->pid;
			break;
		}
	}
	//update cur size so that the next page starts at the next available address
	p->cur_max = p->cur_max - size;
	return act_add;
}
uint64
sys_munmap(void){
	struct proc *p = myproc();
	//int tl;
	uint64 address;
	argaddr(0,&address);
	uint64 start = address;
	//uint64 length;
	//argint(1,&tl);
	//length = (uint64)tl;
	int removed = 0;
	for(int i = 0;i < MAX_MMR;i++){
		//find valid region
		if(p->mmr[i].valid){
			//last address
			uint64 last_address = p->mmr[i].start_addr + p->mmr[i].length;
			//check if the address passed equals to any of the regions
			if(p->mmr[i].start_addr == address){
				p->mmr[i].valid = 0;
				//count how many processes in the region
				int count = 0;
      	for(int j = 0;j < MAX_PROC;j++){
          if(p->mmr[i].sharedproc[j] != -1){
            count++;
          }
      	}
      //remove this p->pid from every other processes shared region
			for(int k = 0;k < MAX_PROC;k++){
				if(p->mmr[i].sharedproc[k] != -1){
          struct proc *mp;
		      for(mp = proc; mp < &proc[NPROC]; mp++){
              if(mp->pid == p->mmr[i].sharedproc[k]){
                for(int q = 0;q < MAX_PROC;q++){
                    if(p->pid == mp->mmr[i].sharedproc[q]){
                        mp->mmr[i].sharedproc[q] = -1;
                    }
                }
              }
            }
				}
			}
        removed = 1;
				while(start < last_address){
					//get the physical address, if it is not mapped then walkaddr returns 0
					uint64 mapped_addr = walkaddr(p->pagetable,start);
      		//only remove physical when the count is 1
					if((mapped_addr > 0) && (count == 1)){
						//uvmunmap only unmaps mapped regions
						uvmunmap(p->pagetable,start,1,1);
					}else if(mapped_addr > 0){
						uvmunmap(p->pagetable,start,1,0);
					}
					//go to the next page
					start += PGSIZE;
				}			
	  	}	
		}
	}
	if(!removed){
		return -1;	
	}
	return 0;
}
uint64
sys_sem_init(void){
	  //get the arguments
		uint64 sem_addr;
		int count;
		argaddr(0,&sem_addr);
		argint(1,&count);
		//boolean vairable to check if the allocation was a success
		//return 0 id successful
    int sem_alloc = -1;
		sem_t * ssem = (sem_t *)sem_addr;
		acquire(&tblock);
		for(int i = 0;i < NSEM;i++){
				if(!sem[i].valid){
					sem[i].valid = 1;
					//give the semaphore the index of the semaphore struct if there is one
					*ssem = i;
					sem[i].count = count;
					initlock(&sem[i].lock,"sem");
					sem_alloc = 0;
					break;
				}
		}
		release(&tblock);
		return sem_alloc;
}
uint64
sys_sem_post(void){
		//get the address of the semaphore
		uint64 sem_addr;
		argaddr(0,&sem_addr);
		sem_t * ssem = (sem_t *)sem_addr;
		//check the semaphore has valid contents
		if((*ssem < 0) || (*ssem >= NSEM)){
		return -1;
		}
		//check it is valid in the table
		if(sem[*ssem].valid){
				acquire(&sem[*ssem].lock);
				//increment count of the semaphore
				sem[*ssem].count++;
				//wake up the waiting process
				wakeup(abc[*ssem]);
				release(&sem[*ssem].lock);
		}

		return 0;
}

uint64
sys_sem_wait(void){

		uint64 sem_addr;
		argaddr(0,&sem_addr);
		sem_t * ssem = (sem_t *)sem_addr;
		if((*ssem < 0) || (*ssem >= NSEM)){
		return -1;
		}
		if(sem[*ssem].valid){
				acquire(&sem[*ssem].lock);
				//put the process to sleep while count of semaphore is 0
				//give up the lock so that if sem_post is called there won't be a deadlock
				while(sem[*ssem].count == 0){
					
					sleep(abc[*ssem],&sem[*ssem].lock);
				}
				sem[*ssem].count--;
				release(&sem[*ssem].lock);
		}
		return 0;
}

uint64
sys_sem_destroy(void){
	uint64 sem_addr;
	argaddr(0,&sem_addr);
	sem_t * ssem = (sem_t *)sem_addr;
	//check semaphore is within range
	if((*ssem < 0) || (*ssem >= NSEM)){
		return -1;
	}

	acquire(&tblock);
	if(sem[*ssem].valid){
		//make the semapthore struct not valid anymore and sem_t value passed now has a negative value
		sem[*ssem].valid = 0;
		*ssem = -1;
	}
	release(&tblock);

	return 0;

}