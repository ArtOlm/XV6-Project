#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/param.h"

int main(int numticks){
	int fd;
	char* errormsg = "sleep: not a proper argument\n";
	//not a valid arg
 	if(numticks <= 0)
	{	fd = write(1,errormsg,sizeof(errormsg));
		
		exit(1);
	}
	//use the sleep system call
	sleep(numticks);
	exit(0);
	return fd;
}
