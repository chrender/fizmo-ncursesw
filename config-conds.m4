
AM_CONDITIONAL([ENABLE_X11_IMAGES],
                [test "$enable_x11_images" != "no"])

AM_CONDITIONAL([ENABLE_SDL_SOUND],
                [test "$enable_sdl_sound" != "no"])

AM_CONDITIONAL([ENABLE_FILELIST],
                [test "$enable_filelist" != "no"])

AM_CONDITIONAL([ENABLE_CONFIG_FILES],
                [test "$enable_config_files" != "no"])

