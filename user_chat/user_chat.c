/* This code interfaces with the /dev/morse drivers and serves
 * as a user space chat program
 */

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define MSGLENGTH 1000

int main(void)
{     

      //Initialization of kernel drivers
      int fd_write = open("/dev/writeMorse", O_WRONLY);
      int fd_read = open("/dev/readMorse", O_RDONLY);

      //TODO: make program continuously loop
      //Program currently only sends and receives message one time. 
      //Nothing else.

      //Message buffer variables
      char sendstring[MSGLENGTH];
      int sendlength;
      char rcvstring[MSGLENGTH];
      int rcvlength;

      //Get Local User Input to Send String
      printf("Enter a message to send [1000 characters max]: ");
      fgets(sendstring,MSGLENGTH,stdin);
      printf("Sending Message: %s\n", sendstring);
      sendlength = strlen(sendstring);

      //Get Remote User Input (Morse Switch Button)
      read(fd_read, rcvstring, MSGLENGTH);
      printf("Received Message: %s\n", rcvstring);
      return 0;
}
