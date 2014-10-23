/*
 Copyright (c) 2014 Intel Corporation  All Rights Reserved.
 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation 
 or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. 
 No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. 
 No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. 
 Any license under such intellectual property rights must be express and approved by Intel in writing.

*
* Author: Sulamita Garcia
* Based on LEDblink.c by Nandkishor Sonar.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.

Include supplier trademarks or logos as supplier requires Intel to use, preceded by an asterisk. An asterisked footnote can be added as follows: *Third Party trademarks are the property of their respective owners.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.”

*/

/* This code gets character inputs from the user using morse code and a button
 */

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

clock_t start, end;
double cpu_time_used;

#define GP_LED                 (3) // GPIO3 is GP LED - LED connected between Cypress CY8C9540A and RTC battery header
#define GP_5				   (17) //GPIO17 corresponds to Arduino PIN5
#define GP_6				   (24) //GPIO24 corresponds to Arduino PIN6
#define GP_7				   (27) //GPIO27 corresponds to Arduino PIN7
#define BLINK_TIME_SEC         (1) // 1 seconds blink time
#define GPIO_DIRECTION_IN      (1)
#define GPIO_DIRECTION_OUT     (0)
#define ERROR                  (-1)


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
        char value;
	int ret = 0;
        read(fileHandle, &value, 1);  

        if('0' == value)
        {
             // Current GPIO status low
               ret = 0;
        }
        else if('1' == value)
        {
             // Current GPIO status high
               ret = 1;
        }

        return ret;
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

int readSignal(){
  int fileHandleGPIO_7 = openGPIO(GP_7, GPIO_DIRECTION_IN);
  int x = readGPIO(fileHandleGPIO_7);
  closeGPIO(GP_7, fileHandleGPIO_7);
  return x;
}

void printArray(int* A, int n){
  int i = 0;
  for(; i<n; i++){
    printf("%d  ", A[i]);
  }
  printf("\n");
}

void init(int* A, int n){
  int i=0;
  for(; i<n; i++){
    A[i] = -2;
  }
}

int main(void)
{
  puts("Starting LED blink GP_LED - gpio-3 on Galileo board.");
  /*
  int fileHandleGPIO_7;
  int i=0;

  while(1)
  {
  fileHandleGPIO_7 = openGPIO(GP_7, GPIO_DIRECTION_IN);
  //LED ON
  i = readGPIO(fileHandleGPIO_7);
  printf("i = %d\n", i);
  sleep(1);
  closeGPIO(GP_7, fileHandleGPIO_7);
  }
  */
  start = clock();
  const int BUF_LEN = 16;
  const int LONG_WAIT = 100;
  const int unit = 4;
  const int MAX_DIT = 15;
  char morseMap[512];
  initMorseMap(morseMap);
  while(1){
    if(readSignal() == 1){
      printf("receive a press\n");
      int buf[BUF_LEN];
      init(buf,BUF_LEN);
      int cnt = 0;
      while(1){
	int cnt1 = 0, cnt0 = 0;
	while(readSignal() == 1){
	  cnt1++;
	  usleep(unit);
	}
	while(readSignal() == 0){
	  cnt0++;
	  if(cnt0 > LONG_WAIT)
	    break;
	  else
	    usleep(unit);
	}
	if(cnt0 > LONG_WAIT){
	  buf[cnt++] = (cnt1 > MAX_DIT?0:1);
	  break;
	}else{
	  buf[cnt++] = (cnt1 > MAX_DIT?0:1);
	}
      }
      printArray(buf, BUF_LEN);
      int key = getDecodeKey(buf, BUF_LEN);
      char result = morseMap[key];
      printf("Do you mean %c?\n", result);
      end = clock();
      cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
      printf("cpu_time_used:%f", cpu_time_used);
      return
    }
  }
        


  puts("Finished LED blink GP_LED - gpio-3 on Galileo board.");

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
