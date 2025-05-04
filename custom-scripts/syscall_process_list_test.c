#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

#define SYS_get_sleeping_pids 386

int main() {
    pid_t pids[64];
    int count = syscall(SYS_get_sleeping_pids, pids, 64);
    if (count < 0) {
        perror("syscall failed");
        return 1;
    }

    printf("Sleeping processes:\n");
    for (int i = 0; i < count; i++) {
        printf("PID: %d\n", pids[i]);
    }

    return 0;
}