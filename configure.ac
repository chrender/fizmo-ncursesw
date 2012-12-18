
# configure.ac
#
# This file is part of fizmo.
#
# Copyright (c) 2009-2012 Christoph Ender.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


AC_ARG_ENABLE([tracing],
 [AS_HELP_STRING([--enable-tracing],
                 [enable tracelog for debugging])],
 [],
 [enable_tracing=no])

#AC_ARG_ENABLE([x11-images],
# [AS_HELP_STRING([--disable-x11-images],
#                 [Don't display frontispiece in X11-window])],
# [],
# [enable_x11_images=yes])

AC_ARG_ENABLE([sdl-sound])
 [AS_HELP_STRING([--disable-sdl-sound],
                 [Disable sound support])],
 [],
 [enable_sdl_sound=yes])

AC_ARG_ENABLE([filelist],
 [AS_HELP_STRING([--disable-filelist],
                  [disable story-list-keeping functionality])],
 [],
 [enable_filelist=yes])

AC_ARG_ENABLE([config-files],
 [AS_HELP_STRING([--disable-config-files],
	 [disable parsing of configuration files])],
 [],
 [enable_config_files=yes])

AC_INIT([fizmo-ncursesw], [0.7.6])

AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
AC_PROG_RANLIB

PKG_CHECK_MODULES([libfizmo], [libfizmo >= 0.7.5])
PKG_CHECK_MODULES([libcellif], [libcellif >= 0.7.5])
PKG_CHECK_MODULES([libsndifsdl], [libsndifsdl >= 0.7.3])
PKG_CHECK_MODULES([libdrilbo], [libdrilbo >= 0.2.3])
m4_include(config-libs.m4)

AS_IF([test "x$enable_x11" != "xno"], [
  PKG_CHECK_MODULES([x11], [x11])
])

AM_CONDITIONAL([ENABLE_TRACING],
                [test "$enable_tracing" = "yes"])

m4_include(config-conds.m4)
m4_include(config-subst.m4)

AC_SUBST([SOUND_INTERFACE_NAME], libsndifsdl)
AC_SUBST([SOUND_INTERFACE_STRUCT_NAME], sound_interface_sdl)
AC_SUBST([SOUND_INTERFACE_CONFIGNAME], SOUNDSDL)
AC_SUBST([SOUND_INTERFACE_INCLUDE_FILE], sound_sdl/sound_sdl.h)

AC_CONFIG_FILES([Makefile src/fizmo-ncursesw/Makefile])
AC_OUTPUT

