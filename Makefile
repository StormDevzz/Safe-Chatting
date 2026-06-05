.PHONY: all clean run

CC = gcc
CFLAGS = -O2 -Wall -Wextra -Ic -I.
LDFLAGS = -lcrypto

C_FILES := c/main/server.c c/connection/connection.c c/chatting/chatting.c \
           c/encryption/encryption.c c/encryption/crypto.c c/files/files.c
CLIENT_C := c/main/client.c

all: chatserver chat

chatserver: $(C_FILES) common.h
	$(CC) $(CFLAGS) -o $@ $(C_FILES) $(LDFLAGS)

chat: $(CLIENT_C) common.h
	$(CC) $(CFLAGS) -o $@ $(CLIENT_C)

run: chatserver
	@mkdir -p storage/messages
	./chatserver $(PORT)

clean:
	rm -f chatserver chat
