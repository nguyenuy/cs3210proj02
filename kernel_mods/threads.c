#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>  // for threads
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>

#define PIN 27 //GPIO Pin 7 = GPIO#27
#define DEVICE_NAME "morseThread"
#define BUF_LEN 120      /* Max length of the message from the device */
#define SUCCESS 0


static struct task_struct *thread1;
static int major;
int len = 0, Device_Open = 0, key = 0;
char currentLetter = 0;
static char msg[BUF_LEN];  /* The msg the device will give when asked */
static char *msg_Ptr;
char morseMap[512];


static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations fops = {
   .read = device_read,
   .write = device_write,
   .open = device_open,
   .release = device_release
};

void init(int* A, int n){
  int i=0;
  for(; i<n; i++){
    A[i] = -2;
  }
}

int getDecodeKey(int* buf,int n){
  int ans = 1;
  int i=0;
  for(; i<n && buf[i] >= 0; i++){
    ans = (ans<<1) + buf[i];
  }
  
  return ans;
}

void initMorseMap(char* binToChar){
  binToChar[0b110] = 'a';
  binToChar[0b10111] = 'b';
  binToChar[0b10101] = 'c';
  binToChar[0b1011] = 'd';
  binToChar[0b11] = 'e';
  binToChar[0b11101] = 'f';
  binToChar[0b1001] = 'g';
  binToChar[0b11111] = 'h';
  binToChar[0b111] = 'i';
  binToChar[0b11000] = 'j';
  binToChar[0b1010] = 'k';
  binToChar[0b11011] = 'l';
  binToChar[0b100] = 'm';
  binToChar[0b101] = 'n';
  binToChar[0b1000] = 'o';
  binToChar[0b11001] = 'p';
  binToChar[0b10010] = 'q';
  binToChar[0b1101] = 'r';
  binToChar[0b1111] = 's';
  binToChar[0b10] = 't';
  binToChar[0b1110] = 'u';
  binToChar[0b11110] = 'v';
  binToChar[0b1100] = 'w';
  binToChar[0b10110] = 'x';
  binToChar[0b10100] = 'y';
  binToChar[0b10011] = 'z';
  binToChar[0b110000] = '1';
  binToChar[0b111000] = '2';
  binToChar[0b111100] = '3';
  binToChar[0b111110] = '4';
  binToChar[0b111111] = '5';
  binToChar[0b101111] = '6';
  binToChar[0b100111] = '7';
  binToChar[0b100011] = '8';
  binToChar[0b100001] = '9';
  binToChar[0b100000] = '0';
  binToChar[0b1101010] = '.';
  binToChar[0b1110011] = '?';
  binToChar[0b1010100] = '!';
  binToChar[0b101001] = '(';
  binToChar[0b1010011] = ')';
  binToChar[0b1000111] = ':';
  binToChar[0b101110] = '=';
  binToChar[0b1011110] = '-';
  binToChar[0b1101101] = '"';
  binToChar[0b1001100] = ',';
  binToChar[0b1100001] = '\'';
  binToChar[0b101101] = '/';
  binToChar[0b1010101] = ';';
  binToChar[0b1110010] = '_';
  binToChar[0b1100101] = '@';
  binToChar[0b11111111]= ' ';
}

int thread_fn(void) {

    unsigned long j0,j1;
    int delay = 600*HZ;
    j0 = jiffies;
    j1 = j0 + delay;

    while (1){ 
        if(kthread_should_stop()) {
            do_exit(0);
        }
      schedule();
      
      if(gpio_get_value_cansleep(PIN) == 1) {
        printk(KERN_INFO "Received a key press!");
        const int LONG_WAIT = 100;
        const int unit = 4;
        const int MAX_DIT = 15;
        int buf[16];
        init(buf, 16);
        int cnt = 0;
            while(1) {
                int cnt1 = 0, cnt0 = 0;
                while(gpio_get_value_cansleep(PIN) == 1) {
                    cnt1++;
                    msleep(unit);
                }
                while(gpio_get_value_cansleep(PIN) == 0) {
                    cnt0++;
                    if(cnt0 > LONG_WAIT) {
                        break;
                    } else {
                        msleep(unit);
                    }
                }
                if(cnt0 > LONG_WAIT) {
                    buf[cnt++] = (cnt1 > MAX_DIT?0:1);
                    break;
                } else {
                    buf[cnt++] = (cnt1 > MAX_DIT?0:1);
                }
            
            }
            key = getDecodeKey(buf, 16);
            currentLetter = morseMap[key];
            *(msg_Ptr++) = currentLetter;
            len++;
        }
    }
    return 0;
}

int thread_init (void) {
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
         printk(KERN_ALERT "Registering char device failed with %d\n", major);
         return major;
    }
    
    
   printk(KERN_INFO "I was assigned major number %d. To talk to\n", major);
   printk(KERN_INFO "the driver, create a dev file with\n");
   printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, major);
   printk(KERN_INFO "Try various minor numbers. Try to cat and echo to\n");
   printk(KERN_INFO "the device file.\n");
   printk(KERN_INFO "Remove the device file and module when done.\n");


 int g;
 g = gpio_request(PIN, "morse");
 if(g != 0) {
    return -EINVAL;
 }
 gpio_direction_input(PIN);
 char name[8]="thread1";
 printk(KERN_INFO "in init\n");
 msg_Ptr = msg;
 if(!msg_Ptr) {
    return -ENOMEM;
 }
 initMorseMap(morseMap);
 thread1 = kthread_create(thread_fn,NULL,name);
 if((thread1))
  {
      printk(KERN_INFO "in if\n");
      wake_up_process(thread1);
  }

 return 0;
}



void thread_cleanup(void) {
 int ret;
 gpio_free(PIN);
 ret = kthread_stop(thread1);
 unregister_chrdev(major, DEVICE_NAME);
 if(!ret)
  printk(KERN_INFO "Thread stopped; Key = %d, Current Letter = %c\n", key, currentLetter);

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
   sprintf(msg, "Keycode is: %d, Current Letter = %c\n", key, currentLetter);
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
   while (len && *msg_Ptr) {

      /* 
       * The buffer is in the user data segment, not the kernel 
       * segment so "*" assignment won't work.  We have to use 
       * put_user which copies data from the kernel data segment to
       * the user data segment. 
       */
      put_user(*(msg_Ptr++), buffer++);

      len--;
      bytes_read++;
   }

   /* 
    * Most read functions return the number of bytes put into the buffer
    */
   len = 0;
   msg_Ptr = msg;
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


MODULE_LICENSE("GPL"); 
module_init(thread_init);
module_exit(thread_cleanup);