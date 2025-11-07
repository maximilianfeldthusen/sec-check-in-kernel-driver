

#include <linux/module.h>

#include <linux/kernel.h>

#include <linux/fs.h>

#include <linux/uaccess.h>

#include <linux/cdev.h>

#include <linux/device.h>

#include <linux/slab.h>

#include <linux/cred.h>

#include <linux/sched.h>

#define DEVICE_NAME "securedev"

#define CLASS_NAME  "secureclass"

#define BUFFER_SIZE 1024

static int majorNumber;

static struct class*  secureClass  = NULL;

static struct device* secureDevice = NULL;

static char* kernelBuffer;

static int secure_open(struct inode *inodep, struct file *filep) {

    // Security check: only allow root user

    if (!capable(CAP_SYS_ADMIN)) {

        pr_warn("securedev: access denied, not CAP_SYS_ADMIN\n");

        return -EPERM;

    }

    pr_info("securedev: device opened\n");

    return 0;

}

static ssize_t secure_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {

    size_t to_copy = min(len, (size_t)BUFFER_SIZE);

    if (copy_to_user(buffer, kernelBuffer, to_copy)) {

        return -EFAULT;

    }

    return to_copy;

}

static ssize_t secure_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {

    size_t to_copy = min(len, (size_t)BUFFER_SIZE);

    if (copy_from_user(kernelBuffer, buffer, to_copy)) {

        return -EFAULT;

    }

    return to_copy;

}

static int secure_release(struct inode *inodep, struct file *filep) {

    pr_info("securedev: device closed\n");

    return 0;

}

static struct file_operations fops = {

    .open = secure_open,

    .read = secure_read,

    .write = secure_write,

    .release = secure_release,

};

static int __init secure_init(void) {

    kernelBuffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);

    if (!kernelBuffer) {

        pr_err("securedev: failed to allocate buffer\n");

        return -ENOMEM;

    }

    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);

    if (majorNumber < 0) {

        pr_err("securedev: failed to register device\n");

        kfree(kernelBuffer);

        return majorNumber;

    }

    secureClass = class_create(THIS_MODULE, CLASS_NAME);

    if (IS_ERR(secureClass)) {

        unregister_chrdev(majorNumber, DEVICE_NAME);

        kfree(kernelBuffer);

        return PTR_ERR(secureClass);

    }

    secureDevice = device_create(secureClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);

    if (IS_ERR(secureDevice)) {

        class_destroy(secureClass);

        unregister_chrdev(majorNumber, DEVICE_NAME);

        kfree(kernelBuffer);

        return PTR_ERR(secureDevice);

    }

    pr_info("securedev: device initialized\n");

    return 0;

}

static void __exit secure_exit(void) {

    device_destroy(secureClass, MKDEV(majorNumber, 0));

    class_unregister(secureClass);

    class_destroy(secureClass);

    unregister_chrdev(majorNumber, DEVICE_NAME);

    kfree(kernelBuffer);

    pr_info("securedev: device removed\n");

}

module_init(secure_init);

module_exit(secure_exit);

MODULE_LICENSE("MIT");

MODULE_AUTHOR("Your Name");

MODULE_DESCRIPTION("A secure kernel device driver sample");
