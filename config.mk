# sinit version
VERSION = 0.7

# paths
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/man

CC = cc
LD = $(CC)
CPPFLAGS =
CFLAGS   = -Wextra -Werror -Wall -Os $(CPPFLAGS)
LDFLAGS  = -static
