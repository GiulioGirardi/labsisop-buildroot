/**
 * @brief   An introductory character driver. This module maps to /dev/simple_driver and
 * comes with a helper C program that can be run in Linux user space to communicate with
 * this the LKM.
 *
 * Modified from Derek Molloy (http://www.derekmolloy.ie/)
 */

#include <linux/init.h>           // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/kernel.h>         // Contains types, macros, functions for the kernel
#include <linux/fs.h>             // Header for the Linux file system support
#include <linux/uaccess.h>

#define DEVICE_NAME "xtea_driver" ///< The device will appear at /dev/xtea_driver
#define CLASS_NAME  "xtea_class"  ///< The device class
#define MAX_MESSAGE_SIZE 512      ///< Max buffer size for input/output messages

MODULE_LICENSE("GPL");            ///< The license type -- this affects available functionality
MODULE_AUTHOR("Author Name");    ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("A generic Linux char driver.");  ///< The description -- see modinfo
MODULE_VERSION("0.2");            ///< A version number to inform users

// Module parameters for the XTEA key
static uint32_t key0 = 0x00000000;
static uint32_t key1 = 0x00000000;
static uint32_t key2 = 0x00000000;
static uint32_t key3 = 0x00000000;
module_param(key0, uint, 0644);
module_param(key1, uint, 0644);
module_param(key2, uint, 0644);
module_param(key3, uint, 0644);
MODULE_PARM_DESC(key0, "First 32-bit key for XTEA encryption");
MODULE_PARM_DESC(key1, "Second 32-bit key for XTEA encryption");
MODULE_PARM_DESC(key2, "Third 32-bit key for XTEA encryption");
MODULE_PARM_DESC(key3, "Fourth 32-bit key for XTEA encryption");

// Global variables
static int majorNumber;
static char *message = NULL;       ///< Dynamic buffer for encrypted data
static size_t size_of_message;     ///< Size of the stored message
static int numberOpens = 0;
static struct class *charClass = NULL;
static struct device *charDevice = NULL;
static uint32_t xtea_key[4];       ///< XTEA key set from module parameters

// Function prototypes
static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

// XTEA encryption functions
void encipher(uint32_t num_rounds, uint32_t v[2], const uint32_t key[4]) {
    uint32_t i;
    uint32_t v0 = v[0], v1 = v[1], sum = 0, delta = 0x9E3779B9;

    for (i = 0; i < num_rounds; i++) {
        v0 += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]);
        sum += delta;
        v1 += (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum >> 11) & 3]);
    }
    v[0] = v0;
    v[1] = v1;
}

// File operations structure
static struct file_operations fops = {
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
    .release = dev_release,
};

/**
 * @brief Initialize the LKM
 */
static int __init xtea_init(void) {
    printk(KERN_INFO "XTEA Driver: Initializing the LKM\n");

    // Allocate dynamic buffer
    message = kmalloc(MAX_MESSAGE_SIZE, GFP_KERNEL);
    if (!message) {
        printk(KERN_ALERT "XTEA Driver: Failed to allocate memory for message buffer\n");
        return -ENOMEM;
    }
    memset(message, 0, MAX_MESSAGE_SIZE);

    // Set the XTEA key from module parameters
    xtea_key[0] = key0;
    xtea_key[1] = key1;
    xtea_key[2] = key2;
    xtea_key[3] = key3;
    printk(KERN_INFO "XTEA Driver: Key set to %08x %08x %08x %08x\n",
           xtea_key[0], xtea_key[1], xtea_key[2], xtea_key[3]);

    // Register major number
    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if (majorNumber < 0) {
        kfree(message);
        printk(KERN_ALERT "XTEA Driver: Failed to register a major number\n");
        return majorNumber;
    }
    printk(KERN_INFO "XTEA Driver: Registered with major number %d\n", majorNumber);

    // Register device class
    charClass = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(charClass)) {
        unregister_chrdev(majorNumber, DEVICE_NAME);
        kfree(message);
        printk(KERN_ALERT "XTEA Driver: Failed to register device class\n");
        return PTR_ERR(charClass);
    }
    printk(KERN_INFO "XTEA Driver: Device class registered\n");

    // Create device
    charDevice = device_create(charClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(charDevice)) {
        class_destroy(charClass);
        unregister_chrdev(majorNumber, DEVICE_NAME);
        kfree(message);
        printk(KERN_ALERT "XTEA Driver: Failed to create the device\n");
        return PTR_ERR(charDevice);
    }
    printk(KERN_INFO "XTEA Driver: Device created successfully\n");

    return 0;
}

/**
 * @brief Cleanup the LKM
 */
static void __exit xtea_exit(void) {
    device_destroy(charClass, MKDEV(majorNumber, 0));
    class_unregister(charClass);
    class_destroy(charClass);
    unregister_chrdev(majorNumber, DEVICE_NAME);
    kfree(message);
    printk(KERN_INFO "XTEA Driver: Goodbye from the LKM!\n");
}

/**
 * @brief Device open function
 */
static int dev_open(struct inode *inodep, struct file *filep) {
    numberOpens++;
    printk(KERN_INFO "XTEA Driver: Device opened %d time(s)\n", numberOpens);
    return 0;
}

/**
 * @brief Device read function
 */
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
    int error_count = 0;

    if (*offset >= size_of_message) {
        return 0; // EOF
    }

    // Copy encrypted data to userspace
    error_count = copy_to_user(buffer, message, size_of_message);

    if (error_count == 0) {
        printk(KERN_INFO "XTEA Driver: Sent %zu bytes to user\n", size_of_message);
        *offset += size_of_message;
        return size_of_message;
    } else {
        printk(KERN_ALERT "XTEA Driver: Failed to send %d bytes to user\n", error_count);
        return -EFAULT;
    }
}

/**
 * @brief Device write function
 * Expects input in the format: "enc key0 key1 key2 key3 data_size data"
 */
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {
    char *input = NULL;
    char command[4];
    uint32_t keys[4];
    uint32_t data_size;
    unsigned char *data = NULL;
    int ret;

    // Allocate temporary buffer for input
    input = kmalloc(len + 1, GFP_KERNEL);
    if (!input) {
        printk(KERN_ALERT "XTEA Driver: Failed to allocate input buffer\n");
        return -ENOMEM;
    }

    // Copy data from userspace
    if (copy_from_user(input, buffer, len)) {
        kfree(input);
        printk(KERN_ALERT "XTEA Driver: Failed to copy data from user\n");
        return -EFAULT;
    }
    input[len] = '\0';

    // Parse input: command, keys, data_size, data
    ret = sscanf(input, "%3s %x %x %x %x %u", command, &keys[0], &keys[1], &keys[2], &keys[3], &data_size);
    if (ret != 6 || strcmp(command, "enc") != 0 || data_size % 8 != 0 || data_size > 256) {
        kfree(input);
        printk(KERN_ALERT "XTEA Driver: Invalid command format or data size\n");
        return -EINVAL;
    }

    // Allocate buffer for data
    data = kmalloc(data_size, GFP_KERNEL);
    if (!data) {
        kfree(input);
        printk(KERN_ALERT "XTEA Driver: Failed to allocate data buffer\n");
        return -ENOMEM;
    }

    // Extract data (hex string) from input
    char *data_start = strstr(input, " ") + 1; // Skip command
    for (int i = 0; i < 4; i++) data_start = strstr(data_start, " ") + 1; // Skip keys
    data_start = strstr(data_start, " ") + 1; // Skip data_size
    for (int i = 0; i < data_size; i++) {
        unsigned int byte;
        if (sscanf(data_start + 2 * i, "%2x", &byte) != 1) {
            kfree(data);
            kfree(input);
            printk(KERN_ALERT "XTEA Driver: Invalid data format\n");
            return -EINVAL;
        }
        data[i] = (unsigned char)byte;
    }

    // Perform encryption (data_size is in bytes, process 8 bytes at a time)
    size_of_message = 0;
    for (size_t i = 0; i < data_size; i += 8) {
        uint32_t v[2];
        // Convert 8 bytes to two 32-bit words (little-endian)
        v[0] = ((uint32_t)data[i] << 24) | ((uint32_t)data[i + 1] << 16) |
               ((uint32_t)data[i + 2] << 8) | (uint32_t)data[i + 3];
        v[1] = ((uint32_t)data[i + 4] << 24) | ((uint32_t)data[i + 5] << 16) |
               ((uint32_t)data[i + 6] << 8) | (uint32_t)data[i + 7];
        // Encrypt
        encipher(32, v, xtea_key);
        // Store result in message buffer
        for (int j = 0; j < 4; j++) {
            message[size_of_message++] = (v[0] >> (24 - j * 8)) & 0xFF;
            message[size_of_message++] = (v[1] >> (24 - j * 8)) & 0xFF;
        }
    }

    kfree(data);
    kfree(input);
    printk(KERN_INFO "XTEA Driver: Processed %zu bytes from user\n", len);
    return len;
}

/**
 * @brief Device release function
 */
static int dev_release(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "XTEA Driver: Device closed\n");
    return 0;
}

module_init(xtea_init);
module_exit(xtea_exit);