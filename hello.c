#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

int a1;
EXPORT_SYMBOL_GPL(a1);

static int __init hello_world_init(void)
{
    a1 = 10;
    printk(KERN_ALERT "HELLO_INIT: a1 = %d\n", a1);  // Thêm \n và đổi chuỗi cho rõ
    return 0;
}

static void __exit goodbye_world(void)
{
    printk(KERN_ALERT "GOODBYE: bye world, a1 = %d\n", a1);  // Thêm thông tin a1
}

module_init(hello_world_init);
module_exit(goodbye_world);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("THANH TU");
MODULE_DESCRIPTION("first driver");
