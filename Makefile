CC = gcc
CFLAGS = -Wall -Wextra -Isrc -g
LDFLAGS =

SRCS = builder.c src/dependencies.c src/gpu.c src/image.c src/kernel.c src/logging.c src/rootfs.c src/system_utils.c src/uboot.c src/gaming.c src/auth.c
OBJS = $(SRCS:.c=.o)

TARGET = builder
PREFIX = /usr
BINDIR = $(PREFIX)/bin
CONFDIR = /etc/orangepi-ubuntu-builder
DOCDIR = $(PREFIX)/share/doc/orangepi-ubuntu-builder
MANDIR = $(PREFIX)/share/man/man1

.PHONY: all clean install uninstall deb

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

install: $(TARGET)
	install -D -m 755 $(TARGET) $(DESTDIR)$(BINDIR)/orangepi-ubuntu-builder
	install -D -m 644 README.md $(DESTDIR)$(DOCDIR)/README.md
	install -D -m 644 debian/orangepi-ubuntu-builder.1 $(DESTDIR)$(MANDIR)/orangepi-ubuntu-builder.1
	install -d $(DESTDIR)$(CONFDIR)
	install -d $(DESTDIR)$(CONFDIR)/patches/uboot
	install -d $(DESTDIR)$(CONFDIR)/patches/kernel

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/orangepi-ubuntu-builder
	rm -rf $(DESTDIR)$(DOCDIR)
	rm -f $(DESTDIR)$(MANDIR)/orangepi-ubuntu-builder.1

# Build Debian package
deb: clean
	dpkg-buildpackage -us -uc -b
