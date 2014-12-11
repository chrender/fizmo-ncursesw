
AM_CONDITIONAL([ENABLE_X11_IMAGES],
                [test "$enable_x11" != "no"])

AM_CONDITIONAL([ENABLE_SDL],
                [test "$enable_sdl" != "no"])

AM_CONDITIONAL([ENABLE_FILELIST],
                [test "$enable_filelist" != "no"])

AM_CONDITIONAL([ENABLE_CONFIG_FILES],
                [test "$enable_config_files" != "no"])

