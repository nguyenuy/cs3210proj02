/* This code blinks LEDs connected to Galileo GPIO3 (Arduino PIN13), GPIO17,
 * GPIO24 and GPIO27 (pins 5 6 and 7)
 */

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define GP_LED                 (3) // GPIO3 is GP LED - LED connected between Cypress CY8C9540A and RTC battery header
#define GP_5                   (17) //GPIO17 corresponds to Arduino PIN5
#define GP_6                   (24) //GPIO24 corresponds to Arduino PIN6
#define GP_7                   (27) //GPIO27 corresponds to Arduino PIN7
#define BLINK_TIME_SEC         (1) // 1 seconds blink time
#define GPIO_DIRECTION_IN      (1)
#define GPIO_DIRECTION_OUT     (0)
#define ERROR                  (-1)


/*
* Morse code structure and timing
* http://www.nu-ware.com/NuCode%20Help/index.html?morse_code_structure_and_timing_.htm
* ==================================
* Dash Length = Dot length x 3
* Pause between elements = Dot Length
* Pause between characters = Dot Length x 3
* Pause between words = Dot Length x 7
*/
#define DIT_LENGTH             (1)
#define DAH_LENGTH             (3)

#define SLEEP_ELEMENT          (1)
#define SLEEP_CHARACTER        (3)
#define SLEEP_WORD             (7)

#define CHAR_SIZE              (256)
short charToBin[CHAR_SIZE];
char translated[1000];
int transLength;

//Putting Morse Code Function Declarations Here
void initialize_character_array();
int flash_led(int gpio, int seconds, int type);
int string_to_flash_led(char *buf);
int string_to_morse(char *buf, int length);


int openGPIO(int gpio, int direction )
{
        char buffer[256];
        int fileHandle;
        int fileMode;

  //Export GPIO
        fileHandle = open("/sys/class/gpio/export", O_WRONLY);
        if(ERROR == fileHandle)
        {
               puts("Error: Unable to opening /sys/class/gpio/export");
               return(-1);
        }
        sprintf(buffer, "%d", gpio);
        write(fileHandle, buffer, strlen(buffer));
        close(fileHandle);

   //Direction GPIO
        sprintf(buffer, "/sys/class/gpio/gpio%d/direction", gpio);
        fileHandle = open(buffer, O_WRONLY);
        if(ERROR == fileHandle)
        {
               puts("Unable to open file:");
               puts(buffer);
               return(-1);
        }

        if (direction == GPIO_DIRECTION_OUT)
        {
               // Set out direction
               write(fileHandle, "out", 3);
               fileMode = O_WRONLY;
        }
        else
        {
               // Set in direction
               write(fileHandle, "in", 2);
               fileMode = O_RDONLY;
        }
        close(fileHandle);


   //Open GPIO for Read / Write
        sprintf(buffer, "/sys/class/gpio/gpio%d/value", gpio);
        fileHandle = open(buffer, fileMode);
        if(ERROR == fileHandle)
        {
               puts("Unable to open file:");
               puts(buffer);
               return(-1);
        }

        return(fileHandle);  //This file handle will be used in read/write and close operations.
}

int writeGPIO(int fHandle, int val)
{
        if(val ==  0)
        {
               // Set GPIO low status
               write(fHandle, "0", 1);
        }
        else
        {
               // Set GPIO high status
               write(fHandle, "1", 1);
        }

        return(0);
}

int readGPIO(int fileHandle)
{
        int value;

        read(fileHandle, &value, 1);

        if('0' == value)
        {
             // Current GPIO status low
               value = 0;
        }
        else
        {
             // Current GPIO status high
               value = 1;
        }

        return value;
}

int closeGPIO(int gpio, int fileHandle)
{
        char buffer[256];

        close(fileHandle); //This is the file handle of opened GPIO for Read / Write earlier.

        fileHandle = open("/sys/class/gpio/unexport", O_WRONLY);
        if(ERROR == fileHandle)
        {
               puts("Unable to open file:");
               puts(buffer);
               return(-1);
        }
        sprintf(buffer, "%d", gpio);
        write(fileHandle, buffer, strlen(buffer));
        close(fileHandle);

        return(0);
}


int main(void)
{
        //Initialization
        initialize_character_array();

        //Get User Input
        char msgstring[1000];
        int msglength;
        printf("Enter a string: ");
        fgets(msgstring,1000,stdin);
        printf("Your string is: %s", msgstring);

        for(msglength=0; msgstring[msglength]!='\0'; ++msglength);

        //String Processing/Flashing
        transLength = 0;
        string_to_morse(msgstring, msglength);
        string_to_flash_led(translated);
        

        //TODO: REMOVE CODE BELOW LATER
        //Modify Code Below Later 
        int fileHandleGPIO_LED;
        int fileHandleGPIO_5;
        int fileHandleGPIO_6;
        int fileHandleGPIO_7;
        int i=0;

        puts("Starting LED blink GP_LED - gpio-3 on Galileo board.");

        fileHandleGPIO_LED = openGPIO(GP_LED, GPIO_DIRECTION_OUT);
        fileHandleGPIO_5 = openGPIO(GP_5, GPIO_DIRECTION_OUT);
        fileHandleGPIO_6 = openGPIO(GP_6, GPIO_DIRECTION_OUT);
        fileHandleGPIO_7 = openGPIO(GP_7, GPIO_DIRECTION_OUT);


        if(ERROR ==  fileHandleGPIO_LED)
        {
               return(-1);
        }


        for(i=0; i< 10; i++)
        {
               //LED ON
               writeGPIO(fileHandleGPIO_LED, 1);
               writeGPIO(fileHandleGPIO_5, 1);
               writeGPIO(fileHandleGPIO_6, 1);
               writeGPIO(fileHandleGPIO_7, 1);
               sleep(BLINK_TIME_SEC);

               //LED OFF
               writeGPIO(fileHandleGPIO_LED, 0);
               writeGPIO(fileHandleGPIO_5, 0);
               writeGPIO(fileHandleGPIO_6, 0);
               writeGPIO(fileHandleGPIO_7, 0);
               sleep(BLINK_TIME_SEC);
        }

        closeGPIO(GP_LED, fileHandleGPIO_LED);
        closeGPIO(GP_5, fileHandleGPIO_5);
        closeGPIO(GP_6, fileHandleGPIO_6);
        closeGPIO(GP_7, fileHandleGPIO_7);


        puts("Finished LED blink GP_LED - gpio-3 on Galileo board.");

        return 0;
}

//Morse Code Definitions Here
void initialize_character_array(){
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

int flash_led(int gpio, int seconds, int type) {

    int filehandle_LEDGPIO;

    filehandle_LEDGPIO = openGPIO(gpio, GPIO_DIRECTION_OUT);

    if(filehandle_LEDGPIO == ERROR) {
        return -1;
    }

    //LED ON
    writeGPIO(filehandle_LEDGPIO, 1);
    sleep(seconds);

    //LED OFF
    writeGPIO(filehandle_LEDGPIO, 0);
    sleep(type);


    closeGPIO(gpio, filehandle_LEDGPIO);  

    return 0;
}

//string_to_morse encodes a string buffer and puts into globabl variable 'translated'
int string_to_morse(char *buf, int length) {

    int i=0;
    char *p = translated;
    for(; i<length; i++){
        char ch = *(buf+i);
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


int string_to_flash_led(char* buf) {
    int i = 0;
    
    //added a var called transLength for the length so we don't have to waste time calculating it twice (it's calc'd in the string_to_morse function)

    //Iterate over encoded string
    //TODO: Change/keep wait times...if changed will have to change in flash_led(...) as well
    //References: Morse Code Timing link at the beginning of this file 
    for (i=0; i < transLength; i++) {
        char ch = *(buf+i);
        if (ch == 'i') { //dit
            flash_led(GP_LED, 1, 0); //flash_led (gpio 3, 1 second, 0 = dit)
        } else if (ch == 'a') { //dah
            flash_led(GP_LED, 3, 1); //flash_led (gpio 3, 3 seconds, 1 = dah)
        } else if (ch == '-') { //between characters
            sleep(3);
        } else if (ch == ' ') { //spaces
            sleep(7);
        }
    }
    return 1;
}