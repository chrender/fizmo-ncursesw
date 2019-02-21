
AC_ARG_ENABLE([x11],
 [AS_HELP_STRING([--disable-x11],
                 [disable X11 support])],
 [],
 [enable_x11=yes])

AC_ARG_ENABLE([sound],
 [AS_HELP_STRING([--disable-sound],
                 [Disable sound support])],
 [],
 [enable_sound=yes])

AC_ARG_WITH([ncurses-includedir],
 [AS_HELP_STRING([--with-ncurses-includedir],
          [Specify include directory to use for ncurses.h])],
 [AC_SUBST([with_ncurses_includedir],
   [$( echo $(cd $(dirname "$with_ncurses_includedir") && pwd -P)/$(basename "$with_ncurses_includedir") )])],
 [with_ncurses_includedir=])

AC_ARG_WITH([ncursesw-libdir],
 [AS_HELP_STRING([--with-ncursesw-libdir],
          [Specify library directory for ncursesw])],
 [AC_SUBST([with_ncursesw_libdir],
   [$( echo $(cd $(dirname "$with_ncursesw_libdir") && pwd -P)/$(basename "$with_ncursesw_libdir") )])],
 [with_ncursesw_libdir=])

AC_ARG_WITH([ncurses-libdir],
 [AS_HELP_STRING([--with-ncurses-libdir],
          [Specify library directory for ncurses])],
 [AC_SUBST([with_ncurses_libdir],
   [$( echo $(cd $(dirname "$with_ncurses_libdir") && pwd -P)/$(basename "$with_ncurses_libdir") )])],
 [with_ncurses_libdir=])

