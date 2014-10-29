#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>

/*
 *  GPIO Definitions 
 */
#define GP_LED                 (3) // GPIO3 is GP LED - LED connected between Cypress CY8C9540A and RTC battery header
#define GP_5                   (17) //GPIO17 corresponds to Arduino PIN5
#define GP_6                   (24) //GPIO24 corresponds to Arduino PIN6
#define GP_7                   (27) //GPIO27 corresponds to Arduino PIN7

#define SUCCESS 0
#define DEVICE_NAME "chardev"	/* Dev name as it appears in /proc/devices   */
#define BUF_LEN 80		/* Max length of the message from the device */

 /* 
 * Global variables are declared as static, so are global within the file. 
 */

static int Major;		/* Major number assigned to our device driver */
static int Device_Open = 0;	/* Is device open?  
				 * Used to prevent multiple access to device */
static char msg[BUF_LEN];	/* The msg the device will give when asked */
static char *msg_Ptr;

/*
 * Struct defining pins, direction and inital state 
 */
static struct gpio leds[] = {
		{  GP_LED, GPIOF_OUT_INIT_HIGH, "LED 1" },
};

/*
 * Module init function
 */
static int __init gpiomod_init(void)
{
	Major = register_chrdev(0, DEVICE_NAME, &fops);
	if (Major < 0) {
	  printk(KERN_ALERT "Registering char device failed with %d\n", Major);
	  return Major;
	}
	
	int ret = 0;

	printk(KERN_INFO "%s\n", __func__);

	// register LED GPIOs, turn LEDs on
	ret = gpio_request_array(leds, ARRAY_SIZE(leds));

	if (ret) {
		printk(KERN_ERR "Unable to request GPIOs: %d\n", ret);
	}
	gpio_set_value(leds[0].gpio, 1);

	return ret;
}

/*
 * Module exit function
 */
static void __exit gpiomod_exit(void)
{
	int i;

	printk(KERN_INFO "%s\n", __func__);
	unregister_chrdev(Major, DEVICE_NAME);
	// turn all LEDs off
	for(i = 0; i < ARRAY_SIZE(leds); i++) {
		gpio_set_value(leds[i].gpio, 0); 
	}
	
	// unregister all GPIOs
	gpio_free_array(leds, ARRAY_SIZE(leds));
}

MODULE_LICENSE("GPL");

module_init(gpiomod_init);
module_exit(gpiomod_exit);
