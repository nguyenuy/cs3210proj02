#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>  // for threads
#include <linux/time.h>
#include <linux/timer.h>

#define PIN 27 //GPIO Pin 7 = GPIO#27
#define DEVICE_NAME "morseThread"
#define BUF_LEN 120      /* Max length of the message from the device */


static struct task_struct *thread1;
static int major;
int var, Device_Open = 0;
static char msg[BUF_LEN];  /* The msg the device will give when asked */
static char *msg_Ptr;

static struct file_operations fops = {
   .read = device_read,
   .write = device_write,
   .open = device_open,
   .release = device_release
};

int thread_fn(void) {

unsigned long j0,j1;
int delay = 60*HZ;
j0 = jiffies;
j1 = j0 + delay;

while (time_before(jiffies, j1)){ 
    if(kthread_should_stop()) {
        do_exit(0);
    }
  schedule();
  var++;
}
return 0;
}

int thread_init (void) {
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
         printk(KERN_ALERT "Registering char device failed with %d\n", major);
         return major;
    }
    
    
   printk(KERN_INFO "I was assigned major number %d. To talk to\n", Major);
   printk(KERN_INFO "the driver, create a dev file with\n");
   printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
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
 var = 0;
 printk(KERN_INFO "in init\n");
 msg_Ptr = kmalloc(4*PAGE_SIZE);
 thread1 = kthread_create(thread_fn,NULL,name);
 if((thread1))
  {
      printk(KERN_INFO "in if\n");
      wake_up_process(thread1);
  }

 return 0;
}



void thread_cleanup(void) {

 unregister_chrdev(major, DEVICE_NAME);
 gpio_free(PIN);
 int ret;
 ret = kthread_stop(thread1);
 if(!ret)
  printk(KERN_INFO "Thread stopped; Var = %d\n",var);

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
  printk(KERN_ALERT "Sorry, this operation isn't supported.\n");
  return -EINVAL;
}


MODULE_LICENSE("GPL"); 
module_init(thread_init);
module_exit(thread_cleanup);