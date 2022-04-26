#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char*
fmtname(char *path)
{ 
 
  char *p;
  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--);
  p++;
  return p;

  
}

void
f(char *path,char *ffind,int start)
{
   	
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;
  char* n = fmtname(path);
  int val1 = strcmp(".",n);
  int val2 = strcmp("..",n);
  if((val1 == 0 || val2 == 0) && (start > 0)){
	  return;
  }
  
  if((fd = open(path, 0)) < 0){
    fprintf(2, "ls: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type){
  case T_FILE:
   
    if(1){
	   //do nothing jsut handeling the label erroe 
    }	    
    int val = strcmp(fmtname(path),ffind);
    if(val == 0){
    	    printf("%s\n",path);	
    }
    break;

  case T_DIR:
   
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("ls: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if(stat(buf, &st) < 0){
        printf("ls: cannot stat %s\n", buf);
        continue;
      }
     
      f(buf,ffind,start + 1);
    }
    break;
  }
  close(fd);
}

int
main(int argc,char *argv[])
{ 
  	
  
 
  f(argv[1],argv[2],0);
  exit(0);
  return 0;

}
