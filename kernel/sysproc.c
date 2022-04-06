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
	if(p->mmrsize == 0){
		p->cur_max = MAXVA - (2 * 4096);
	}	
	if(p->mmrsize == (MAX_MMR)){
		printf("Error: not more mmr allowed");
		return 0;
	}
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
	int success = 0;
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
			success = 1;
			break;
		}
	}//check if there was a mapped region
	if(!success){
		printf("Error: No more memory available");
		exit(-1);
	}
	//update the size of the mmrregion
	p->mmrsize++;
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
	//uint64 length;
	//argint(1,&tl);
	//length = (uint64)tl;
	int removed = 0;
	for(int i = 0;i < MAX_MMR;i++){
		//find valid region
		if(p->mmr[i].valid){
			p->mmr[i].valid = 0;
			//last address
			uint64 last_address = p->mmr[i].start_addr + p->mmr[i].length;
			//check if the address passed equals to any of the regions
			if(p->mmr[i].start_addr <= address){
                                removed = 1;
				while(address < last_address){
                        		//get the physical address, if it is not mapped then walkaddr returns 0
                        		uint64 mapped_addr = walkaddr(p->pagetable,address);
                        		if((mapped_addr > 0) && ((p->mmr[i].flags & MAP_SHARED) == MAP_SHARED) && p->hasp){
                                		//uvmunmap only unmaps mapped regions
                                		uvmunmap(p->pagetable,address,1,0);
                        		}
                        		else if(mapped_addr > 0){
                                		uvmunmap(p->pagetable,address,1,1);
                        			}
                        		//go to the next page
                        		address+= PGSIZE;
                		}
			}	
		}
	}
	if(!removed){
		return -1;	
	}
	return 0;
}
