
include ../../config.mk


ifneq ($(ENABLE_GDB_SYMBOLS),)
override CFLAGS += -g
endif

ifneq ($(ENABLE_OPTIMIZATION),)
override CFLAGS += -O2
endif


.PHONY : clean

all: fizmo_ncursesw_locales.o


fizmo_ncursesw_locales.o: fizmo_ncursesw_locales.c
	$(CC) $(CFLAGS) -I$(FIZMO_INC_DIR)/fizmo -c fizmo_ncursesw_locales.c

clean:
	rm -f *.o

distclean: clean

