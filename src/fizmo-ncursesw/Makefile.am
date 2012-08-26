
AM_CFLAGS = $(libfizmo_CFLAGS) $(libcellif_CFLAGS) $(ncursesw_CFLAGS)
AM_CPPFLAGS =
LDADD = $(libfizmo_LIBS) $(libcellif_LIBS) $(ncursesw_LIBS)

bin_PROGRAMS = fizmo-ncursesw
fizmo_ncursesw_SOURCES = ../locales/fizmo_ncursesw_locales.c fizmo-ncursesw.c

if ENABLE_TRACING
AM_CFLAGS += -DENABLE_TRACING=
endif

if ENABLE_X11_IMAGES
AM_CFLAGS += $(libdrilbo_CFLAGS) $(x11_CFLAGS) -DENABLE_X11_IMAGES=
LDADD += $(libdrilbo_LIBS) $(x11_LIBS)
endif

if ENABLE_SDL_SOUND
AM_CFLAGS += $(libsndifsdl_CFLAGS) \
 -DSOUND_INTERFACE_STRUCT_NAME="$(SOUND_INTERFACE_STRUCT_NAME)" \
 -DSOUND_INTERFACE_INCLUDE_FILE="<$(SOUND_INTERFACE_INCLUDE_FILE)>"
LDADD += $(libsndifsdl_LIBS)
endif

if ENABLE_FILELIST
else
AM_CFLAGS += -DDISABLE_FILELIST=
endif

if ENABLE_CONFIG_FILES
else
AM_CFLAGS += -DDISABLE_CONFIGFILES=
endif

