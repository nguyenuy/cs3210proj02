#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>

/*
 * Struct defining pins, direction and inital state 
 */
static struct gpio leds[] = {
		{  3, GPIOF_OUT_INIT_HIGH, "LED 1" },
};

/*
 * Module init function
 */
static int __init morse_init(void)
{
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
static void __exit morse_exit(void)
{
	int i;

	printk(KERN_INFO "%s\n", __func__);

	// turn all LEDs off
	for(i = 0; i < ARRAY_SIZE(leds); i++) {
		gpio_set_value(leds[i].gpio, 0); 
	}
	
	// unregister all GPIOs
	gpio_free_array(leds, ARRAY_SIZE(leds));
}

MODULE_LICENSE("GPL");

module_init(morse_init);
module_exit(morse_exit);
