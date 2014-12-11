
# This is included from fizmo-dist and not required by libfizmo's own
# configuration. It nevertheless needs to be maintained so fizmo-dist
# will still work.
#
# The $build_prefix, $build_prefix_cflags and $build_prefix_libs are
# pre-defined by fizmo-dist.

AC_SUBST([LIBNCURSESW_NONPKG_CFLAGS], "$libncursesw_nonpkg_cflags")
AC_SUBST([LIBNCURSESW_NONPKG_LIBS], "$libncursesw_nonpkg_libs")

