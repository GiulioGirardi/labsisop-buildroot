// #include <stdio.h>
// #include <linux/kernel.h>
// #include <sys/syscall.h>
// #include <unistd.h>
// #include <stdlib.h>

// #define SYSCALL_PROCESSINFO	385

// void usage(char* s){
// 	printf("Usage: %s <PID>\n", s);
// 	exit(0);
// }

// int main(int argc, char** argv){  
// 	char buf[1024];
// 	long ret;
// 	int pid;
	
// 	if(argc < 2){
// 		usage(argv[0]);
// 	}
	
// 	pid = atoi(argv[1]);
	
// 	printf("Invoking 'listProcessInfo' system call.\n");
         
// 	ret = syscall(SYSCALL_PROCESSINFO, buf, sizeof(buf)); 
         
// 	if(ret > 0) {
// 		/* Success, show the process info. */
// 		printf("%s\n", buf);
// 	}
// 	else {
// 		printf("System call 'listProcessInfo' did not execute as expected error %d\n", ret);
// 	}
          
// 	return 0;
// }

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
    printf("No PID required - lists all processes in sleep state\n");
    printf("Run with sudo (e.g., 'sudo %s') to ensure proper permissions\n", s);
    exit(0);
}

int main(int argc, char** argv){  
    char buf[16384]; // 16KB buffer to match kernel
    long ret;
    
    if(argc > 1){
        usage(argv[0]);
    }

    /* Check if running as root */
    if (getuid() != 0) {
        fprintf(stderr, "Warning: This program should be run with sudo to avoid permission issues\n");
    }
    
    printf("Invoking 'listProcessInfo' system call to list all sleeping processes.\n");
         
    ret = syscall(SYSCALL_PROCESSINFO, buf, sizeof(buf)); 
         
    if(ret > 0) {
        /* Success, show the process info. */
        printf("%s\n", buf);
    }
    else {
        printf("System call 'listProcessInfo' failed with error %ld: %s\n", 
               ret, ret == -EINVAL ? "Invalid argument (buffer too small or invalid)" :
                    ret == -EFBIG ? "Kernel buffer overflow" :
                    ret == -EFAULT ? "Failed to copy data to user space" :
                    ret == -EPERM ? "Permission denied (try running with sudo)" :
                    ret == -ENOSYS ? "System call not implemented" :
                    strerror(-ret));
    }
          
    return 0;
}