#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(){
//pipes which are used to read and write a char to
int ppipe[2];
int cpipe[2];
pipe(ppipe);
pipe(cpipe);
//for creates a child process
int val = fork();
if(val < 0){//error child did not create
	fprintf(2,"pingpong: fork error");
	exit(1);	
}
else if(val == 0){
	//close unused pipeends
	close(cpipe[1]);
	close(ppipe[0]);
	char ret[1];
	//reads a char form the parent
	read(cpipe[0],ret,sizeof(ret));

	fprintf(1,"%d: received ping\n",getpid());
	//cose read end of cpipe
	close(cpipe[0]);
	//write to parent program
	write(ppipe[1],ret,sizeof(ret));
	//cloe the remaining ends of pipes left
	close(ppipe[1]);
	exit(0);
	
}
else{	//close unused pipe ends
	close(cpipe[0]);
	close(ppipe[1]);
	//write a char to the child using cpipe
	write(cpipe[1],"p",sizeof(char));
	close(cpipe[1]);

	char ret[1];
	//read a char form the child
	read(ppipe[0],ret,sizeof(ret));
	fprintf(1,"%d: received pong\n ",getpid());
	close(cpipe[0]);
	exit(0);

}
exit(0);
}
