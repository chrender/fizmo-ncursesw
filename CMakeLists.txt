
cmake_minimum_required(VERSION 3.10)

include(ExternalProject)
include(FindPkgConfig)

project(fizmo-ncursesw
  LANGUAGES C
  HOMEPAGE_URL https://fizmo.spellbreaker.org
  DESCRIPTION "fizmo ncursesw frontend"
  VERSION 0.9.0)

option(ENABLE_TRACING "Enable tracing" OFF)
if (ENABLE_TRACING)
  add_definitions(-DENABLE_TRACING)
endif()

option(ENABLE_READCHAR_VIA_TERMIOS "Enable readchar via termios" ON)
if (ENABLE_READCHAR_VIA_TERMIOS)
  add_definitions(-DENABLE_READCHAR_VIA_TERMIOS)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBFIZMO REQUIRED libfizmo>=0.9.0)
pkg_check_modules(LIBMONOSPACEIF REQUIRED libmonospaceif>=0.9.0)
pkg_check_modules(NCURSESW REQUIRED ncursesw)

include_directories(${LIBFIZMO_INCLUDE_DIRS})
include_directories(${LIBMONOSPACEIF_INCLUDE_DIRS})
include_directories(${NCURSESW_INCLUDE_DIRS})

message("LIBFIZMO_INCLUDE_DIRS: ${LIBFIZMO_INCLUDE_DIRS}")

link_directories(${LIBFIZMO_LIBDIR})
link_directories(${LIBMONOSPACEIF_LIBDIR})
link_directories(${NCURSESW_LIBDIR})

set (MyCSources
  src/fizmo-ncursesw/fizmo-ncursesw.c
  src/locales/fizmo_ncursesw_locales.c
  src/locales/locale_data.c
  src/locales/locale_data.h)

add_executable(fizmo-ncursesw ${MyCSources})

message("LIBFIZMO_LIBDIR: ${LIBFIZMO_LIBDIR}")

target_link_libraries(${PROJECT_NAME} ${LIBFIZMO_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${LIBMONOSPACEIF_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${NCURSESW_LIBRARIES})


# TODO: install target, including manpage.

