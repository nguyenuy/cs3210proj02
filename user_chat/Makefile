# the compiler: gcc for C program, define as g++ for C++
CC = gcc

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -g -Wall

# the build target executable:

all: user_chat

user_chat: user_chat.c
	$(CC) $(CFLAGS) -o user_chat user_chat.c

clean:
	$(RM) user_chat
