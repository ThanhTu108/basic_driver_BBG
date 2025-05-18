#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/err.h>



dev_t dev = 0;
static struct class *dev_class;

static int __init auto_cre(void)
{
	if((alloc_chrdev_region(&dev, 0, 1, "TT"))<0)
	{
		pr_err("can not allocate major, minor number\n");
		return -1;
	}
	pr_info("MAJOR = %d, MINOR = %d \n", MAJOR(dev), MINOR(dev));
	
	dev_class = class_create(THIS_MODULE, "ex_class");
	if(IS_ERR(dev_class))
	{
		pr_err("can not create struct class\n");
		goto r_class;
	}

	if(IS_ERR(device_create(dev_class, NULL, dev, NULL, "ex_device")))
	{
		pr_err("can not create device file\n");
		goto r_device;
	}
	pr_info("sucessfully\n");
	return 0;

r_device:
	class_destroy(dev_class);
r_class:
	unregister_chrdev_region(dev, 1);
	return -1;
}


static void __exit exit_auto_cr(void)
{
	device_destroy(dev_class, dev);
	class_destroy(dev_class);
	unregister_chrdev_region(dev, 1);
	pr_info("removed everything sucessfully\n");
}

module_init(auto_cre);
module_exit(exit_auto_cr);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");

