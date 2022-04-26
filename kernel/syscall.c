#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "syscall.h"
#include "defs.h"

// Fetch the uint64 at addr from the current process.
int
fetchaddr(uint64 addr, uint64 *ip)
{
  struct proc *p = myproc();
  if(addr >= p->sz || addr+sizeof(uint64) > p->sz)
    return -1;
  if(copyin(p->pagetable, (char *)ip, addr, sizeof(*ip)) != 0)
    return -1;
  return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Returns length of string, not including nul, or -1 for error.
int
fetchstr(uint64 addr, char *buf, int max)
{
  struct proc *p = myproc();
  int err = copyinstr(p->pagetable, buf, addr, max);
  if(err < 0)
    return err;
  return strlen(buf);
}

static uint64
argraw(int n)
{
  struct proc *p = myproc();
  switch (n) {
  case 0:
    return p->trapframe->a0;
  case 1:
    return p->trapframe->a1;
  case 2:
    return p->trapframe->a2;
  case 3:
    return p->trapframe->a3;
  case 4:
    return p->trapframe->a4;
  case 5:
    return p->trapframe->a5;
  }
  panic("argraw");
  return -1;
}

// Fetch the nth 32-bit system call argument.
int
argint(int n, int *ip)
{
  *ip = argraw(n);
  return 0;
}

// Retrieve an argument as a pointer.
// Doesn't check for legality, since
// copyin/copyout will do that.
int
argaddr(int n, uint64 *ip)
{
  *ip = argraw(n);
  return 0;
}

// Fetch the nth word-sized system call argument as a null-terminated string.
// Copies into buf, at most max.
// Returns string length if OK (including nul), -1 if error.
int
argstr(int n, char *buf, int max)
{
  uint64 addr;
  if(argaddr(n, &addr) < 0)
    return -1;
  return fetchstr(addr, buf, max);
}

extern uint64 sys_chdir(void);
extern uint64 sys_close(void);
extern uint64 sys_dup(void);
extern uint64 sys_exec(void);
extern uint64 sys_exit(void);
extern uint64 sys_fork(void);
extern uint64 sys_fstat(void);
extern uint64 sys_getpid(void);
extern uint64 sys_kill(void);
extern uint64 sys_link(void);
extern uint64 sys_mkdir(void);
extern uint64 sys_mknod(void);
extern uint64 sys_open(void);
extern uint64 sys_pipe(void);
extern uint64 sys_read(void);
extern uint64 sys_sbrk(void);
extern uint64 sys_sleep(void);
extern uint64 sys_unlink(void);
extern uint64 sys_wait(void);
extern uint64 sys_write(void);
extern uint64 sys_uptime(void);
extern uint64 sys_strace(void);
extern uint64 sys_mmap(void);
extern uint64 sys_munmap(void);
extern uint64 sys_sem_init(void);
extern uint64 sys_sem_post(void);
extern uint64 sys_sem_wait(void);
extern uint64 sys_sem_destroy(void);

static uint64 (*syscalls[])(void) = {
[SYS_fork]    sys_fork,
[SYS_exit]    sys_exit,
[SYS_wait]    sys_wait,
[SYS_pipe]    sys_pipe,
[SYS_read]    sys_read,
[SYS_kill]    sys_kill,
[SYS_exec]    sys_exec,
[SYS_fstat]   sys_fstat,
[SYS_chdir]   sys_chdir,
[SYS_dup]     sys_dup,
[SYS_getpid]  sys_getpid,
[SYS_sbrk]    sys_sbrk,
[SYS_sleep]   sys_sleep,
[SYS_uptime]  sys_uptime,
[SYS_open]    sys_open,
[SYS_write]   sys_write,
[SYS_mknod]   sys_mknod,
[SYS_unlink]  sys_unlink,
[SYS_link]    sys_link,
[SYS_mkdir]   sys_mkdir,
[SYS_close]   sys_close,
[SYS_strace]  sys_strace,
[SYS_mmap]    sys_mmap,
[SYS_munmap]  sys_munmap,
[SYS_sem_init]  sys_sem_init,
[SYS_sem_post]  sys_sem_post,
[SYS_sem_wait]  sys_sem_wait,
[SYS_sem_destroy]  sys_sem_destroy
};

void
syscall(void)
{
  int num;
  struct proc *p = myproc();
  
  


  num = p->trapframe->a7;
  if(num > 0 && num < NELEM(syscalls) && syscalls[num]){
    //save regiters a0-a5,(argument regiters)
    //this way we can access after the system call
    //i did not inlcude a3-a5 bcause no system call has more than three arguments
    //this is raw value inly
    int ta0 = argraw(0);
    int ta1 = argraw(1);
    int ta2 = argraw(2);
    //two strings which get the arguments a0 and a1
    char sa0[50];
    char sa1[50];
    argstr(0,sa0,50);
    argstr(1,sa1,50);
    //

    
	  
    p->trapframe->a0 = syscalls[num]();
    //mask passed by the user
     uint pmask = p->stracemask;
     //mask generatd by num
     uint nmask = 1 << (num);
     
     if(((pmask & nmask) == nmask)){//see if the process mask and num mask create num mask again, meaning it is a trace we want
	//we call argrw(0) again since the convention in riscv is to put the return in register 10 or a0
	//i did not use an array of names, instead I just did this if else statements to check the number and based on that it handles the name
	//it also handles the argument, since arguments change based on the call I could not make is as dynamic as I wanted
	if(num ==1){
		printf("%d: syscall fork(void) -> %d\n",p->pid,argraw(0));}
	else if(num == 2){
		printf("%d: syscall exit(%d) -> %d\n",p->pid,ta0,argraw(0));}
	else if(num == 3){
		printf("%d: syscall wait(%d) -> %d\n",p->pid,ta0,argraw(0));}
	else if(num == 4){
		printf("%d: syscall pipe(%d) -> %d\n",p->pid,ta0,argraw(0));}
	else if(num == 5){
		printf("%d: syscall read(%d,%d,%d) -> %d\n",p->pid,ta0,ta1,ta2,argraw(0));}
	else if(num == 6){
		printf("%d: syscall kill(%d) -> %d\n",p->pid,ta0,argraw(0));}
	else if(num == 7){
		printf("%d: syscall exec(%s,%d) -> %d\n",p->pid,sa0,ta1,argraw(0));}
	else if(num == 8){
		printf("%d: syscall fstat(%d,%d) -> %d\n",p->pid,ta0,ta1,argraw(0));}
	else if(num == 9){
		printf("%d: syscall chdir(%s) -> %d\n",p->pid,sa0,argraw(0));}
	else if(num == 10){
		printf("%d: syscall dup(%d) -> %d\n",p->pid,ta0,argraw(0));}
	else if(num == 11){
		printf("%d: syscall getpid(void) -> %d\n",p->pid,argraw(0));}
	else if(num == 12){
		printf("%d: syscall sbrk(%d) -> %d\n",p->pid,ta0,argraw(0));}
	else if(num == 13){
		printf("%d: syscall sleep(%d) -> %d\n",p->pid,ta0,argraw(0));}
	else if(num == 14){
		printf("%d: syscall uptime(void) -> %d\n",p->pid,argraw(0));}
	else if(num == 15){
		printf("%d: syscall open(%s,%d) -> %d\n",p->pid,sa0,ta1,argraw(0));}
	else if(num == 16){
		printf("%d: syscall write(%d,%d,%d) -> %d\n",p->pid,ta0,ta1,ta2,argraw(0));}
	else if(num == 17){
		printf("%d: syscall mknod(%s,%d,%d) -> %d\n",p->pid,sa0,ta1,ta2,argraw(0));}
	else if(num == 18){
		printf("%d: syscall link(%s,%s) -> %d\n",p->pid,sa0,sa1,argraw(0));}
	else if(num == 19){
		printf("%d: syscall unlink(%s) -> %d\n",p->pid,sa0,argraw(0));}
	else if(num == 20){
		printf("%d: syscall mkdir(%s) -> %d\n",p->pid,sa0,argraw(0));}
	else if(num == 21){
		printf("%d: syscall close(%d) -> %d\n",p->pid,ta0,argraw(0));}
	else if(num == 22){
		printf("%d: syscall strace(%d) -> %d\n",p->pid,ta0,argraw(0));}
	else if(num == 23){
		printf("%d: syscall mmap() -> %d\n",p->pid,argraw(0));
	}
     
     }
  } else {
    printf("%d %s: unknown sys call %d\n",
            p->pid, p->name, num);
    p->trapframe->a0 = -1;
  }
}
