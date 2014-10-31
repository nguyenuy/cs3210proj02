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
#define LED 3 //GPIO for galileo LED = 3
#define DEVICE_NAME "morseThread"
#define BUF_LEN 120      /* Max length of the message from the device */
#define SUCCESS 0
#define CHAR_SIZE 256


static struct task_struct *thread1;
static int major;
int len = 0, Device_Open = 0, key = 0;
char currentLetter = 0;
static char msg[BUF_LEN];  /* The msg the device will give when asked */
static char *msg_Ptr;
char morseMap[512];
short charToBin[CHAR_SIZE];
char translated[1024];
char flashStr[1024];
int transLength = 0;
int flashLength = 0;
char english[256];


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

void initialize(){
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

int flash(int gpio, int milliseconds) {
    gpio_set_value(gpio, 1);
    msleep(500 * milliseconds);
    gpio_set_value(gpio, 0);
    return 0;
}

int sleepLED(int milliseconds) {
    msleep(50 * milliseconds);
    return 0;
}

int string_to_morse(char *buf, int length) {

    int i=0;
    char *p = translated;
    for(; i<length; i++){
        char ch = *(buf+i);
        if(ch == ' ') {
                *(p++) = 'S';
                continue;
        }
        int shift = 15;
        int bin = charToBin[ch];
        for( ; shift>=0; shift--){
          short mask = 1<<shift;
          if((mask & bin) != 0)
            break;
        }

    shift--;

    for(; shift>=0; shift--){
      short mask = 1<<shift;
      if((mask & bin) != 0){
            //*(p++) = 'd';
            *(p++) = 'i';
            transLength++;
            //*(p++) = 't';
          } else {
            //*(p++) = 'd';
            *(p++) = 'a';
            transLength++;
            //*(p++) = 'h';
        }
      *(p++) = (shift==0?' ':'-');
      transLength++;
        }
    }

    *(p) = '\0';

    return 0;
}

void createFlashString() {
        int i=0;
        char *p = flashStr;
        for(i; i<transLength; i++) {
                char currentC = *(translated+i);
                char next = *(translated+i+1);
                
                if(currentC == 'i') {
                        *(p++) = 'f';
                        *(p++) = 1;
                        flashLength += 2;
                } else if (currentC == 'a') {
                        *(p++) = 'f';
                        *(p++) = 3;
                        flashLength += 2;
                } else if (currentC == ' ') {
                        if(next == 'S') {
                                *(p++) = 'w';
                                *(p++) = 7;
                        } else {
                                *(p++) = 'w';
                                *(p++) = 3;
                        }
                        flashLength += 2;
                } else if(currentC == '-') {
                        *(p++) = 'w';
                        *(p++) = 1;
                        flashLength += 2;
                }
                
                
        }
}

void flashLED(int gpio) {
        int i=0;
        for(i; i<flashLength; i+=2) {
                char currentC = *(flashStr+i);
                char next = *(flashStr+i+1);
                
                if(currentC == 'f') {
                        flash(gpio, next);
                } else if(currentC == 'w') {
                        sleepLED(next);
                        
                }
        }
}


int thread_fn(void) {

    while (1){ 
        if(kthread_should_stop()) {
            do_exit(0);
        }
      schedule();
      
      if(gpio_get_value_cansleep(PIN) == 1) {
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
 g = gpio_request(LED, "led");
 gpio_direction_input(PIN);
 gpio_direction_output(LED, 0);
 char name[8]="thread1";
 msg_Ptr = msg;
 int i=0;
 for(; i<BUF_LEN; i++) {
    *(msg_Ptr + i) = 0;
 }
 if(!msg_Ptr) {
    return -ENOMEM;
 }
 initMorseMap(morseMap);
 initialize();
 thread1 = kthread_create(thread_fn,NULL,name);
 if((thread1))
  {
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
  printk(KERN_INFO "Thread stopped", key, currentLetter, len);

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
   msg_Ptr = msg;
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
   int i = 0;
   for(; i<BUF_LEN; i++) {
        *(msg_Ptr + i) = 0;
   }
   return bytes_read;
}

/*  
 * Called when a process writes to dev file: echo "hi" > /dev/hello 
 */
static ssize_t
device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
    int msglength, i=0;
	for(; i<len && i < 32; i++) {
		get_user(english[i], buff + i);
	}
    transLength = 0;
    string_to_morse(english, i);
    createFlashString();
    flashLED(3);
    
    int j=0;
    for(; j<1024; j++) {
        *(translated+j) = 0;
        *(flashStr+j) = 0;
    }
    flashLength = 0;
    transLength = 0;
    return i;
    
}


MODULE_LICENSE("GPL"); 
module_init(thread_init);
module_exit(thread_cleanup);
