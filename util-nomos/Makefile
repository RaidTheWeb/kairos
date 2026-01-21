CC ?= gcc
CFLAGS ?= -O2 -Wall -static
LDFLAGS ?=
PREFIX ?= /usr
BINDIR ?= $(PREFIX)/bin

# Build outputs are placed here before installation.
INTERMEDIATEDIR ?= build

.PHONY: all install

all: mount umount pivot_root

mount: mount.c
	mkdir -p $(INTERMEDIATEDIR)
	$(CC) $(CFLAGS) -o $(INTERMEDIATEDIR)/mount mount.c $(LDFLAGS)

umount: umount.c
	mkdir -p $(INTERMEDIATEDIR)
	$(CC) $(CFLAGS) -o $(INTERMEDIATEDIR)/umount umount.c $(LDFLAGS)

pivot_root: pivot_root.c
	mkdir -p $(INTERMEDIATEDIR)
	$(CC) $(CFLAGS) -o $(INTERMEDIATEDIR)/pivot_root pivot_root.c $(LDFLAGS)

install:
	mkdir -p $(DESTDIR)$(BINDIR)
	cp $(INTERMEDIATEDIR)/* $(DESTDIR)$(BINDIR)/