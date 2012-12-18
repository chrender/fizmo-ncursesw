
AS_IF([test "x$enable_sound" == "xyes"], [
  AC_SUBST([SOUND_INTERFACE_NAME], libsndifsdl)
  AC_SUBST([SOUND_INTERFACE_STRUCT_NAME], sound_interface_sdl)
  AC_SUBST([SOUND_INTERFACE_CONFIGNAME], SOUNDSDL)
  AC_SUBST([SOUND_INTERFACE_INCLUDE_FILE], sound_sdl/sound_sdl.h)
])

