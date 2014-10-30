#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <linux/vmalloc.h>

/*
 *  GPIO Definitions 
 */
#define GP_LED                 (3) // GPIO3 is GP LED - LED connected between Cypress CY8C9540A and RTC battery header
#define GP_5                   (17) //GPIO17 corresponds to Arduino PIN5
#define GP_6                   (24) //GPIO24 corresponds to Arduino PIN6
#define GP_7                   (27) //GPIO27 corresponds to Arduino PIN7

#define SUCCESS 0
#define DEVICE_NAME "morseEncode"   /* Dev name as it appears in /proc/devices   */
#define BUF_LEN 80      /* Max length of the message from the device */

#define CHAR_SIZE 256
#define MORSE_BIN 16



/*
 * Struct defining pins, direction and inital state 
 */
static struct gpio morse_gpio[] = {
      {  GP_LED, GPIOF_OUT_INIT_LOW, "LED" },
};

/*
 * GLobal variables undeclared as static
*/
typedef __u16 morse_t;
int len,temp;
morse_t charToBin[CHAR_SIZE];
char *morse_msg;
char *english_msg;

/* 
 * Global variables are declared as static, so are global within the file. 
 */

static int Major;    /* Major number assigned to our device driver */
static int Device_Open = 0;   /* Is device open?  
             * Used to prevent multiple access to device */
static char msg[BUF_LEN];  /* The msg the device will give when asked */
static char *msg_Ptr;

/*  
 *  Function Prototypes - this would normally go in a .h file
 */
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

void initialize_char_to_bin_array();
int string_to_morse(char *buf, int length);
void flashLED(int seconds);

static struct file_operations fops = {
   .read = device_read,
   .write = device_write,
   .open = device_open,
   .release = device_release
};


/*
 * Module init function
 */
static int __init morse_init(void)
{
   printk(KERN_INFO "%s\n", __func__);

   //Init the file
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

   //register the GPIOs
   int ret = 0;
   ret = gpio_request_array(morse_gpio, ARRAY_SIZE(morse_gpio));

   if (ret) {
      printk(KERN_ERR "Unable to request GPIOs: %d\n", ret);
   }

   //Initialize morse arrays
   initialize_char_to_bin_array();

   //Allocate memory for character points
   english_msg = vmalloc(PAGE_SIZE*4);

   //TO DELETE: TURN LED ON
   //gpio_set_value(morse_gpio[0].gpio, 1);

   return ret;
}

/*
 * Module exit function
 */
static void __exit morse_exit(void)
{
   printk(KERN_INFO "%s\n", __func__);

   //Unregister device module in fs
   unregister_chrdev(Major, DEVICE_NAME);

   //Unregister all GPIOs
   //TO DELETE: TURN LED OFF
   //gpio_set_value(morse_gpio[0].gpio, 0); 
   
   
   // unregister all GPIOs
   gpio_free_array(morse_gpio, ARRAY_SIZE(morse_gpio));
}

/* 
 * Called when a process tries to open the device file, like
 * "cat /dev/morse"
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
   Device_Open--;    /* We're now ready for our next caller */

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
static ssize_t device_read(struct file *filp,   /* see include/linux/fs.h   */
            char *buffer,  /* buffer to fill with data */
            size_t length, /* length of the buffer     */
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
  //printk(KERN_ALERT "Sorry, this operation isn't supported.\n");
  //return -EINVAL;
  size_t copy_size = len*sizeof(char)*27;
  copy_from_user(english_msg, buff,copy_size);
  printk(KERN_INFO "English Message: %s\n", english_msg);
  //string_to_morse(english_msg, copy_size);
  //printk(KERN_INFO "Translated Message: %s\n", english_msg);


  
  return 0;
}

/*******************************
 * Morse encoding functions here
*******************************/
void initialize_char_to_bin_array() {
int i=0;
  for(; i<CHAR_SIZE;i++){
    charToBin[i] = 0;
  }
  charToBin['a'] = 0b110;
  charToBin['b'] = 0b10111;
  charToBin['c'] = 0b10101;
  charToBin['d'] = 0b1011;
  charToBin['e'] = 0b11;
  charToBin['f'] = 0b11101;
  charToBin['g'] = 0b1001;
  charToBin['h'] = 0b11111;
  charToBin['i'] = 0b111;
  charToBin['j'] = 0b11000;
  charToBin['k'] = 0b1010;
  charToBin['l'] = 0b11011;
  charToBin['m'] = 0b100;
  charToBin['n'] = 0b101;
  charToBin['o'] = 0b1000;
  charToBin['p'] = 0b11001;
  charToBin['q'] = 0b10010;
  charToBin['r'] = 0b1101;
  charToBin['s'] = 0b1111;
  charToBin['t'] = 0b10;
  charToBin['u'] = 0b1110;
  charToBin['v'] = 0b11110;
  charToBin['w'] = 0b1100;
  charToBin['x'] = 0b10110;
  charToBin['y'] = 0b10100;
  charToBin['z'] = 0b10011;
  charToBin['1'] = 0b110000;
  charToBin['2'] = 0b111000;
  charToBin['3'] = 0b111100;
  charToBin['4'] = 0b111110;
  charToBin['5'] = 0b111111;
  charToBin['6'] = 0b101111;
  charToBin['7'] = 0b100111;
  charToBin['8'] = 0b100011;
  charToBin['9'] = 0b100001;
  charToBin['0'] = 0b100000;
  charToBin['.'] = 0b1101010;
  charToBin['?'] = 0b1110011;
  charToBin['!'] = 0b1010100;
  charToBin['('] = 0b101001;
  charToBin[')'] = 0b1010011;
  charToBin[':'] = 0b1000111;
  charToBin['='] = 0b101110;
  charToBin['-'] = 0b1011110;
  charToBin['"'] = 0b1101101;
  charToBin[','] = 0b1001100;
  charToBin['\'']= 0b1100001;
  charToBin['/'] = 0b101101;
  charToBin[';'] = 0b1010101;
  charToBin['_'] = 0b1110010;
  charToBin['@'] = 0b1100101;
  charToBin[' '] = 0b11111111;
  
  char diff = 'A'-'a';
  char ch = 'A';
  for(; ch<='Z'; ch++){
    char lower = ch - diff;
    charToBin[ch] = charToBin[lower];
  }   
}

int string_to_morse(char *buf, int length) {
  char* translated = vmalloc(length);
  char* p = translated;
  int i = 0;
  for(; i<len;i++){
    char ch = *(buf+i);
    if(ch == ' ') {
      *(p++) = 'S';
      continue;
    }

    int shift = MORSE_BIN-1;
    morse_t bin = charToBin[ch];
    for (; shift>=0; shift--) {
      morse_t mask = 1<<shift;
      if((mask & bin) != 0) {
        break;
      }
    }
    shift--;
    for (; shift>0; shift--) {
      morse_t mask = 1 << shift;
      if((mask & bin) !=0 ) {
        *(p++) = 'i';
        //transLength++;
      } else {
        *(p++) = 'a';
        //transLength++;
      }
      *(p++) = (shift==0?' ':'-');
      //transLength++
    }
  }
  *(p) = '\0';
  memcpy(english_msg, translated, length);
  vfree(translated);
  return 0;
}
void flashLED(int seconds) {
  gpio_set_value(morse_gpio[0].gpio, 1);
  ssleep(seconds);
  gpio_set_value(morse_gpio[0].gpio, 0);
}

MODULE_LICENSE("GPL");

module_init(morse_init);
module_exit(morse_exit);
