/**************************************************************
* Class:  CSC-415-01 Fall 2020
* Name: Rinay Kumar
* Student ID: 913859133
* GitHub ID: rinaykumar
* Project: Assignment 6 - Device Driver
*
* File: kernelMod.c
*
* Description: This Linux Kernel Module connects with the 
* user space testProgram to return the nth number in the 
* Fibonacci Sequence 
*
* Template followed from: 
* http://derekmolloy.ie/writing-a-linux-kernel-module-part-2-a-character-device/
*
**************************************************************/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>

#define DEVICE_NAME "kernelmod"
#define CLASS_NAME "kmod"
#define WR_VALUE _IOW('a', 'a', int)
#define RD_VALUE _IOR('a', 'b', int)

MODULE_LICENSE("GPL");

static int majorNumber;
static char message[256] = {0};
static short size_of_message;
static int value = 0;
static int fibResult = 0;
static struct class *kernelmodClass = NULL;
static struct device *kernelmodDevice = NULL;

// Prototype functions
static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
static long dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
static int dev_fibonacci(int n);

static struct file_operations fops =
    {
        .open = dev_open,
        .read = dev_read,
        .write = dev_write,
        .unlocked_ioctl = dev_ioctl,
        .release = dev_release,
};

// Initializes kernel mod with magic number and registers class
static int __init kernelmod_init(void)
{
   printk(KERN_INFO "KernelMod: Initializing the KernelMod LKM\n");

   majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
   
   if (majorNumber < 0)
   {
      printk(KERN_ALERT "KernelMod: Failed to register a major number\n");
      return majorNumber;
   }
   
   printk(KERN_INFO "KernelMod: Registered correctly with major number %d\n", majorNumber);

   kernelmodClass = class_create(THIS_MODULE, CLASS_NAME);
   
   if (IS_ERR(kernelmodClass))
   { 
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "KernelMod: Failed to register device class\n");
      return PTR_ERR(kernelmodClass);
   }
   
   printk(KERN_INFO "KernelMod: Device class registered correctly\n");

   kernelmodDevice = device_create(kernelmodClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
   
   if (IS_ERR(kernelmodDevice))
   {                                 
      class_destroy(kernelmodClass); 
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "KernelMod: Failed to create the device\n");
      return PTR_ERR(kernelmodDevice);
   }
   
   printk(KERN_INFO "KernelMod: Device class created correctly\n");
   
   return 0;
}

// Exit, destroys and unregisters
static void __exit kernelmod_exit(void)
{
   device_destroy(kernelmodClass, MKDEV(majorNumber, 0)); 
   class_unregister(kernelmodClass);                      
   class_destroy(kernelmodClass);                         
   unregister_chrdev(majorNumber, DEVICE_NAME);           
   printk(KERN_INFO "KernelMod: Goodbye from the LKM\n");
}

// Open, prints to kernel log
static int dev_open(struct inode *inodep, struct file *filep)
{
   printk(KERN_INFO "KernelMod: Device opened\n");
   return 0;
}

// Read, sends message to user buffer
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
   int error_count = 0;
   
   error_count = copy_to_user(buffer, message, size_of_message);

   if (error_count == 0)
   { 
      printk(KERN_INFO "KernelMod: Sent %d characters to the user\n", size_of_message);
      return (size_of_message = 0); 
   }
   else
   {
      printk(KERN_INFO "KernelMod: Failed to send %d characters to the user\n", error_count);
      return -EFAULT; 
   }
}

// Write, receives message from user
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
   sprintf(message, "%s", buffer);
   size_of_message = strlen(message);
   printk(KERN_INFO "KernelMod: Received %zu characters from the user\n", len);
   return len;
}

// IOCTL function, on write receives from user, on read sends to user
static long dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
   switch (cmd)
   {
   case WR_VALUE:
      copy_from_user(&value, (int32_t*) arg, sizeof(value));
      printk(KERN_INFO "KernelMod: Received value %d from the user\n", value);
      break;
   case RD_VALUE:
      fibResult = dev_fibonacci(value);
      copy_to_user((int32_t*) arg, &fibResult, sizeof(fibResult));
      printk(KERN_INFO "KernelMod: Sent Fibonacci result %d to the user\n", fibResult);
      break;
   }

   return 0;
}

// Recursize function that calculates nth Fibonacci Sequence number
static int dev_fibonacci(int n)
{
   if (n <= 1)
   {
      return n;
   }
   return dev_fibonacci(n - 1) + dev_fibonacci(n - 2);
}

// Release, prints to kernel log
static int dev_release(struct inode *inodep, struct file *filep)
{
   printk(KERN_INFO "KernelMod: Device successfully closed\n");
   return 0;
}

module_init(kernelmod_init);
module_exit(kernelmod_exit);
