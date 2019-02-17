
libncursesw_nonpkg_cflags=""
libncursesw_nonpkg_libs=""

PKG_CHECK_MODULES(
  [ncursesw],
  [ncursesw],
  ,
  [for dir in $with_ncurses_includedir /usr/include /usr/local/include /opt/local/include ; do
    AC_MSG_CHECKING(for $dir/ncurses.h)
    if [ test -e $dir/ncurses.h ]; then
      AC_MSG_RESULT(yes)
      ncurses_h_dir=$dir
      break
    else
      AC_MSG_RESULT(no)
    fi
  done

  if [ test "x$ncurses_h_dir" == "x"] ; then
    echo "Could not find ncurses.h."
    echo "Try setting the location using --with-ncurses-includedir."
    exit
  fi

  libncursesw_nonpkg_cflags+="-I$ncurses_h_dir"

  LIBS_SAVED=$LIBS
  LDFLAGS_SAVED=$LDFLAGS
  LIBS="-lncursesw"
  for dir in $with_ncursesw_libdir /usr/lib /usr/local/lib /opt/local/lib ; do
    AC_MSG_CHECKING(for libncursesw in $dir)
    LDFLAGS="-L$dir"
    AC_LINK_IFELSE(
      [AC_LANG_PROGRAM(
       [[#define _XOPEN_SOURCE_EXTENDED 1
         #include <stdio.h>
         #include "$ncurses_h_dir/ncurses.h"]],
       [[wchar_t buf[2]; addwstr(buf); ]])],
      [AC_MSG_RESULT(yes)
       ncursesw_l_dir=$dir
       break],
      [AC_MSG_RESULT(no)])
  done
  if [ test "x$ncursesw_l_dir" != "x"] ; then
    libncursesw_nonpkg_libs="-L$ncursesw_l_dir -lncursesw"
  else
    echo "Could not find libncursesw. Trying to use libcurses instead."

    LIBS="-lncurses"
    for dir in $with_ncurses_libdir /usr/lib /usr/local/lib /opt/local/lib ; do
      AC_MSG_CHECKING(for libncurses in $dir)
      LDFLAGS="-L$dir"
      AC_LINK_IFELSE(
        [AC_LANG_PROGRAM(
         [[#define _XOPEN_SOURCE_EXTENDED 1
           #include <stdio.h>
           #include "$ncurses_h_dir/ncurses.h"]],
         [[wchar_t buf[2]; addwstr(buf); ]])],
        [AC_MSG_RESULT(yes)
         ncursesw_l_dir=$dir
         break],
        [AC_MSG_RESULT(no)])
    done
    if [ test "x$ncursesw_l_dir" != "x"] ; then
      libncursesw_nonpkg_libs="-L$ncursesw_l_dir -lncurses"
    else
      echo "Couldn't find libncursesw or libncurses. You can set the location manually using --with-ncurses-libdir or --with-ncursesw-libdir."
      exit
    fi
  fi
  LIBS=$LIBS_SAVED
  LDFLAGS=$LDFLAGS_SAVED
])

AS_IF([test "x$enable_x11" != "xno"], [
  PKG_CHECK_MODULES([x11], [x11])
])

