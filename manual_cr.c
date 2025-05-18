#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>


dev_t dev =0;

static int __init man_create(void)
{
	if((alloc_chrdev_region(&dev, 0, 1, "TTu"))<0)
	{
		pr_err("can not allocate major and minor number\n");
		return -1;
	}
	pr_info("sucessfully\n");
	return 0;
}

static void __exit exit_create(void)
{
	unregister_chrdev_region(dev ,1);
	pr_info("rm sucessful");
}

module_init(man_create);
module_exit(exit_create);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");
