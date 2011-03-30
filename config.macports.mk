
CC = gcc
AR = ar
CFLAGS = -Wall -Wextra

ifneq ($(DESTDIR),)
INSTALL_PREFIX = $(DESTDIR)
else
#INSTALL_PREFIX = /usr/local
INSTALL_PREFIX = $(HOME)/opt/fizmo
endif

# Uncomment to install binaries to $(INSTALL_PREFIX)/$(FIZMO_BIN_DIR).
#FIZMO_BIN_DIR = bin


# General:
ENABLE_OPTIMIZATION = 1
ENABLE_TRACING = 1
#ENABLE_GDB_SYMBOLS = 1


# fizmo-ncursesw:
NCURSESW_PKG_CFLAGS = $(shell pkg-config --cflags ncursesw)
NCURSESW_PKG_LIBS = $(shell pkg-config --libs ncursesw)
NCURSESW_NONPKG_CFLAGS =
NCURSESW_NONPKG_LIBS =
SOUND_INTERFACE_NAME = libsndifsdl
SOUND_INTERFACE_CONFIGNAME = SOUNDSDL
SOUND_INTERFACE_STRUCT_NAME = sound_interface_sdl
SOUND_INTERFACE_INCLUDE_FILE = sound_sdl/sound_sdl.h
ENABLE_X11_IMAGES = 1

