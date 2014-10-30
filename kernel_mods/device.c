/*
 *  chardev.c: Creates a read-only char device that says how many times
 *  you've read from the dev file
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>	/* for put_user */
#include <linux/gpio.h>

/*
 *  GPIO Definitions 
 */
#define GP_LED                 (3) // GPIO3 is GP LED - LED connected between Cypress CY8C9540A and RTC battery header
#define GP_5                   (17) //GPIO17 corresponds to Arduino PIN5
#define GP_6                   (24) //GPIO24 corresponds to Arduino PIN6
#define GP_7                   (27) //GPIO27 corresponds to Arduino PIN7
#define BLINK_TIME_SEC         (1) // 1 seconds blink time
#define GPIO_DIRECTION_IN      (1)
#define GPIO_DIRECTION_OUT     (0)
#define ERROR                  (-1)

//Define GPIOs for LEDs
static struct gpio leds[] = {
	{  GP_LED, GPIOF_OUT_INIT_LOW, "LED" },
};


/*  
 *  Prototypes - this would normally go in a .h file
 */
int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

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

static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};

irqreturn_t change_state_interrupt(int irq, void *dev_id, struct pt_regs *regs){
  printk(KERN_ALERT "Interrupt\n");
  gpio_set_value(led_gpio, 0);
  sleep(5);
  gpio_set_value(led_gpio, 1);
  return IRQ_HANDLED;
}

/*
 * This function is called when the module is loaded
 */
int init_module(void)
{
    Major = register_chrdev(0, DEVICE_NAME, &fops);

	if (Major < 0) {
	  printk(KERN_ALERT "Registering char device failed with %d\n", Major);
	  return Major;
	}

	printk(KERN_INFO "I was assigned major number %d. To talk to\n", Major);
	printk(KERN_INFO "the driver, create a dev file with\n");
	printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
	printk(KERN_INFO "Try various minor numbers. Try to cat and echo to\n");
	printk(KERN_INFO "the device file.\n");
	printk(KERN_INFO "Remove the device file and module when done.\n");

	int led_gpio=3;
	if (!gpio_is_valid(led_gpio)){
	  printk(KERN_INFO "The requested GPIO is not available \n");
	  return SUCCESS;
	} else {
	  printk(KERN_INFO "The requested GPIO is available \n");	
	}
	gpio_set_value(led_gpio, 1);

	if(gpio_request(GP_7,"GP_7")){
	  printk(KERN_ALERT "can't set interrupt of GP 7\n");
	  return -1;
	}

	if((dir_err = gpio_direction_input(GP_7)) < 0){
	  printk(KERN_ALTERT "cannot set GP 7 as input");
	  return -1;
	}

	/*
	 * the interrupts
	 */

	if((irq_line = gpio_to_irq(GP_7)) < 0){
	  printk(KERN_ALERT 'GP 7 cannot be used as interrupt\n');
	  return -1;
	}

	if((irq_req_res = request_irq(irq_line,change_state_interrupt, IRQF_TRIGGER_FALLING, "gpio_change_state", NULL)) < 0){
	  printk(KERN_ALERT 'cannot install GP 7\n');
	}
	

	return SUCCESS;
}

/*
 * This function is called when the module is unloaded
 */
void cleanup_module()
{
	/* 
	 * Unregister the device 
	 */
	unregister_chrdev(Major, DEVICE_NAME);
	//if (ret < 0) { printk(KERN_ALERT "Error in unregister_chrdev: %d\n", ret); }
}

/*
 * Methods
 */

/* 
 * Called when a process tries to open the device file, like
 * "cat /dev/mycharfile"
 */
static int device_open(struct inode *inode, struct file *file)
{
	static int counter = 0;

	if (Device_Open)
		return -EBUSY;

	Device_Open++;
	sprintf(msg, "I already told you %d times Hello world!\n", counter++);
	msg_Ptr = msg;
	try_module_get(THIS_MODULE);

	return SUCCESS;
}

/* 
 * Called when a process closes the device file.
 */
static int device_release(struct inode *inode, struct file *file)
{
	Device_Open--;		/* We're now ready for our next caller */

	/* 
	 * Decrement the usage count, or else once you opened the file, you'll
	 * never get get rid of the module. 
	 */
	module_put(THIS_MODULE);

	return 0;
}

/* 
 * Called when a process, which already opened the dev file, attempts to
 * read from it.
 */
static ssize_t device_read(struct file *filp,	/* see include/linux/fs.h   */
			   char *buffer,	/* buffer to fill with data */
			   size_t length,	/* length of the buffer     */
			   loff_t * offset)
{
	/*
	 * Number of bytes actually written to the buffer 
	 */
	int bytes_read = 0;

	/*
	 * If we're at the end of the message, 
	 * return 0 signifying end of file 
	 */
	if (*msg_Ptr == 0)
		return 0;

	/* 
	 * Actually put the data into the buffer 
	 */
	while (length && *msg_Ptr) {

		/* 
		 * The buffer is in the user data segment, not the kernel 
		 * segment so "*" assignment won't work.  We have to use 
		 * put_user which copies data from the kernel data segment to
		 * the user data segment. 
		 */
		put_user(*(msg_Ptr++), buffer++);

		length--;
		bytes_read++;
	}

	/* 
	 * Most read functions return the number of bytes put into the buffer
	 */
	return bytes_read;
}

/*  
 * Called when a process writes to dev file: echo "hi" > /dev/hello 
 */
static ssize_t
device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	printk(KERN_ALERT "Sorry, this operation isn't supported.\n");
	return -EINVAL;
}
