.DEFAULT_GOAL := ee
RACDOOR = ../..
TARGET ?= ee
SRCS = probe.s
include $(RACDOOR)/make/config.$(TARGET).mk
include $(RACDOOR)/make/compile.mk

.PHONY: ee host clean
host: install-probe
ee: probe.bin

probe.elf: probe.ee.o probe.ld
	$(CC) -nostdlib -Wl,-T,probe.ld -o $@ $<

probe.bin: probe.elf
	ee-objcopy -O binary -j .text $< $@

install-probe: install.c ../../sdk/save.c ../../sdk/buffer.c ../../sdk/exploit.c
	gcc -std=c99 -O2 -Wall -Werror -D_HOST -I../../include $^ -o $@

clean: compileclean
	$(RM) probe.elf probe.bin install-probe
