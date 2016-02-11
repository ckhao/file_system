CC = gcc
CFLAGS = -Wall
OUTPUT = diskinfo disklist diskget diskput

.PHONY: all $(OUTPUT)
all: $(OUTPUT)

$(OUTPUT): 
	$(CC) $@.c -o $@



