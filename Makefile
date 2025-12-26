CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g -D_POSIX_C_SOURCE=200809L
INCLUDES = -Iinclude

SRCS = src/main.c \
       src/image.c \
       src/container.c \
       src/fsutil.c \
       src/log.c \
       src/timeutil.c

OBJS = $(SRCS:.c=.o)
TARGET = mdock

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
