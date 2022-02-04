#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/param.h"


char* substring(char *str);
//method which returns pointer to a fromated argument
char* substring(char *str){
	int c = 0;
	char* f = str;
	while(*f != '\n' && *f != '\0' && *f != ' '){
		c++;
		f++;
	}	
	c++;
	char *substr = (char*)malloc(sizeof(char) * (c + 1));
	char *mas = substr;
	char *x = str;
	while(*x != '\n' && *x != '\0' && *x != ' '){
		*mas = *x;
		mas++;
		x++;

	}	
	return substr;
}


int main(int argc,char* argv[]){
	if(argc == 0){
		printf("xargs: error not sufficient arguments");
		exit(1);
	}
	
	argv++;
	if(argc >= (MAXARG - 1)){
		printf("xargs: argument size exceeded");
		exit(1);
	}
	char *rargs[MAXARG];
	//new array which has pointers to executable and to arguments
	rargs[0] = argv[0];
	int i;
	for(i = 1;i < argc - 1;i++){
		rargs[i] = argv[i];
	}	
         char ch;
	 //line holds argument by argument
	 char line[512];
	 char *ptr = line;
	//read line by line to distinush args
	while(read(0, &ch, 1) > 0)
	{	if(ch == '\n'){//every new line an arg
		   *ptr = ch;	
				      
		   if(fork() == 0){ //child execute the program with new arguments
			 if(i == (MAXARG - 1)){
				 rargs[i] = substring(line);
			 }
		         else if(i < (MAXARG - 1)){
				 rargs[i] = substring(line);
				 i++;
				 //ensure that args have the null pointer at th end of the last arg
				 rargs[i] = '\0';
			 }
			 else{
			 	printf("xargs: failed exec to many argumnets");	
				exit(1);
			 }	 
			 exec(rargs[0],rargs);
			
		   }		
		    wait(0);//wait for the child to finish before continuing
		    memset(line,0,sizeof(line));
		    ptr = line;
		}
		else if(ch == ' '){
			*ptr = ch;
			rargs[i] = substring(line);
			i++;
			memset(line,0,sizeof(line));
			ptr = line;
		}
		else{
		*ptr = ch;
		ptr++;
		}
 	}	
	exit(0);
	return 0;
}
