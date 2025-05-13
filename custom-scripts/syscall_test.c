#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define SYSCALL_PROCESSINFO 385

void usage(char* s){
    printf("Usage: %s\n", s);
    exit(0);
}

int main(int argc, char** argv){  
    char buf[50000];
    long ret;

    buf = argv[1];
    printf("Invoking 'listProcessInfo' system call to list all sleeping processes.\n");
         
    ret = syscall(SYSCALL_PROCESSINFO, buf, sizeof(buf)); 
         
    if(ret > 0) {
        printf("%s\n", buf);
    }
          
    return 0;
}