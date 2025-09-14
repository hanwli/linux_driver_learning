#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio/consumer.h>
#define IO_LED 21
#define IO_BUTTON 20
#define IO_OFFSET 512

static struct gpio_desc *led, *button;
// set static to limit this function's visibility
// You can't invoke this function from outside
static int __init my_init(void) 
{
    // get the GPIO descriptor of the LED pin
    int status;
    led = gpio_to_desc(IO_LED + IO_OFFSET);
    if(!led){
        printk("gpioctrl - Error getting pin %d\n", IO_LED);
        return -ENODEV;
    }

    // get the GPIO descriptor of the BUTTON pin
    button = gpio_to_desc(IO_BUTTON + IO_OFFSET);
    if(!button){
        printk("gpioctrl - Error getting pin %d\n", IO_BUTTON);
        return -ENODEV;
    }

    // set the direction of the LED pin to output
    // and set the direction of the BUTTON pin to input
    status = gpiod_direction_output(led, 0);
    if(status){
        printk("gpioctrl - Error setting pin %d to output\n", IO_LED);
        return status;
    }

    status = gpiod_direction_input(button);
    if(status){
        printk("gpioctrl - Error setting pin %d to input\n", IO_BUTTON);
        return status;
    }

    // set the LED on
    gpiod_set_value(led, 1);
    printk("gpioctrl - Button is %spressed\n", gpiod_get_value(button) ? "" : "not ");
    return 0;
}

static void __exit my_exit(void) 
{
    gpiod_set_value(led, 0);
    printk("gpioctrl - Goodbye, cruel world\n");
}

module_init(my_init);
module_exit(my_exit);

// metadata of this module
MODULE_LICENSE("GPL");
MODULE_AUTHOR("hanwli 4 Linux");
MODULE_DESCRIPTION("A GPIO module");