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


/* 
 * Global variables are declared as static, so are global within the file. 
 */

int readSignal(){
  if( gpio_get_value(switch_gpio[0].gpio) == 0)
    return 0;
  else
    return 1;
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

void init(char* A, int n){
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

  char morseMap[512];
  initMorseMap(morseMap);

  int buff_len = 16;
  int LONG_WAIT = 100;
  int unit = 4;
  int MAX_DIT = 15;
  
  
  int iter = 1;
  for(; iter<100000; iter++){
    if(readSignal() == 1){
      char buf[buff_len];
      init(buf,buff_len);
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
      int key = getDecodeKey(buf, buff_len);
      char result = morseMap[key];
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
