CC = gcc
CFLAGS = -Wall -Wextra -Os -static
TARGETS = dos gnidos-installer

all: $(TARGETS)

dos: dos_emulator.c
	$(CC) $(CFLAGS) -o $@ $<

gnidos-installer: gnidos-installer.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)

install: all
	cp $(TARGETS) ../overlay/bin/

.PHONY: all clean install

