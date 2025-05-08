// #include <linux/kernel.h>
// #include <linux/init.h>
// #include <linux/sched.h>
// #include <linux/syscalls.h>
// #include "processInfo.h"
  
// asmlinkage long sys_listProcessInfo(long pid, const char __user *buf, int size) {
// 	struct task_struct *proces;
// 	unsigned char kbuf[256];
// 	int bufsz;
// 	int ret;

// 	/* Find the process */
// 	for_each_process(proces) {
// 		if( (long)task_pid_nr(proces) == pid){
// 			/* Print the process info to the buffer */
// 			snprintf(kbuf, sizeof(kbuf), "Process: %s\n PID_Number: %ld\n Process State: %ld\n Priority: %ld\n RT_Priority: %ld\n Static Priority: %ld\n Normal Priority: %ld\n", 
// 					proces->comm, 
// 					(long)task_pid_nr(proces), 
// 					(long)proces->state, 
// 					(long)proces->prio, 
// 					(long)proces->rt_priority, 
// 					(long)proces->static_prio, (long)proces->normal_prio);
// 			bufsz = strlen(kbuf)+1;

// 			/* User buffer is too small */
// 			if(bufsz > size){
// 				return -1;
// 			}

// 			/* success */
// 			ret = copy_to_user((void*)buf, (void*)kbuf, bufsz);

// 			return bufsz - ret;
// 		}
// 	}

// 	/* Process not found */
// 	return -2;	
// }


#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/cred.h>
#include "processInfo.h"

asmlinkage long sys_listProcessInfo(const char __user *buf, int size) {
    struct task_struct *proces;
    unsigned char kbuf[16384]; // 16KB buffer
    int bufsz = 0;
    int ret;
    int offset = 0;

    printk(KERN_DEBUG "listProcessInfo: System call invoked by UID %d, buf=%p, size=%d\n",
           from_kuid(&init_user_ns, current_uid()), buf, size);

    /* Validate user buffer */
    if (!buf || size <= 0) {
        printk(KERN_ERR "listProcessInfo: Invalid user buffer or size\n");
        return -EINVAL;
    }

    for_each_process(proces) {
        if (proces->state == TASK_UNINTERRUPTIBLE || proces->state == TASK_INTERRUPTIBLE) {
            /* Format process info */
            int len = snprintf(kbuf + offset, sizeof(kbuf) - offset,
                             "Process: %s\n PID_Number: %ld\n Process State: %ld\n Priority: %ld\n RT_Priority: %ld\n Static Priority: %ld\n Normal Priority: %ld\n\n",
                             proces->comm,
                             (long)task_pid_nr(proces),
                             (long)proces->state,
                             (long)proces->prio,
                             (long)proces->rt_priority,
                             (long)proces->static_prio,
                             (long)proces->normal_prio);

            /* Check if buffer has enough space */
            if (offset + len >= sizeof(kbuf)) {
                printk(KERN_ERR "listProcessInfo: Buffer overflow, need larger buffer\n");
                return -EFBIG;
            }

            offset += len;
        }
    }

    bufsz = offset + 1;

    /* User buffer is too small */
    if (bufsz > size) {
        printk(KERN_ERR "listProcessInfo: User buffer too small, need %d bytes\n", bufsz);
        return -EINVAL;
    }

    /* Copy to user space */
    ret = copy_to_user((void*)buf, (void*)kbuf, bufsz);
    if (ret) {
        printk(KERN_ERR "listProcessInfo: Failed to copy %d bytes to user space\n", ret);
        return -EFAULT;
    }

    printk(KERN_DEBUG "listProcessInfo: Successfully copied %d bytes\n", bufsz);
    return bufsz;
}