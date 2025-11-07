## sec-check-in-kernel-driver

Implementing a security check in a kernel device driver typically involves validating user input, checking permissions, and ensuring safe access to hardware resources. A simple code in C that demonstrates a basic security check in a Linux kernel module

-----

## Header Inclusions

These headers provide access to kernel APIs:

linux/module.h: Core module definitions (e.g., module_init, module_exit)

linux/kernel.h: Kernel logging and basic utilities

linux/fs.h: File system and device operations

linux/uaccess.h: Safe user-space memory access (copy_to_user, copy_from_user)

linux/cdev.h: Character device structures

linux/device.h: Device creation and management

linux/slab.h: Kernel memory allocation (kmalloc, kfree)

linux/cred.h and linux/sched.h: Credential and process info for permission checks

-----

Constants and Globals

#define DEVICE_NAME "securedev"

#define CLASS_NAME  "secureclass"

#define BUFFER_SIZE 1024

DEVICE_NAME: Name of the device node (e.g., /dev/securedev)

CLASS_NAME: Used to group devices under /sys/class/

BUFFER_SIZE: Size of the internal buffer

static int majorNumber;

static struct class*  secureClass  = NULL;

static struct device* secureDevice = NULL;

static char* kernelBuffer;

majorNumber: Assigned major number for the device

secureClass / secureDevice: Used to create device node in /dev

kernelBuffer: Internal buffer for read/write operations

## File Operations

```console

secure_open

if (!capable(CAP_SYS_ADMIN)) {

    return -EPERM;

}

```

Checks if the calling process has administrative privileges (CAP_SYS_ADMIN)

Denies access if not authorized

secure_read

copy_to_user(buffer, kernelBuffer, to_copy)

Copies data from kernel space to user space safely

secure_write

copy_from_user(kernelBuffer, buffer, to_copy)

Copies data from user space to kernel space safely

secure_release

Logs when the device is closed

File Operations Structure

```console

static struct file_operations fops = { ... };

```

Maps system calls (open, read, write, release) to driver functions

Module Initialization

secure_init

```console

kernelBuffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);

```

Allocates memory for the internal buffer

majorNumber = register_chrdev(0, DEVICE_NAME, &fops);

Registers the character device and assigns a major number


```console

secureClass = class_create(...);

secureDevice = device_create(...);

```

Creates a device class and device node (e.g., /dev/securedev)

## Module Cleanup

secure_exit

Destroys device and class

Unregisters the character device

Frees allocated memory

## Module Metadata

MODULE_LICENSE("MIT");

MODULE_AUTHOR("Your Name");

MODULE_DESCRIPTION("A secure kernel device driver");

Provides metadata for the module (license, author, description)

## Summary

This module:

Creates a secure character device

Restricts access to privileged users

Safely handles read/write operations

Cleans up resources on exit


## To compile and install your Linux kernel device driver code

Step 1: Prepare Your Environment

Ensure you have the following:

A Linux system with kernel headers installed (linux-headers-$(uname -r))

Root access (for installing the module)

A working C compiler (gcc) and make

Step 2: Create the Module Files

Create a directory and add your source code

mkdir securedev

cd securedev

Save your code in a file named securedev.c.

Step 3: Create a Makefile

Create a file named Makefile with the following content:

```console

obj-m += securedev.o

```

all:


```console

    make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

```

clean:

```console

    make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean


```

Step 4: Compile the Module

Run:

```console

make

```

This will generate a file named securedev.ko, which is your kernel module.

Step 5: Install the Module

Load the module


```console

sudo insmod securedev.ko


```

Check kernel logs

```console

dmesg | tail

```

You should see messages like:

securedev: device initialized

Step 6: Verify Device Creation

Check if the device node exists:

```console

ls /dev/securedev


```

If not, create it manually:

```console

sudo mknod /dev/securedev c 0

```

Replace with the number printed in dmesg.

Step 7: Uninstall the Module

To remove the module:

```console

sudo rmmod securedev

```

Clean up build files:

```console

make clean

```

## Summary

You’ve now compiled, installed, and tested your secure kernel device driver.



