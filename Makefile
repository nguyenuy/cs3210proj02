# the compiler: gcc for C program, define as g++ for C++
CC = gcc

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -g -Wall

# the build target executable:

all: userWriteMorse userReadMorse

userWriteMorse: userWriteMorse.c
	$(CC) $(CFLAGS) -o userWriteMorse userWriteMorse.c

userReadMorse: userReadMorse.c
	$(CC) $(CFLAGS) -o userReadMorse userReadMorse.c
clean:
	$(RM) userWriteMorse userReadMorse

test: chenTest.c
	gcc chenTest.c -o test