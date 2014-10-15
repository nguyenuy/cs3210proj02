Project Log
=========


###Project 2 Progress Log
10/15 - Compiled userMorseWrite.c which only blinks LEDs now on pins GPIO 3,5,6,7 [Ses,Uy]

10/14 - Installed Debian onto SD Card. (Ses has documentation on this) Using v1.2
 http://sourceforge.net/projects/galileodebian/files/SD%20card%20Image/<br>
 Added userWriteMorse.c and Makefile

10/09 - Ses and Uy were able to manually get an LED light to blink on the board. Ses established the GitHub repository and have put a resource in the folder "openGPIO.c"

10/08 - Get the serial connection working on the Galileo Board [Ses]

           
###Project 2 To-Do until Meeting Until 10/15
1. work towards implementing user space code based on openGPIO.c to be tested on Wednesday
2. think about how to implement the switch. The following links may help 
	1. http://youtu.be/ZdzjvIk_aY0?t=2m23s
	2. http://youtu.be/kDeVR6sWFZ4
3. Think about building simple LED circuit on the board. Maybe Two LEDS for dit and dah?

###Project 2 Milestones (Full List)
1. User space morse code to flash LED
2. User space morse code to read from switch
3. Build Yocto Linux infrastructure (or load Debian) so that can load kernel module
4. Kernel Module - Implement read side of morse character 5. device in kernel. GPIO signal from board to character
6. Kernel Module - Implement write side.
7. Implement User Space Chat Program  
8. Complete Project Write-Up
