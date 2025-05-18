#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/cdev.h> 	//cdev_struct
#include <linux/kdev_t.h>	//major/ minor number
#include <linux/fs.h>		//file_operation
#include <linux/err.h>
#include <linux/slab.h> 	//kmalloc()
#include <linux/uaccess.h>	//copt_from/to_user
#include <linux/device.h>				
#include <linux/io.h>
#include <linux/ioctl.h>
//define 
#define GPIO_BASE 0X44E07000
#define GPIO_SIZE 0X100
#define GPIO_SET_OFFSET 0x194
#define GPIO_CLR_OFFSET 0X190
#define GPIO_OE  0x134

#define class_name "my_classs"
#define device_name "my_gpioo"
#define mm_number "my_mmm"

//define ioctl
#define GPIO_SET_PIN _IOW('a', 1, int32_t*)
#define GPIO_SET_HIGH _IOW('a', 2, int32_t*)
#define GPIO_SET_LOW _IOW('a', 3, int32_t*)
#define IOCTL_DATA_LEN 1024

//config major, minor number, pointer struct class, cdev(file operation)
dev_t dev = 0;
static struct class *gpio_class;
static struct cdev gpio_cdev;

//Pointer to memory GPIO
static void __iomem *gpio_base;
//gpio pin default
int32_t gpio_pin = -1;

//value from user
int32_t value=0;

//function init/exit				
static int __init gpioex_init(void);
static void __exit gpioex_exit(void);


//function file operation (open, release, read, write)
static int gpio_open(struct inode *inode, struct file *file);
static int gpio_release(struct inode *inode, struct file *file);
static ssize_t gpio_write(struct file *filp, const char *buf, size_t len, loff_t *off);
static ssize_t gpio_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
static long gpio_ioctl(struct file *file, unsigned int cmd, unsigned long arg);


//struct file operation (fops)
static struct file_operations gpio_fops =
{
	.owner = THIS_MODULE,
	.open = gpio_open,
	.write = gpio_write,
	.read = gpio_read,
	.release = gpio_release,
	.unlocked_ioctl = gpio_ioctl,
};


//function will be call when we open device file
static int gpio_open(struct inode *inode, struct file *file)
{
	pr_info("open driver...oke\n");
	return 0;
}

//function will be call when we release device file
static int gpio_release(struct inode *inode, struct file *file)
{
        pr_info("release driver...oke\n");
        return 0;
}
//function will be call when we write device file
static ssize_t gpio_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
	pr_info("write driver\n");
	return 0;
}

//function will be call when we read device file
static ssize_t gpio_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
	pr_info("read driver\n");
	return 0;
}

//function will be call when we using ioctl to read or write 
static long gpio_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    int32_t value;
    uint32_t reg_val;

    if (!gpio_base) {
        printk(KERN_ERR "GPIO base not mapped\n");
        return -EINVAL;
    }

    switch(cmd) {
        case GPIO_SET_PIN:
            if (copy_from_user(&gpio_pin, (int32_t __user *)arg, sizeof(int32_t))) {
                return -EFAULT;
            }
            if (gpio_pin < 0 || gpio_pin >= 32) {
                printk(KERN_ERR "ERR to set gpio_pin: %d!\n", gpio_pin);
                return -EINVAL;
            }
            reg_val = readl(gpio_base + GPIO_OE);
            reg_val &= ~(1 << gpio_pin);
            writel(reg_val, gpio_base + GPIO_OE);
            printk(KERN_INFO "Set gpio %d to OUTPUT, OE: 0x%x\n", gpio_pin, reg_val);
            break;

        case GPIO_SET_HIGH:
            if (gpio_pin < 0) {
                printk(KERN_ERR "GPIO pin not set\n");
                return -EINVAL;
            }
            if (copy_from_user(&value, (int32_t __user *)arg, sizeof(value))) {
                pr_err("Set pin %d high: FALL!!!\n", gpio_pin);
                return -EFAULT;
            }
            if (value == 1) {
                writel(1 << gpio_pin, gpio_base + GPIO_SET_OFFSET);
                pr_info("gpio %d set high, value = %d\n", gpio_pin, value);
            } else {
                pr_info("err set high, value = %d\n", value);
            }
            break;

        case GPIO_SET_LOW:
            if (gpio_pin < 0) {
                printk(KERN_ERR "GPIO pin not set\n");
                return -EINVAL;
            }
            if (copy_from_user(&value, (int32_t __user *)arg, sizeof(value))) {
                pr_err("Set pin %d low: FALL!!!\n", gpio_pin);
                return -EFAULT;
            }
            if (value == 2) { // Đồng bộ với ứng dụng: 2 cho LOW
                writel(1 << gpio_pin, gpio_base + GPIO_CLR_OFFSET);
                pr_info("gpio %d set low, value = %d\n", gpio_pin, value);
            } else {
                pr_info("err set low, value = %d\n", value);
            }
            break;

        default:
            return -EINVAL;
    }
    return 0;
}


//function init
static int __init gpioex_init(void)
{
	//create major minor
	if(alloc_chrdev_region(&dev, 0, 1,mm_number))
	{
		pr_err("Can not locate major and minor number\n");
		return -1; 	//error
	}
	pr_info("MAJOR = %d, MINOR = %d\n", MAJOR(dev), MINOR(dev));
	
	//create cdev struct
	cdev_init(&gpio_cdev, &gpio_fops);

	if(cdev_add(&gpio_cdev, dev, 1) < 0)
	{
		pr_err("Can not add device to systens\n");
		goto err_del;
	}

	gpio_class = class_create(THIS_MODULE, class_name);

	if(IS_ERR(gpio_class))
	{
		pr_err("Can not create struct class\n");
		goto err_class;
	}

	if(IS_ERR(device_create(gpio_class, NULL, dev, NULL, device_name)))
	{
		pr_err("Can not create device\n");
		goto err_device;
	}
	
	gpio_base = ioremap(GPIO_BASE, GPIO_SIZE);
	if(!gpio_base)
	{
		pr_err("Can not mapping to gpio\n");
		device_destroy(gpio_class, dev);
		class_destroy(gpio_class);
		cdev_del(&gpio_cdev);
		unregister_chrdev_region(dev,1);
		return -1;
	}
	return 0;

err_del:
	cdev_del(&gpio_cdev);
err_device:
	class_destroy(gpio_class);
err_class:
	unregister_chrdev_region(dev,1);
	return -1;
}




static void __exit gpioex_exit(void)
{
	iounmap(gpio_base);
	device_destroy(gpio_class, dev);
	class_destroy(gpio_class);
	cdev_del(&gpio_cdev);
	unregister_chrdev_region(dev,1);
	pr_info("clear all");
}



module_init(gpioex_init);
module_exit(gpioex_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");
MODULE_DESCRIPTION("example gpio control using ioctl");
