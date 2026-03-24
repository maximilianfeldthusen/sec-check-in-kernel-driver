## sec-check-in-kernel-driver

---

## Explanation (What this code does)

This is a **Linux kernel character device driver** called `securedev` with built-in **security and safety features**.

## Core Idea

It creates a device file (e.g. `/dev/securedev`) that:

* Only **root (CAP_SYS_ADMIN)** can open
* Allows **read/write of a kernel buffer**
* Uses **mutex locking** for thread safety
* Prevents **data leaks and race conditions**

---

##  Key Components Explained

### 1. Device Setup

* Uses `alloc_chrdev_region()` → dynamically assigns major number
* Registers device with:

  * `cdev_init()`
  * `cdev_add()`
* Creates:

  * Device class → `/sys/class/secureclass`
  * Device node → `/dev/securedev`

---

### 2. Security (Important )

#### Root-only access

```c
return capable(CAP_SYS_ADMIN);
```

* Only privileged users can open device
* Prevents unauthorized access

---

### 3. Thread Safety

```c
DEFINE_MUTEX(secure_mutex);
```

* Protects shared `kernelBuffer`
* Prevents race conditions in read/write

---

### 4. Kernel Buffer Management

```c
kernelBuffer = kzalloc(BUFFER_SIZE, GFP_KERNEL);
```

* Allocated in kernel space
* Zero-initialized (security best practice)

---

### 5. Read Operation

```c
copy_to_user(...)
```

* Safely copies data **kernel → user**
* Respects offset and size
* Prevents overflow

---

### 6. Write Operation

```c
copy_from_user(...)
```

* Copies data **user → kernel**
* Clears unused memory:

```c
memset(kernelBuffer + to_copy, 0, BUFFER_SIZE - to_copy);
```

 Prevents **stale data leakage**

---

### 7. Cleanup

* Frees memory (`kfree`)
* Removes device + class
* Unregisters driver

---

## ⚠️ Security Best Practices Used

*  Capability-based access control (`CAP_SYS_ADMIN`)
*  Mutex locking (race condition protection)
* Zeroed memory (`kzalloc`)
* Buffer clearing (`memset`)
*  Safe user/kernel copy APIs

---

#  GitHub-Ready Markdown (README style)

# Secure Linux Character Device Driver (`securedev`)

## Overview

This project implements a **secure Linux kernel character device driver** with modern best practices:

* Root-only access (`CAP_SYS_ADMIN`)
* Thread-safe operations using mutex
* Safe user-kernel memory handling
* Protection against data leakage

---

## Features

*  **Access Control** – Only privileged users can open the device
*  **Thread Safety** – Mutex-protected read/write
*  **Memory Safety** – Uses `kzalloc` and safe copy APIs
*  **Secure Cleanup** – Prevents memory leaks

---

## Device Details

| Property    | Value            |
| ----------- | ---------------- |
| Device Name | `/dev/securedev` |
| Class       | `secureclass`    |
| Buffer Size | 1024 bytes       |

---

## Code Structure

```text
securedev.c
├── Headers & Macros
├── Global Variables
├── Security Helper (is_root_user)
├── File Operations
│   ├── open
│   ├── read
│   ├── write
│   └── release
├── Initialization (secure_init)
└── Cleanup (secure_exit)
```

---

## Key Functions

### Access Control

```c
static bool is_root_user(void) {
    return capable(CAP_SYS_ADMIN);
}
```

* Ensures only privileged users can access the device

---

###  Open

```c
static int secure_open(struct inode *inodep, struct file *filep)
```

* Denies access if not root
* Logs access attempts

---

### Read

```c
static ssize_t secure_read(...)
```

* Copies data from kernel buffer to user space
* Uses mutex for safety
* Handles offsets correctly

---

###  Write

```c
static ssize_t secure_write(...)
```

* Copies user data into kernel buffer
* Clears unused memory to prevent leaks

---

###  Release

```c
static int secure_release(...)
```

* Logs device close event

---

## Initialization Flow

```c
secure_init()
```

1. Allocate device number
2. Initialize character device
3. Create class
4. Create device node
5. Allocate kernel buffer

---

## Cleanup Flow

```c
secure_exit()
```

* Free buffer
* Destroy device
* Remove class
* Unregister device

---

## Build Instructions

```bash
make -C /lib/modules/$(uname -r)/build M=$(pwd) modules
```

---

## Load & Test

```bash
sudo insmod securedev.ko
sudo dmesg | tail
ls /dev/securedev
```

### Write

```bash
echo "hello" | sudo tee /dev/securedev
```

### Read

```bash
sudo cat /dev/securedev
```

---

## Security Considerations

* Requires `CAP_SYS_ADMIN` (root access)
* Prevents:

  * Race conditions
  * Buffer overflows
  * Data leakage
* Uses safe kernel APIs (`copy_to_user`, `copy_from_user`)

---

## License

MIT

---

## Author

Updated by AI Assistant

---

If you want, I can also:

* add a **Makefile**
* show **how to test with multiple processes (race condition demo)**
* or extend this into a **real secure IPC mechanism**

