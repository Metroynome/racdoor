.DEFAULT_GOAL := ee
RACDOOR = ../..
TARGET ?= ee
SRCS = payload.c

include $(RACDOOR)/make/config.$(TARGET).mk
include $(RACDOOR)/make/compile.mk

.PHONY: all host ee test clean
all: ee
host:
ee: uya-dnas-pal.bin uya-dnas-pal.dis

uya-dnas-pal.elf: payload.ee.o payload.ld
	$(CC) -nostdlib -Wl,-T,payload.ld,-Map,uya-dnas-pal.map -o $@ $<

uya-dnas-pal.bin: uya-dnas-pal.elf
	ee-objcopy -O binary -j .text $< $@

uya-dnas-pal.dis: uya-dnas-pal.elf
	$(OBJDUMP) -d $< > $@

test:
	gcc -std=c99 -O2 -Wall -Wextra -Werror -I../../include test.c -o test-host
	./test-host

clean: compileclean
	$(RM) uya-dnas-pal.elf uya-dnas-pal.bin uya-dnas-pal.dis uya-dnas-pal.map test-host
