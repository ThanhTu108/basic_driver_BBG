#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/cdev.h>         //cdev_struct
#include <linux/kdev_t.h>       //major/ minor number
#include <linux/fs.h>           //file_operation
#include <linux/err.h>
#include <linux/slab.h>         //kmalloc()
#include <linux/uaccess.h>      //copt_from/to_user
#include <linux/device.h>
#include <linux/io.h>
#include <linux/ioctl.h>
#include <linux/of.h>
#include <linux/platform_device.h>

//define register 
#define GPIO_OE  0x134
#define GPIO_DATAOUT 0x13C

//define major, minor, struct class, cdev(file operation)
dev_t dev_id = 0;
static struct class *gpio_class;
static struct cdev gpio_cdev;

//config gpio using ioctl userspace c
struct gpio_config {
    int bank;  // GPIO bank (0-3)
    int num;   // GPIO number 0 to 31
};
struct gpio_config config;

//define name
#define class_name "gpio_class"
#define device_name "gpio_device"
#define mm_num "gpio_num"
static int gpio_pin = -1;
static int current_bank = -1;
static void __iomem *gpio_base = NULL;

static u32 gpio_base_address[4];

//define ioctl
#define GPIO_NUMBER_MAGIC 'G'
#define GPIO_SET_PIN _IOW(GPIO_NUMBER_MAGIC, 1, struct gpio_config)
#define GPIO_SET_VALUE _IOW(GPIO_NUMBER_MAGIC, 2, int)

//function file operation (open, release, read, write), ioctl
static int gpio_open(struct inode *inode, struct file *file);
static int gpio_release(struct inode *inode, struct file*file);
static ssize_t gpio_write(struct file *filp, const char *buf, size_t len, loff_t *off);
static ssize_t gpio_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
static long gpio_ioctl(struct file *file, unsigned int cmd, unsigned long arg);



//file oeration (fops)
static struct file_operations gpio_fops=
{
	.owner = THIS_MODULE,
	.open = gpio_open,
	.read = gpio_read,
	.write = gpio_write,
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
	if(gpio_pin >= 0)
	{
		gpio_pin = -1;
		current_bank = -1;
	}
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
static long gpio_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int value;
	uint32_t reg_val;
	switch(cmd)
	{
		case GPIO_SET_PIN:
			if(copy_from_user(&config, (struct gpio_config *)arg, sizeof(config)))
			{
				return -EFAULT;
			}

			if(config.bank <0 || config.bank > 3 || config.num <0 || config.num >31)
			{
				pr_err("GPIO: Invalid GPIO%d_%d\n", config.bank, config.num);
				return -EINVAL;
			}

			gpio_pin = (config.bank *32)+ config.num;
			current_bank = config.bank;

			reg_val = ioread32(gpio_base + GPIO_OE);
			reg_val &= ~(1 << config.num);
			iowrite32(reg_val, gpio_base + GPIO_OE);
	
			pr_info("GPIO_EX: Selected GPIO%d_%d (pin %d), Base Address: 0x%lx\n", config.bank, config.num, gpio_pin, (unsigned long)(gpio_base - (void *)gpio_base + gpio_base_address[current_bank]));
            		break;


		case GPIO_SET_VALUE:
			if(copy_from_user(&value, (int*) arg, sizeof(value)))
			{
				return -EFAULT;
			}

			if (gpio_pin < 0 || current_bank < 0) 
			{
               	 		pr_err("GPIO_EX: No GPIO selected\n");
                		return -EINVAL;
            		}
			if(value !=0 && value !=1)
			{
				pr_err("Invalid value %d \n", value);
				return -EINVAL;
			}

			reg_val = ioread32(gpio_base + GPIO_DATAOUT);
			if(value)
				reg_val |= (1 << (gpio_pin % 32));	//gpio_pin chia duw cho 32 de lay gia tri gpio pin trong bank, phep or giup giu nguyen lai gia tri chan khac
			else 
				reg_val &= ~(1 << (gpio_pin % 32));
			iowrite32(reg_val, gpio_base + GPIO_DATAOUT);
			printk(KERN_INFO "GPIO_EX: Set GPIO%d_%d (pin %d) to %s\n", current_bank, gpio_pin % 32, gpio_pin, value ? "HIGH" : "LOW");
            		break;

        	default:
            		return -ENOTTY;		
	}
	return 0;
	
}


static int gpio_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = pdev->dev.of_node;
	int len;

	if (of_property_read_u32_array(np, "gpio_base_address", gpio_base_address, 4)) 
	{
        	dev_err(dev, "Failed to get gpio_base_address from DT\n");
        	return -EINVAL;
    	}

    	gpio_base = ioremap(gpio_base_address[0], 0x1000);
    	if (!gpio_base) 
	{
        	dev_err(dev, "Failed to map GPIO base address\n");
        	return -ENOMEM;
   	}

	if(alloc_chrdev_region(&dev_id, 0, 1, mm_num))
	{
		pr_err("can not allocate major and minor number\n");
		return -EFAULT;
	}
	
	pr_info("MAJOR =%d, MINOR = %d", MAJOR(dev_id), MINOR(dev_id));

	//create cdev_struct
	
	cdev_init(&gpio_cdev, &gpio_fops);
	if(cdev_add(&gpio_cdev, dev_id, 1) < 0)
	{
		pr_err("can not add cdev struct to the system!\n");
		goto err_del;
	}
	
	gpio_class = class_create(THIS_MODULE, class_name);
	
	if(IS_ERR(gpio_class))
	{
		pr_err("can not create struct class\n");
		goto err_class;
	}

	if(IS_ERR(device_create(gpio_class, NULL, dev_id, NULL, device_name)))
        {
                pr_err("can not create device\n");
                goto err_device;
        }
	dev_info(dev, "GPIO driver loaded, Base Addresses: 0x%lx, 0x%lx, 0x%lx, 0x%lx\n",
             gpio_base_address[0], gpio_base_address[1], gpio_base_address[2], gpio_base_address[3]);
   	return 0;

err_del:
        cdev_del(&gpio_cdev);
	iounmap(gpio_base);
err_device:
        class_destroy(gpio_class);
	iounmap(gpio_base);
err_class:
        unregister_chrdev_region(dev,1);
	iounmap(gpio_base);
        return -1;
}	

static int gpio_remove(struct platform_device *pdev)
{
	if(gpio_pin >= 0)
	{
		gpio_pin = -1;
		current_bank = -1;
	}

	if(gpio_base)
	{
		iounmap(gpio_base);
	}

	device_destroy(gpio_class, dev_id);
        class_destroy(gpio_class);
        cdev_del(&gpio_cdev);
        unregister_chrdev_region(dev_id,1);
	pr_info("clear all");
	return 0;
}


static const struct of_device_id gpio_id_ids[] ={
{.compatible = "gpio-example"},
{}
};


MODULE_DEVICE_TABLE(of, gpio_id_ids);

static struct platform_driver gpio_driver_dt =
{
	.probe = gpio_probe,
	.remove = gpio_remove,
	.driver = {
		.name = "gpio-example",
		.of_match_table = gpio_id_ids,
	},
};

module_platform_driver(gpio_driver_dt);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");










