
.PHONY : all clean distclean

include ../../config.mk

include $(FIZMO_INC_DIR)/fizmo/libfizmo.mk
include $(FIZMO_INC_DIR)/fizmo/libcellif.mk
include $(FIZMO_INC_DIR)/fizmo/libsndifsdl.mk

CC = gcc -Wall
CFLAGS = -g -O2 $(SOUND_INTERFACE_CFLAGS)

ifneq ($(ENABLE_GDB_SYMBOLS),)
override CFLAGS += -g
endif

ifneq ($(ENABLE_TRACING),)
override CFLAGS += -DENABLE_TRACING=
endif

INC_DIRS = $(LIBFIZMO_INC_DIRS) $(LIBCELLIF_INC_DIRS) -I$(NCURSESW_INC_DIR)
LIB_DIRS = $(LIBFIZMO_LIB_DIRS) $(LIBCELLIF_LIB_DIRS) $(SOUND_INTERFACE_LIB_DIRS) -L$(NCURSESW_LIB_DIR)
LIBS = $(LIBCELLIF_LIBS) $(LIBFIZMO_LIBS) $(SOUND_INTERFACE_LIBS) -lncursesw
FLAGS =

FIZMO_SOUND_INTERFACE = $(SOUND_INTERFACE_STRUCT_NAME)

ifneq ($(ENABLE_X11_IMAGES),)
FLAGS += -DENABLE_X11_IMAGES
endif


all: fizmo-ncursesw

fizmo-ncursesw: fizmo-ncursesw.o ../locales/fizmo_ncursesw_locales.o
	$(CC) $(CFLAGS) fizmo-ncursesw.o ../locales/fizmo_ncursesw_locales.o $(LIB_DIRS) $(LIBS) -o fizmo-ncursesw

../locales/fizmo_ncursesw_locales.o::
	cd ../locales ; make

fizmo-ncursesw.o: fizmo-ncursesw.c
	$(CC) $(CFLAGS) $(INC_DIRS) $(FLAGS) \
	-DSOUND_INTERFACE_STRUCT_NAME="$(SOUND_INTERFACE_STRUCT_NAME)" \
	-DSOUND_INTERFACE_INCLUDE_FILE="<$(SOUND_INTERFACE_INCLUDE_FILE)>" \
	-c fizmo-ncursesw.c

clean:
	rm -f *.o
	cd ../locales ; make clean

distclean: clean
	rm -f fizmo-ncursesw
	cd ../locales ; make distclean

