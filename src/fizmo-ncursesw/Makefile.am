
# Makefile.am
#
# This file is part of fizmo.
#
# Copyright (c) 2009-2014 Christoph Ender.
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


AUTOMAKE_OPTIONS = subdir-objects

AM_CFLAGS = $(libfizmo_CFLAGS) $(libcellif_CFLAGS) $(ncursesw_CFLAGS) $(LIBNCURSESW_NONPKG_CFLAGS)
AM_CPPFLAGS =
LDADD = $(libfizmo_LIBS) $(libcellif_LIBS) $(ncursesw_LIBS) $(LIBNCURSESW_NONPKG_LIBS)

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

