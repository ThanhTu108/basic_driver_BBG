#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h> //file_operations
#include <linux/err.h>
#include <linux/cdev.h> //cdev_struct
#include <linux/device.h>
#include <linux/kdev_t.h> //major, minor number
#include <linux/slab.h>	//kmalloc (kernel memory allocation
#include<linux/uaccess.h> //copy_to/from_user



#define mem_size 1024

dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;
uint8_t *kernel_buffer;

static int __init etx_driver_init(void);
static void __exit etx_driver_exit(void);
static int etx_open(struct inode *inode, struct file *file);
static int etx_release(struct inode *inode, struct file *file);
static ssize_t etx_write(struct file *filp, const char *buf, size_t len, loff_t *off);
static ssize_t etx_read(struct file *filp, char __user *buf, size_t len, loff_t *off);

static struct file_operations fops = 
{
	.owner = THIS_MODULE,
	.open = etx_open,
	.release = etx_release,
	.write = etx_write,
	.read = etx_read,
};

static int etx_open(struct inode *inode, struct file *file)
{
	pr_info("DEVICE FILE OPENED....");
	return 0;
}

static int etx_release(struct inode *inode, struct file *file)
{
	pr_info("DEVICE FILE CLOSED...");
	return 0;
}

static ssize_t etx_write(struct file *filp, const char __user  *buf, size_t len, loff_t *off)
{
	if(copy_from_user(kernel_buffer, buf, len))
	{
		pr_err("DATA write : ERR!!!\n");
		return -1;
	}
	pr_info("DATA write : done!!!\n");
	return len;
}
static ssize_t etx_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
	if(copy_to_user(buf, kernel_buffer, mem_size))
	{
		pr_err("DATA read: err!!!\n");
		return -1;
	}
	pr_info("DATA read: done!!!\n");
	return mem_size;
}

static int __init etx_driver_init(void)
{
	//create major and minor number
	if((alloc_chrdev_region(&dev, 0, 1, "mm_num"))<0)
	{
		pr_err("Can not allocate major and minor number\n");
		return -1;
	}
	pr_info("MAJOR = %d, MINOR = %d \n", MAJOR(dev), MINOR(dev));
	
	cdev_init(&etx_cdev, &fops);

	//add device to the system
	if((cdev_add(&etx_cdev, dev, 1))<0)
	{
		pr_err("can not add the devie to the systems\n");
		goto r_del;
	}

	//create class struct
	dev_class = class_create(THIS_MODULE, "etx_class");
	
	if(IS_ERR(dev_class))
	{
		pr_err("can not create the struct class\n");
		goto r_class;
	}
	
	//create the device
	if(IS_ERR(device_create(dev_class, NULL, dev, NULL, "etx_device")))
	{
		pr_err("can not create the device /n");
		goto r_device;
	}
	if((kernel_buffer = kmalloc(mem_size, GFP_KERNEL)) == 0)
        {
                pr_info("CAN NOT ALLOCATE MEMORY IN KERNEL");
                goto r_device;
               // return -1;
        }
	strcpy(kernel_buffer, "HELLO WORLD");
	
	pr_info("device driver insert .... done!!!\n");
	return 0;
r_device:
	class_destroy(dev_class);
r_del:
	cdev_del(&etx_cdev);
r_class:
        unregister_chrdev_region(dev,1);
        return -1;

}

static void __exit etx_driver_exit(void)
{
	device_destroy(dev_class, dev);
	class_destroy(dev_class);
	kfree(kernel_buffer);
	cdev_del(&etx_cdev);
	unregister_chrdev_region(dev, 1);
	pr_info("clear everything... done\n");
}


module_init(etx_driver_init);
module_exit(etx_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");






