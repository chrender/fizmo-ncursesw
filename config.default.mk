
CC = gcc
AR = ar
override CFLAGS += -Wall -Wextra

prefix = /usr/local
destprefix = $(DESTDIR)$(prefix)
bindir = $(destprefix)/bin
datarootdir = $(destprefix)/share
mandir = $(datarootdir)/man
localedir = $(datarootdir)/fizmo/locales


# -----
# General settings:
ENABLE_OPTIMIZATION = 1
#ENABLE_TRACING = 1
#ENABLE_GDB_SYMBOLS = 1
# -----


# -----
# Settings for ncursesw -- required by fizmo-ncursesw and fizmo-glktermw:
# If pkg-config provides information how to find the ncursesw files, you
# can use the following two lines to make fizmo automatically find the
# required information:
#NCURSESW_PKG_CFLAGS = $(shell pkg-config --cflags ncursesw)
#NCURSESW_PKG_LIBS = $(shell pkg-config --libs ncursesw)

# In case pkg-config has no information about ncursesw, you have to provide
# the required flags below:
NCURSESW_NONPKG_CFLAGS = -I/usr/include/ncursesw
NCURSESW_NONPKG_LIBS = -L/usr/lib -lncursesw
# -----



# -----
# Settings for fizmo-ncursesw (also requires ncursesw, above):
ENABLE_X11_IMAGES = 1
SOUND_INTERFACE_CONFIGNAME = SOUNDSDL
SOUND_INTERFACE_STRUCT_NAME = sound_interface_sdl
SOUND_INTERFACE_INCLUDE_FILE = sound_sdl/sound_sdl.h
SOUND_INTERFACE_NAME = libsndifsdl
# -----

