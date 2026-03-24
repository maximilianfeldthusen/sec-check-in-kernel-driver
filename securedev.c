
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/cred.h>
#include <linux/sched.h>
#include <linux/mutex.h>

#define DEVICE_NAME "securedev"
#define CLASS_NAME  "secureclass"
#define BUFFER_SIZE 1024

static int majorNumber;
static struct class* secureClass = NULL;
static struct device* secureDevice = NULL;
static char* kernelBuffer;
static struct cdev secure_cdev;
static dev_t dev_number;
static DEFINE_MUTEX(secure_mutex); // Mutex for thread safety

// Helper to check permissions
static bool is_root_user(void) {
    return capable(CAP_SYS_ADMIN);
}

static int secure_open(struct inode *inodep, struct file *filep) {
    if (!is_root_user()) {
        pr_warn("%s: Access denied. CAP_SYS_ADMIN required.\n", DEVICE_NAME);
        return -EPERM;
    }
    pr_info("%s: Device opened by root.\n", DEVICE_NAME);
    return 0;
}

static ssize_t secure_read(struct file *filep, char __user *buffer, size_t len, loff_t *offset) {
    size_t to_copy;
   
    if (*offset >= BUFFER_SIZE) {
        return 0; // End of data
    }

    mutex_lock(&secure_mutex);
    to_copy = min(len, (size_t)(BUFFER_SIZE - *offset));
   
    if (copy_to_user(buffer, kernelBuffer + *offset, to_copy)) {
        mutex_unlock(&secure_mutex);
        return -EFAULT;
    }
   
    *offset += to_copy;
    mutex_unlock(&secure_mutex);
   
    pr_info("%s: Read %zu bytes.\n", DEVICE_NAME, to_copy);
    return to_copy;
}

static ssize_t secure_write(struct file *filep, const char __user *buffer, size_t len, loff_t *offset) {
    size_t to_copy;

    mutex_lock(&secure_mutex);
    to_copy = min(len, (size_t)BUFFER_SIZE);
   
    if (copy_from_user(kernelBuffer, buffer, to_copy)) {
        mutex_unlock(&secure_mutex);
        return -EFAULT;
    }
   
    // Clear remaining buffer to prevent stale data leakage if write < BUFFER_SIZE
    memset(kernelBuffer + to_copy, 0, BUFFER_SIZE - to_copy);
   
    mutex_unlock(&secure_mutex);
   
    pr_info("%s: Wrote %zu bytes.\n", DEVICE_NAME, to_copy);
    return to_copy;
}

static int secure_release(struct inode *inodep, struct file *filep) {
    pr_info("%s: Device closed.\n", DEVICE_NAME);
    return 0;
}

static const struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = secure_open,
    .read = secure_read,
    .write = secure_write,
    .release = secure_release,
};

static int __init secure_init(void) {
    int ret;

    // Allocate dynamic major number
    ret = alloc_chrdev_region(&dev_number, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        pr_err("%s: Failed to allocate chrdev region.\n", DEVICE_NAME);
        return ret;
    }
    majorNumber = MAJOR(dev_number);
    pr_info("%s: Registered with major number %d.\n", DEVICE_NAME, majorNumber);

    // Initialize and add character device
    cdev_init(&secure_cdev, &fops);
    secure_cdev.owner = THIS_MODULE;
   
    ret = cdev_add(&secure_cdev, dev_number, 1);
    if (ret < 0) {
        pr_err("%s: Failed to add cdev.\n", DEVICE_NAME);
        goto fail_cdev;
    }

    // Create class
    secureClass = class_create(DEVICE_NAME);
    if (IS_ERR(secureClass)) {
        pr_err("%s: Failed to create class.\n", DEVICE_NAME);
        ret = PTR_ERR(secureClass);
        goto fail_class;
    }

    // Create device
    secureDevice = device_create(secureClass, NULL, dev_number, NULL, DEVICE_NAME);
    if (IS_ERR(secureDevice)) {
        pr_err("%s: Failed to create device.\n", DEVICE_NAME);
        ret = PTR_ERR(secureDevice);
        goto fail_device;
    }

    // Allocate buffer
    kernelBuffer = kzalloc(BUFFER_SIZE, GFP_KERNEL);
    if (!kernelBuffer) {
        pr_err("%s: Failed to allocate kernel buffer.\n", DEVICE_NAME);
        ret = -ENOMEM;
        goto fail_buffer;
    }

    pr_info("%s: Initialization successful.\n", DEVICE_NAME);
    return 0;

fail_buffer:
    device_destroy(secureClass, dev_number);
fail_device:
    class_destroy(secureClass);
fail_class:
    cdev_del(&secure_cdev);
fail_cdev:
    unregister_chrdev_region(dev_number, 1);
    return ret;
}

static void __exit secure_exit(void) {
    pr_info("%s: Cleaning up.\n", DEVICE_NAME);

    if (kernelBuffer) {
        kfree(kernelBuffer);
        kernelBuffer = NULL;
    }

    if (secureDevice)
        device_destroy(secureClass, dev_number);
   
    if (secureClass)
        class_destroy(secureClass);

    cdev_del(&secure_cdev);
    unregister_chrdev_region(dev_number, 1);

    pr_info("%s: Cleanup complete.\n", DEVICE_NAME);
}

module_init(secure_init);
module_exit(secure_exit);

MODULE_LICENSE("MIT");
MODULE_AUTHOR("Updated by AI Assistant");
MODULE_DESCRIPTION("A secure kernel device driver sample with modern best practices");
MODULE_VERSION("1.1");

