#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>	//file_operation
#include <linux/err.h>
#include <linux/cdev.h>
#include <linux/device.h>

dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;

static int __init etx_driver_init(void);
static void __exit etx_driver_exit(void);

static int etx_open (struct inode *inode, struct file *file);
static int etx_release(struct inode *inode, struct file *file);
static ssize_t etx_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
static ssize_t etx_write(struct file *filp, const char __user *buf, size_t len, loff_t *off);

static struct file_operations fops =
{
	.owner = THIS_MODULE,
	.read  = etx_read,
	.write = etx_write,
	.open  = etx_open,
	.release = etx_release,
};
static int etx_open(struct inode *inode, struct file *file)
{
	pr_info("driver open function called...\n");
	return 0;
}

static int etx_release(struct inode *inode, struct file *file)
{
	pr_info("driver release function called..\n");
	return 0;
}
static ssize_t etx_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
	pr_info("driver read function called");
	return 0;
}
static ssize_t etx_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
	pr_info("driver write function called..\n");
	return len;
}
static int __init etx_driver_init(void)
{
	if((alloc_chrdev_region(&dev, 0, 1, "thanh_tu")) <0)
	{
		pr_err("Can not create major and minor number \n");
		return -1;
	}
	pr_info("Major= %d, Minor = %d \n",MAJOR(dev), MINOR(dev));

	cdev_init(&etx_cdev, &fops);

	if((cdev_add(&etx_cdev, dev, 1))<0)
	{
		pr_err("Can not add the device to the system \n");
		goto r_del;
	}
	dev_class = class_create(THIS_MODULE, "etx_class");
	if(IS_ERR(dev_class))
	{
		pr_err("Can not create struct class\n");
		goto r_class;
	}
	if(IS_ERR(device_create(dev_class, NULL, dev, NULL, "etx_device")))
	{
		pr_err("Can not create device\n");
		goto r_device;
	}
	pr_info("Device driver insert .... Done!!!\n");
	return 0;

r_class:
	unregister_chrdev_region(dev,1);
	return -1;
r_device:
	class_destroy(dev_class);
r_del:
	cdev_del(&etx_cdev);
	return -1;
}

static void __exit etx_driver_exit(void)
{
	device_destroy(dev_class, dev);
	class_destroy(dev_class);
	cdev_del(&etx_cdev);
	unregister_chrdev_region(dev,1);
	pr_info("Device driver remove done\n");
}

module_init(etx_driver_init);
module_exit(etx_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");
MODULE_DESCRIPTION("simple linux device driver (file operation)");


