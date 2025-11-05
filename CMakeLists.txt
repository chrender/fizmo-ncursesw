
#[[

  CMakeLists.txt

  This file is part of fizmo.

  ---

  Copyright (c) 2010-2025 Christoph Ender.
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.
  3. Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived
  from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  ---

  This is fizmo-ncursesw's main buildfile. There is another one location
  in "src/locales" which is invoked by this one. The one in "src/locales"
  will pre-process the locales into a ".c" source file, so that they can
  be "hard-compiled" into fizmo-ncursesw, which solves a lot of problems
  regarding handling of external files and makes debugging in IDEs much
  simpler.

  Compile-time options are described individually below.

]]


cmake_minimum_required(VERSION 3.10)

# Enable external commands so it's possible to build the tools which will
# in turn create the ".h" and ".c" files required for the main build first.
include(ExternalProject)
include(FindPkgConfig)


project(fizmo-ncursesw
  LANGUAGES C
  HOMEPAGE_URL https://fizmo.spellbreaker.org
  DESCRIPTION "fizmo ncursesw frontend"
  VERSION 0.9.0)


# Create position-independent code.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


ExternalProject_Add(locale_data_preparation
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/locales
  BINARY_DIR ${CMAKE_BINARY_DIR}/loacles_build
  CMAKE_ARGS
  ${GLOBAL_DEFAULT_ARGS}
  ${GLOBAL_THIRDPARTY_LIB_ARGS}
  BUILD_COMMAND make
  INSTALL_COMMAND cmake -E echo "Skipping install step."
  )


find_package(PkgConfig REQUIRED)


# Enabling tracing will cause fizmo to write a really, really large amount
# of debug data to disk while running. Furthermore, after every single
# write, fizmo will invoke an fsync(), meaning that performance will
# quite suffer. This is only useful for debugging and should be disabled
# in all other cases.
option(ENABLE_TRACING "Enable tracing" OFF)
if (ENABLE_TRACING)
  add_definitions(-DENABLE_TRACING)
endif()


option(ENABLE_FILELIST "Enable filelist" ON)
option(ENABLE_CONFIGFILES "Enable configfiles" ON)
option(ENABLE_SOUND "Enable sound" ON)
option(ENABLE_X11_IMAGES "Enable X11" ON)


# We need libfizmo >= 0.9.
pkg_check_modules(LIBFIZMO REQUIRED libfizmo>=0.9.0)

# We need libmonospaceif >= 0.9.
pkg_check_modules(LIBMONOSPACEIF REQUIRED libmonospaceif>=0.9.0)

# We need ncursesw.
pkg_check_modules(NCURSESW REQUIRED ncursesw)


# Store filenames of all files-to-be-compiled in "C_FILES"
set (c_sources
  src/fizmo-ncursesw/fizmo-ncursesw.c
  src/locales/fizmo_ncursesw_locales.c
  src/locales/locale_data.c
  src/locales/locale_data.h)


# Tell CMake we're going to build an executable "fizmo-ncurses" with the
# given ".c" files, and specify libs and includes.
add_executable(fizmo-ncursesw ${c_sources})
target_include_directories(${PROJECT_NAME} PUBLIC ${LIBFIZMO_INCLUDE_DIRS}
  ${LIBMONOSPACEIF_INCLUDE_DIRS} ${NCURSESW_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PUBLIC ${LIBFIZMO_LIBDIR}
  ${LIBMONOSPACEIF_LIBDIR} ${NCURSESW_LIBDIR})
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBFIZMO_LIBRARIES}
  ${LIBMONOSPACEIF_LIBRARIES} ${NCURSESW_LIBRARIES})


install(TARGETS ${PROJECT_NAME})

