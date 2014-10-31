#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

/*
 *  GPIO Definitions 
 */
#define GP_LED                 (3) // GPIO3 is GP LED - LED connected between Cypress CY8C9540A and RTC battery header
#define GP_5                   (17) //GPIO17 corresponds to Arduino PIN5
#define GP_6                   (24) //GPIO24 corresponds to Arduino PIN6
#define GP_7                   (27) //GPIO27 corresponds to Arduino PIN7

#define SUCCESS 0
#define DEVICE_NAME "morse"   /* Dev name as it appears in /proc/devices   */
#define BUF_LEN 80      /* Max length of the message from the device */

#define CHAR_SIZE 256
#define MORSE_BIN 16



/*
 * Struct defining pins, direction and inital state 
 */
static struct gpio morse_gpio[] = {
  {  GP_LED, GPIOF_OUT_INIT_HIGH, "LED" },
};

static struct gpio switch_gpio[] = {
  { GP_7, GPIOF_IN, "SWITCH"},
};

typedef __u16 morse_t;
int len,temp;
morse_t charToBin[CHAR_SIZE];

/* 
 * Global variables are declared as static, so are global within the file. 
 */

int readSignal(){
  if( gpio_get_value(switch_gpio[0].gpio) == 0)
    return 1;
  else
    return 0;
}


int getDecodeKey(int* buf,int n){
  int ans = 1;
  int i=0;
  for(; i<n && buf[i] >= 0; i++){
    ans = (ans<<1) + buf[i];
  }
  
  return ans;
}

/* *******************************/
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

void init(int* A, int n){
  int i=0;
  for(; i<n; i++){
    A[i] = -2;
  }
}

static int __init morse_init(void)
{
  printk(KERN_INFO "%s\n", __func__);
   
  printk(KERN_INFO "the driver, create a dev file with\n");
  printk(KERN_INFO "Try various minor numbers. Try to cat and echo to\n");
  printk(KERN_INFO "the device file.\n");
  printk(KERN_INFO "Remove the device file and module when done.\n");
  gpio_set_value(morse_gpio[0].gpio, 1);

  initialize_char_to_bin_array();
  
  int BUF_LEN = 16;
  int LONG_WAIT = 100;
  int unit = 4;
  int MAX_DIT = 15;
  
  
  int iter = 1;
  for(; iter<100000; iter++){
    if(readSignal() == 1){
      int buf[BUF_LEN];
      init(buf,BUF_LEN);
      int cnt = 0;
      while(1){
	int cnt1 = 0, cnt0 = 0;
	while(readSignal() == 1){
	  cnt1++;
	  udelay(unit);
	}
	while(readSignal() == 0){
	  cnt0++;
	  if(cnt0 > LONG_WAIT)
	    break;
	  else
	    udelay(unit);
	}
	if(cnt0 > LONG_WAIT){
	  buf[cnt++] = (cnt1>MAX_DIT?0:1);
	  break;
	}else{
	  buf[cnt++] = (cnt1>MAX_DIT?0:1);
	}
      }
      int key = getDecodeKey(buf, BUF_LEN);
      char result = charToBin[key];
      printk(KERN_INFO "do you mean %c?\n", result);
      break;
    }    
  }

  return 0;
}


/*
 * Module exit function
 */
static void __exit morse_exit(void)
{
   printk(KERN_INFO "%s\n", __func__);
   gpio_set_value(morse_gpio[0].gpio, 0);
}


MODULE_LICENSE("GPL");

module_init(morse_init);
module_exit(morse_exit);
