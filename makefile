#
# Makefile
#
CC = gcc
CFLAGS = -Wall -Wshadow -Wundef -Wmaybe-uninitialized
CFLAGS += -O3 -I./
LDFLAGS += -lSDL2 -lm
BIN = demo

OBJFILES = ./main.o

#CORE LIBRARY
LVGL_PATH = ./lvgl
include ./lvgl/lvgl.mk

#DRIVERS
LV_DRIVERS_PATH = ./lv_drivers
include ./lv_drivers/lv_drivers.mk

#EXAMPLES
LV_EXAMPLES_PATH = ./lv_examples
include ./lv_examples/lv_examples.mk

all: clean default

%.o: %.c
	$(CC)  $(CFLAGS) -c $< -o $@

default: $(OBJFILES)
	$(CC) -o $(BIN) $(OBJFILES) $(LDFLAGS)


clean: 
	rm -f $(BIN) $(OBJFILES)
