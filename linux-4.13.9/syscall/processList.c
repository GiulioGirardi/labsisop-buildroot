#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>

asmlinkage long sys_get_sleeping_pids(get_sleeping_pids, pid_t __user *, pids, int, max){
    struct task_struct *task;
    pid_t k_pids[256];
    int count = 0;

    rcu_read_lock();
    for_each_process(task) {
        if (task->state == TASK_INTERRUPTIBLE || task->state == TASK_UNINTERRUPTIBLE) {
            if (count < max && count < ARRAY_SIZE(k_pids)) {
                k_pids[count++] = task->pid;
            }
        }
    }
    rcu_read_unlock();

    if (copy_to_user(pids, k_pids, count * sizeof(pid_t)))
        return -EFAULT;

    return count;
}
