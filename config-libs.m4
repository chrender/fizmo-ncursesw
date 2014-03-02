
libncursesw_nonpkg_cflags=""
libncursesw_nonpkg_libs=""

PKG_CHECK_MODULES(
  [ncursesw],
  [ncursesw],
  ,
  [for dir in $with_ncursesw_includedir /usr/include /usr/local/include /opt/local/include ; do
    AC_MSG_CHECKING(for $dir/ncurses.h)
    if [ test -e $dir/ncurses.h ]; then
      AC_MSG_RESULT(yes)
      ncursesw_h_dir=$dir
      break
    else
      AC_MSG_RESULT(no)
    fi
  done

  if [ test "x$ncursesw_h_dir" == "x"] ; then
    echo "Could not find ncurses.h."
    echo "Try setting the location using --with-ncursesw-includedir."
    exit
  fi

  libncursesw_nonpkg_cflags+="-I$ncursesw_h_dir"

  LIBS_SAVED=$LIBS
  LDFLAGS_SAVED=$LDFLAGS
  LIBS="-lncurses"
  for dir in $with_ncursesw_libdir /usr/lib /usr/local/lib /opt/local/lib ; do
    AC_MSG_CHECKING(for libncuresw in $dir)
    LDFLAGS="-L$dir"
    AC_TRY_LINK(
      [#include <stdio.h>
       #include <ncurses.h>
       #include "$ncursesw_h_dir/jpeglib.h"],
      [ #define _XOPEN_SOURCE_EXTENDED 1
        wchar_t buf[2]; addwstr(buf); ],
      [AC_MSG_RESULT(yes)
       ncursesw_l_dir=$dir
       break],
      [AC_MSG_RESULT(no)])
  done
  if [ test "x$ncursesw_l_dir" == "x"] ; then
    echo "Could not find libncursesw. Please note that you need to have lincursesw(!) installed, libncurses does not work."
    echo "Try setting the location using --with-ncursesw-libdir."
    exit
  fi
  LIBS=$LIBS_SAVED
  LDFLAGS=$LDFLAGS_SAVED
  libncursesw_nonpkg_libs="-L$ncursesw_l_dir -lncursesw"
])

AS_IF([test "x$enable_x11" != "xno"], [
  PKG_CHECK_MODULES([x11], [x11])
])

