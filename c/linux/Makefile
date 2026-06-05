.PHONY: all clean build

CC = gcc
CFLAGS = -O2 -Wall -Wextra -I.
LDFLAGS = -lcrypto
BUILD = build

SRC := main/server.c connection/connection.c chatting/chatting.c \
       encryption/encryption.c encryption/crypto.c files/files.c
OBJ := $(SRC:.c=.o)

all: $(BUILD)/chatserver $(BUILD)/chat

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/chatserver: $(SRC) common.h | $(BUILD)
	$(CC) $(CFLAGS) -o $@ $(SRC) $(LDFLAGS)

$(BUILD)/chat: main/client.c common.h | $(BUILD)
	$(CC) $(CFLAGS) -o $@ main/client.c

run: $(BUILD)/chatserver
	@mkdir -p ../../storage/messages
	$(BUILD)/chatserver

clean:
	rm -rf $(BUILD) $(OBJ)
