#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>

#define MAX_MSG_LEN 256

asmlinkage long sys_printMessage(const char __user *msg) {
    char kbuf[MAX_MSG_LEN];
    long copied;
    
    printk(KERN_DEBUG "printMessage: System call invoked\n");

    copied = strncpy_from_user(kbuf, msg, MAX_MSG_LEN - 1);
    if (copied < 0) {
        printk(KERN_ERR "printMessage: Failed to copy message from user space\n");
        return copied;
    }

    /* Ensure null termination */
    kbuf[copied] = '\0';

    /* Print the message to kernel log */
    printk("print kbuf: %c\n", kbuf);
    printk(KERN_INFO "printMessage: Received message: %s\n", kbuf);

    return 0;
}