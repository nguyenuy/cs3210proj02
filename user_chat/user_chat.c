/* This code interfaces with the /dev/morse drivers and serves
 * as a user space chat program
 */

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

char send[256];
char recv[256];

char* sendP;
char* recvP;
char ch;

int recvLength = 0;

FILE *f;

void timeoutChar() {
	clock_t start = clock(), diff;
	int seconds;
    while(1) {
		
		diff = clock() - start;
		seconds = diff * CLOCKS_PER_SEC;
		if(seconds < 10) {
			while(fscanf(f, "%c", &ch) != EOF) {
				*(recvP++) = ch;
				recvLength++;
				seconds = 0;
				start = clock();
			}
		} else {
			break;
		}
		
	}
    
}

int main(int argc, char* argv[])
{     
	
      //Initialization of kernel drivers
    f = fopen("/dev/morseThread", "rw");
	
	sendP = send;
	recvP = recv;
	
	int i = 0;
	
    if(f == NULL) {
		printf("No Mem Error");
		return -1;
	}
	
	while(1) {
		
		printf("Please enter a message to send: ");
		scanf("%s", send);
		fprintf(f, "%s", send);
		
		timeoutChar();
		*(recvP) = '\0';
		recvP = recv;
		printf("Received: %s", recv);
		
	}
	
	fclose(f);
	
}


