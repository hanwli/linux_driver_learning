#include <linux/module.h>
#include <linux/init.h>

// set static to limit this function's visibility
// You can't invoke this function from outside
static int __init my_init(void) 
{
    printk("hello - Hello, Kernel!\n");
    return 0;
}

static void __exit my_exit(void) 
{
    printk("hello - Goodbye, Kernel!\n");
}

module_init(my_init);
module_exit(my_exit);

// metadata of this module
MODULE_LICENSE("GPL");
MODULE_AUTHOR("hanwli 4 Linux");
MODULE_DESCRIPTION("A better hello world module");