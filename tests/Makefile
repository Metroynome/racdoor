RACDOOR = ..
TARGET = host

include $(RACDOOR)/make/config.host.mk

.PHONY: test clean
test: help_log
	./help_log
	! ./help_log unaligned
	! ./help_log unaligned-log
	! ./help_log width
	! ./help_log positive-byte

help_log: help_log.c ../sdk/exploit.c ../include/racdoor/exploit.h
	$(CC) $(COMPILEFLAGS) $(INCLUDES) help_log.c ../sdk/exploit.c -o $@

clean:
	$(RM) help_log
