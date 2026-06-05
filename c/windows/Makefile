.PHONY: all clean build

CC = gcc
CFLAGS = -O2 -Wall -Wextra -I.
LDFLAGS = -lcrypto -lws2_32
BUILD = build

SRC := main/server.c connection/connection.c chatting/chatting.c \
       encryption/encryption.c encryption/crypto.c files/files.c

all: $(BUILD)/chatserver.exe $(BUILD)/chat.exe

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/chatserver.exe: $(SRC) common.h | $(BUILD)
	$(CC) $(CFLAGS) -o $@ $(SRC) $(LDFLAGS)

$(BUILD)/chat.exe: main/client.c common.h | $(BUILD)
	$(CC) $(CFLAGS) -o $@ main/client.c -lws2_32

run: $(BUILD)/chatserver.exe
	$(BUILD)/chatserver.exe

clean:
	rm -rf $(BUILD)
