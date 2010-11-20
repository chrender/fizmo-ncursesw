
/* fizmo-ncursesw.c
 *
 * This file is part of fizmo.
 *
 * Copyright (c) 2009-2010 Christoph Ender.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#define _XOPEN_SOURCE_EXTENDED 1 // for cchar_t and wcval

// Note: Colors may require --enable-ext-colors ?

#include <locale.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <signal.h>
#include <ncursesw/ncurses.h>

#include <interpreter/fizmo.h>
#include <interpreter/streams.h>
#include <interpreter/config.h>
#include <tools/i18n.h>
#include <tools/tracelog.h>
#include <interpreter/filelist.h>
#include <interpreter/wordwrap.h>
#include <tools/z_ucs.h>
#include <tools/unused.h>
#include <screen_interface/screen_cell_interface.h>
#include <cell_interface/cell_interface.h>

#ifdef SOUND_INTERFACE_INCLUDE_FILE
#include SOUND_INTERFACE_INCLUDE_FILE
#endif /* SOUND_INTERFACE_INCLUDE_FILE */

#include "../locales/fizmo_ncursesw_locales.h"

#define FIZMO_NCURSESW_VERSION "0.7.0-b3"

#ifdef ENABLE_X11_IMAGES
#include "../drilbo/drilbo.h"
#include "../drilbo/drilbo-x11.h"
#endif //ENABLE_X11_IMAGES

#define NCURSESW_COLOUR_CLASS_FOREGROUND 0
#define NCURSESW_COLOUR_CLASS_BACKGROUND 1
#define NCURSESW_WCHAR_T_BUF_SIZE 64
#define NCURSESW_Z_UCS_BUF_SIZE 32
#define NCURSESW_OUTPUT_CHAR_BUF_SIZE 80

static char* interface_name = "ncursesw";
static int ncursesw_if_signalling_pipe[2];
static struct sigaction default_sigaction;
static struct itimerval timerval;
static struct itimerval empty_timerval;
static bool use_xterm_title = false;
static char *fizmo_locale = DEFAULT_LOCALE;
static int ncursesw_argc;
static char **ncursesw_argv;
static bool use_bold_for_bright_foreground = false;
static bool use_blink_for_bright_background = false;
static bool dont_update_story_list_on_start = false;
static bool directory_was_searched = false;
static WORDWRAP *infowin_output_wordwrapper;
static WINDOW *infowin;
static z_ucs *infowin_more, *infowin_back;
static int infowin_height, infowin_width;
static int infowin_topindex;
static int infowin_lines_skipped;
static int infowin_skip_x;
static bool infowin_full = false;
static wchar_t wchar_t_buf[NCURSESW_WCHAR_T_BUF_SIZE];
static z_ucs z_ucs_t_buf[NCURSESW_Z_UCS_BUF_SIZE];
static char output_char_buf[NCURSESW_OUTPUT_CHAR_BUF_SIZE];
//static z_colour ncursesw_custom_foreground_colour = Z_COLOUR_UNDEFINED;
//static z_colour ncursesw_custom_background_colour = Z_COLOUR_UNDEFINED;
//static z_colour ncursesw_current_foreground_colour = Z_COLOUR_UNDEFINED;
//static z_colour ncursesw_current_background_colour = Z_COLOUR_UNDEFINED;
//static z_colour default_foreground_colour = Z_COLOUR_WHITE;
//static z_colour default_background_colour = Z_COLOUR_BLACK;
//static z_colour ncursesw_interface_default_foreground_colour = -1;
//static z_colour ncursesw_interface_default_background_colour = -1;
static bool ncursesw_interface_open = false;
static int n_color_pairs_in_use;
static int n_color_pairs_availabe;
static bool color_initialized = false;
// This array contains (n_color_pairs_in_use) elements. The first element
// contains the number of the color pair the was selected last, the
// second element the color pair used before that. The array is used in
// case the Z-Code tries to use more color pairs than the terminal can
// provide. In such a case, the color pair that has not been used for
// a longer time than all others is recycled.
static short *color_pair_usage;
static int ncursesw_interface_screen_height = -1;
static int ncursesw_interface_screen_width = -1;
//static uint32_t ncursesw_current_colour_code;
static attr_t ncursesw_no_attrs = 0;
//static short ncursesw_no_color = 0;
static wchar_t ncursesw_setcchar_init_string[2];
static bool dont_allocate_new_colour_pair = false;
static bool timer_active = true;

#ifdef ENABLE_X11_IMAGES
static z_image *frontispiece;
static bool enable_x11_graphics = false;
#endif // ENABLE_X11_IMAGES

/*
static void ncursesw_do_nothing(void *UNUSED(dummy_parameter))
{
}
*/


static z_ucs *z_ucs_string_to_wchar_t(wchar_t *dest, z_ucs *src,
    size_t max_dest_len)
{
  if (max_dest_len < 2)
  {
    return NULL;
  }

  while (*src != 0)
  {
    if (max_dest_len == 1)
    {
      *dest = L'\0';
      return src;
    }

    //TRACE_LOG("converting %c to wchar_t.\n", (char)*src);

    *dest = (wchar_t)*src;

    dest++;
    src++;
    max_dest_len--;
  }

  *dest = L'\0';
  return NULL;
}


static void infowin_z_ucs_output_wordwrap_destination(z_ucs *z_ucs_output,
    void *UNUSED(dummyparameter))
{
  z_ucs *ptr;
  int y,x;

  if (infowin_full == true)
    return;

  if (infowin_lines_skipped < infowin_topindex)
  {
    while (
        ((ptr = z_ucs_chr(z_ucs_output, Z_UCS_NEWLINE)) != NULL)
        ||
        ((long)z_ucs_len(z_ucs_output) >= infowin_width - infowin_skip_x)
        )
    {
      if (ptr != NULL)
      {
        if (ptr - z_ucs_output >= infowin_width - infowin_skip_x)
          z_ucs_output += infowin_width - infowin_skip_x;
        else
          z_ucs_output = ptr + 1;
      }
      else
        z_ucs_output += infowin_width - infowin_skip_x;

      if (++infowin_lines_skipped == infowin_topindex)
        break;
    }

    infowin_skip_x = z_ucs_len(z_ucs_output);

    if (infowin_lines_skipped < infowin_topindex)
      return;
  }

  if (infowin_lines_skipped == infowin_topindex)
  {
    while (*z_ucs_output == Z_UCS_NEWLINE)
      z_ucs_output++;
    infowin_lines_skipped++;
  }

  getyx(infowin, y, x);
  if (y >= infowin_height)
  {
    infowin_full = true;
    waddstr(infowin, "[");
    z_ucs_output = infowin_more;
  }

  //wprintw(infowin, "%d,%d\n", infowin_lines_skipped, infowin_topindex);
  while (z_ucs_output != NULL)
  {
    z_ucs_output = z_ucs_string_to_wchar_t(
        wchar_t_buf,
        z_ucs_output,
        NCURSESW_WCHAR_T_BUF_SIZE);

    // Ignore errors, since output on the last line always causes
    // ERR to be returned.
    waddwstr(infowin, wchar_t_buf);
  }

  if (infowin_full == true)
    waddstr(infowin, "]");
}


/*
static struct wordwrap_target ncursesw_wrapper_infowin_target =
{
  &infowin_z_ucs_output_wordwrap_destination,
  &ncursesw_do_nothing,
  &ncursesw_do_nothing
};
*/


/*
static short color_name_to_curses_colour(char *colour_name)
{
  // Keep in sync with char* z_colour_names[] at the top.

  if      (strcasecmp(colour_name, "black") == 0)   { return COLOR_BLACK;   }
  else if (strcasecmp(colour_name, "red") == 0)     { return COLOR_RED;     }
  else if (strcasecmp(colour_name, "green") == 0)   { return COLOR_GREEN;   }
  else if (strcasecmp(colour_name, "yellow") == 0)  { return COLOR_YELLOW;  }
  else if (strcasecmp(colour_name, "blue") == 0)    { return COLOR_BLUE;    }
  else if (strcasecmp(colour_name, "magenta") == 0) { return COLOR_MAGENTA; }
  else if (strcasecmp(colour_name, "cyan") == 0)    { return COLOR_CYAN;    }
  else if (strcasecmp(colour_name, "white") == 0)   { return COLOR_WHITE;   }
  else                                              { return -1;            }
}
*/


static z_colour ncursesw_curses_to_z_colour(short curses_color)
{
  switch (curses_color)
  {
    case COLOR_BLACK:   return Z_COLOUR_BLACK;
    case COLOR_RED:     return Z_COLOUR_RED;
    case COLOR_GREEN:   return Z_COLOUR_GREEN;
    case COLOR_YELLOW:  return Z_COLOUR_YELLOW;
    case COLOR_BLUE:    return Z_COLOUR_BLUE;
    case COLOR_MAGENTA: return Z_COLOUR_MAGENTA;
    case COLOR_CYAN:    return Z_COLOUR_CYAN;
    case COLOR_WHITE:   return Z_COLOUR_WHITE;
  }

  return -1;
}


static void goto_yx(int y, int x)
{
  TRACE_LOG("move: %d,%d\n", y, x);
  move(y-1, x-1);
}


static void ncursesw_fputws(wchar_t *str, FILE *out)
{
#ifdef __CYGWIN__
  while(*str != 0)
    fputc(wctob(*(str++)), out);
#else
  fputws(str, out);
#endif
}


static void z_ucs_output(z_ucs *output)
{
  cchar_t wcval;
  int errorcode;

  TRACE_LOG("Interface-Output(%d): \"", ncursesw_interface_open);
  TRACE_LOG_Z_UCS(output);
  TRACE_LOG("\".\n");

  if (ncursesw_interface_open == false)
  {
    while (*output != 0)
    {
      zucs_string_to_utf8_string(
          output_char_buf,
          &output,
          NCURSESW_OUTPUT_CHAR_BUF_SIZE);

      fputs(output_char_buf, stdout);
    }
    fflush(stdout);
  }
  else
  {
    ncursesw_setcchar_init_string[1] = L'\0';

    while (*output != '\0')
    {
      ncursesw_setcchar_init_string[0] = *output;
      //TRACE_LOG("%c/%d\n", *output, *output);

      errorcode = setcchar(
          &wcval,
          ncursesw_setcchar_init_string,
          ncursesw_no_attrs,
          0,
          NULL);

      add_wch(&wcval);

      output++;
    }
  }
}


static bool is_input_timeout_available()
{
  return true;
}


/*
static void start_timer(int timeout_millis)
{
  if (timeout_millis > 0)
  {
    TRACE_LOG("input timeout: %d ms. (%d/%d)\n", timeout_millis,
        timeout_millis - (timeout_millis % 1000), 
        (timeout_millis % 1000) * 1000);
    timerval.it_value.tv_sec = timeout_millis - (timeout_millis % 1000);
    timerval.it_value.tv_usec = (timeout_millis % 1000) * 1000;
    timer_active = true;
    setitimer(ITIMER_REAL, &timerval, NULL);
  }
}


static void stop_timer()
{
  timer_active = false;
  setitimer(ITIMER_REAL, &empty_timerval, NULL);
}
*/


static void turn_on_input()
{
}


static void turn_off_input()
{
}


static char* get_interface_name()
{
  return interface_name;
}


static bool is_colour_available()
{
  return has_colors();
}


static bool is_bold_face_available()
{
  return true;
}


static bool is_italic_available()
{
  return true;
}


static void print_startup_syntax()
{
  int i;
  char **available_locales = get_available_locale_names();
  endwin();

  streams_latin1_output("\n");
  i18n_translate(
      fizmo_ncursesw_module_name,
      i18n_ncursesw_USAGE_DESCRIPTION);
  streams_latin1_output("\n\n");

  i18n_translate(
      fizmo_ncursesw_module_name,
      i18n_ncursesw_FIZMO_NCURSESW_VERSION_P0S, FIZMO_NCURSESW_VERSION);
  streams_latin1_output("\n");
  i18n_translate(
      fizmo_ncursesw_module_name,
      i18n_ncursesw_LIBFIZMO_VERSION_P0S,
      FIZMO_VERSION);
  streams_latin1_output("\n");
  i18n_translate(
      fizmo_ncursesw_module_name,
      i18n_ncursesw_LIBCELLINTERFACE_VERSION_P0S,
      get_screen_cell_interface_version());
  streams_latin1_output("\n");
  if (active_sound_interface != NULL)
  {
    streams_latin1_output(active_sound_interface->get_interface_name());
    streams_latin1_output(" ");
    streams_latin1_output("version ");
    streams_latin1_output(active_sound_interface->get_interface_version());
    streams_latin1_output(".\n");
  }
  streams_latin1_output("\n");

  i18n_translate(
      fizmo_ncursesw_module_name,
      i18n_ncursesw_LOCALES_AVAILIABLE);
  streams_latin1_output(" ");

  i = 0;
  while (available_locales[i] != NULL)
  {
    if (i != 0)
      streams_latin1_output(", ");

    streams_latin1_output(available_locales[i]);
    free(available_locales[i]);
    i++;
  }
  free(available_locales);
  streams_latin1_output(".\n");

  i18n_translate(
      fizmo_ncursesw_module_name,
      i18n_ncursesw_COLORS_AVAILABLE);
  streams_latin1_output(": ");

  for (i=0; i<NOF_Z_COLOURS; i++)
  {
    if (i != 0)
      streams_latin1_output(", ");
    streams_latin1_output(z_colour_names[i]);
  }
  streams_latin1_output(".\n\n");

  i18n_translate(
      fizmo_ncursesw_module_name,
      i18n_ncursesw_VALID_OPTIONS_ARE);
  streams_latin1_output("\n");

  streams_latin1_output( " -l,  --set-locale: ");
  i18n_translate(
      fizmo_ncursesw_module_name,
      i18n_ncursesw_SET_LOCALE_NAME_FOR_INTERPRETER_MESSAGES);
  streams_latin1_output("\n");

  streams_latin1_output( " -p,  --predictable: ");
  i18n_translate(
      fizmo_ncursesw_module_name,
      i18n_ncursesw_START_WITH_RANDOM_GENERATOR_IN_PREDICTABLE_MODE);
  streams_latin1_output("\n");

  streams_latin1_output( " -fp, --force-predictable: ");
  i18n_translate(
      fizmo_ncursesw_module_name,
      i18n_ncursesw_FORCE_RANDOM_GENERATOR_IN_PREDICTABLE_MODE);
  streams_latin1_output("\n");

  streams_latin1_output( " -st, --start-transcript: ");
  i18n_translate(
      fizmo_ncursesw_module_name,
      i18n_ncursesw_START_GAME_WITH_TRANSCRIPT_ENABLED);
  streams_latin1_output("\n");

  streams_latin1_output( " -rc, --record-commands: ");
  i18n_translate(
      fizmo_ncursesw_module_name,
      i18n_ncursesw_START_GAME_WITH_RECORDING_COMMANDS);
  streams_latin1_output("\n");

  streams_latin1_output( " -if, --input-file: ");
  i18n_translate(
      fizmo_ncursesw_module_name,
      i18n_ncursesw_START_GAME_WITH_INPUT_FROM_FILE);
  streams_latin1_output("\n");

  streams_latin1_output( " -f,  --foreground-color: ");
  i18n_translate(
      fizmo_ncursesw_module_name,
      i18n_ncursesw_SET_FOREGROUND_COLOR);
  streams_latin1_output("\n");

  streams_latin1_output( " -b,  --background-color: ");
  i18n_translate(
      fizmo_ncursesw_module_name,
      i18n_ncursesw_SET_BACKGROUND_COLOR);
  streams_latin1_output("\n");

  streams_latin1_output( " -bf, --bold-for-bright-foreground: ");
  i18n_translate(
      fizmo_ncursesw_module_name,
      i18n_ncursesw_USE_BOLD_FOR_BRIGHT_FOREGROUND_COLORS);
  streams_latin1_output("\n");

  streams_latin1_output( " -bb, --blink-for-bright-background: ");
  i18n_translate(
      fizmo_ncursesw_module_name,
      i18n_ncursesw_USE_BLINK_FOR_BRIGHT_BACKGROUND_COLORS);
  streams_latin1_output("\n");

  streams_latin1_output( " -nc, --dont-use-colors: ");
  i18n_translate(
      fizmo_ncursesw_module_name,
      i18n_ncursesw_DONT_USE_COLORS);
  streams_latin1_output("\n");

  streams_latin1_output( " -ec, --enable-colors: ");
  i18n_translate(
      fizmo_ncursesw_module_name,
      i18n_ncursesw_ENABLE_COLORS);
  streams_latin1_output("\n");

  streams_latin1_output( " -lm, --left-margin: " );
  i18n_translate(
      fizmo_ncursesw_module_name,
      i18n_ncursesw_SET_LEFT_MARGIN_SIZE);
  streams_latin1_output("\n");

  streams_latin1_output( " -rm, --right-margin: " );
  i18n_translate(
      fizmo_ncursesw_module_name,
      i18n_ncursesw_SET_RIGHT_MARGIN_SIZE);
  streams_latin1_output("\n");

  streams_latin1_output( " -um, --umem: ");
  i18n_translate(
      fizmo_ncursesw_module_name,
      i18n_ncursesw_USE_UMEM_FOR_SAVEGAMES);
  streams_latin1_output("\n");

  streams_latin1_output( " -x,  --enable-xterm-graphics: ");
  i18n_translate(
      fizmo_ncursesw_module_name,
      i18n_ncursesw_ENABLE_XTERM_GRAPHICS);
  streams_latin1_output("\n");

  streams_latin1_output( " -xt, --enable-xterm-title: ");
  i18n_translate(
      fizmo_ncursesw_module_name,
      i18n_ncursesw_USE_XTERM_TITLE);
  streams_latin1_output("\n");

  streams_latin1_output( " -s8, --force-8bit-sound: ");
  i18n_translate(
      fizmo_ncursesw_module_name,
      i18n_ncursesw_FORCE_8BIT_SOUND);
  streams_latin1_output("\n");

  streams_latin1_output( " -ds, --disable-sound: ");
  i18n_translate(
      fizmo_ncursesw_module_name,
      i18n_ncursesw_DISABLE_SOUND);
  streams_latin1_output("\n");

  streams_latin1_output( " -t,  --set-tandy-flag: ");
  i18n_translate(
      fizmo_ncursesw_module_name,
      i18n_ncursesw_SET_TANDY_FLAG);
  streams_latin1_output("\n");

  streams_latin1_output( " -nu, --dont-update-story-list: " );
  i18n_translate(
      fizmo_ncursesw_module_name,
      i18n_ncursesw_DONT_UPDATE_STORY_LIST_ON_START);
  streams_latin1_output("\n");

  streams_latin1_output( " -u,  --update-story-list: " );
  i18n_translate(
      fizmo_ncursesw_module_name,
      i18n_ncursesw_UPDATE_STORY_LIST_ON_START);
  streams_latin1_output("\n");

  streams_latin1_output( " -s,  --search: " );
  i18n_translate(
      fizmo_ncursesw_module_name,
      i18n_ncursesw_SEARCH_DIRECTORY);
  streams_latin1_output("\n");

  streams_latin1_output( " -rs, --recursively-search: ");
  i18n_translate(
      fizmo_ncursesw_module_name,
      i18n_ncursesw_RECURSIVELY_SEARCH_DIRECTORY);
  streams_latin1_output("\n");

  streams_latin1_output( " -sy, --sync-transcript: ");
  i18n_translate(
      fizmo_ncursesw_module_name,
      i18n_ncursesw_SYNC_TRANSCRIPT);
  streams_latin1_output("\n");

  streams_latin1_output( " -h,  --help: ");
  i18n_translate(
      fizmo_ncursesw_module_name,
      i18n_ncursesw_SHOW_HELP_MESSAGE_AND_EXIT);
  streams_latin1_output("\n");

  //set_configuration_value("locale", fizmo_locale, "fizmo");

  streams_latin1_output("\n");
}


static int parse_config_parameter(char *key, char *value)
{
  if (strcasecmp(key, "enable-xterm-title") == 0)
  {
    if (value == NULL)
      return -1;
    else if ((strcasecmp(value, "true")==0) || (strcasecmp(value, "yes")==0))
    {
      use_xterm_title = true;
      return 0;
    }
    else if ((strcasecmp(value, "false")==0) || (strcasecmp(value, "no")==0))
    {
      use_xterm_title = false;
      return 0;
    }
    else
      return -1;
  }
  else if (strcasecmp(key, "dont-update-story-list") == 0)
  {
    if (value == NULL)
      return -1;
    else if ((strcasecmp(value, "true")==0) || (strcasecmp(value, "yes")==0))
    {
      dont_update_story_list_on_start = true;
      return 0;
    }
    else
      return -1;
  }
  else
  {
    return -2;
  }
}


static int z_to_curses_colour(z_colour z_colour_to_convert,
    int UNUSED(colour_class))
{
  switch (z_colour_to_convert)
  {
    case Z_COLOUR_BLACK:   return COLOR_BLACK;
    case Z_COLOUR_RED:     return COLOR_RED;
    case Z_COLOUR_GREEN:   return COLOR_GREEN;
    case Z_COLOUR_YELLOW:  return COLOR_YELLOW;
    case Z_COLOUR_BLUE:    return COLOR_BLUE;
    case Z_COLOUR_MAGENTA: return COLOR_MAGENTA;
    case Z_COLOUR_CYAN:    return COLOR_CYAN;
    case Z_COLOUR_WHITE:   return COLOR_WHITE;
    default:               return -1;
  }
}


static void dump_col_usage()
{
  int j;
  int pair;
  short pair_foreground, pair_background;

  for (j=0; j<n_color_pairs_in_use; j++)
  {
    pair = color_pair_usage[j];
    pair_content(pair, &pair_foreground, &pair_background);
    TRACE_LOG("col-usage[%02d]: %d, %d/%d\n", j, pair,
        ncursesw_curses_to_z_colour(pair_foreground),
        ncursesw_curses_to_z_colour(pair_background));
  }
}


// curses provides a constant named COLOR_PAIRS telling us the maximum
// allowed number of color pairs on this terminal. This may be less pairs
// than the great implementor whose program we're interpreting desired,
// so we'll use the available pairs in a round-about fashion. Thus we
// can keep the latest defined color pairs as long as possible and
// hopefully the screen in the best possible look.
// The maximum number of color pairs useful to us is 121 since section
// 8.3.1 of the Z-Spec defines the colors black, red, green, yellow, blue,
// magenta, cyan, white and three different shades of grey. Every possible
// combination results in 11*11 = 121 color pairs.
static short get_color_pair(z_colour z_foreground_colour,
    z_colour z_background_colour)
{
  short curses_foreground_color, curses_background_color;
  short pair_foreground, pair_background;
  short new_color_pair_number;
  int i,j;

  TRACE_LOG("Looking for infocom color pair %d / %d.\n",
      z_foreground_colour, z_background_colour);

  // Convert color codes from infocom to curses.
  curses_foreground_color
    = z_to_curses_colour(
        z_foreground_colour, NCURSESW_COLOUR_CLASS_FOREGROUND);

  curses_background_color
    = z_to_curses_colour(
        z_background_colour, NCURSESW_COLOUR_CLASS_BACKGROUND);

  TRACE_LOG("Looking for color pair %d / %d.\n",
      curses_foreground_color, curses_background_color);

  TRACE_LOG("n_color pairs in use: %d.\n", n_color_pairs_in_use);
  // First, check if we already have allocated this combination. We'll
  // start with index 1 since pair #0 is used for curses' internals.
  for (i=1; i<=n_color_pairs_in_use; i++)
  {
    pair_content(i, &pair_foreground, &pair_background);
    TRACE_LOG("Color pair %d: %d / %d.\n",i,pair_foreground, pair_background);

    if (
        (pair_foreground == curses_foreground_color)
        &&
        (pair_background == curses_background_color)
       )
    {
      TRACE_LOG("Found existing color pair with index %d.\n", i);

      if (n_color_pairs_availabe != 121)
      {
        // In case we're working with a limited number of colors we'll
        // have to update the color_pair_usage array. We'll put the index
        // of the color pair we've just selected to the front of the array
        // to notify that this is the latest used pair.

        // We only have to do something in case the pair is not already
        // in front.
        if (color_pair_usage[0] != i)
        {
          // First, advance j until we find i's index.
          for (j=0; j<n_color_pairs_in_use; j++)
            if (color_pair_usage[j] == i)
              break;

          TRACE_LOG("Found color pair is at usage position %d.\n", j);

          // Now, we'll move backwards, moving the array one index
          // "downwards", thus overwriting i's entry and making space
          // for it on top of the array again.
          for (; j>=0; j--)
            color_pair_usage[j] = color_pair_usage[j-1];

          color_pair_usage[0] = i;
        }
      }

      dump_col_usage();
      return i;
    }
  }

  TRACE_LOG("No existing color pair found.\n");

  // In case we arrive here we have not returned and thus the desired
  // color pair was not found.

  if (bool_equal(dont_allocate_new_colour_pair, true))
    return -1;

  if (n_color_pairs_in_use < n_color_pairs_availabe)
  {
    new_color_pair_number = n_color_pairs_in_use + 1;
    TRACE_LOG("Allocating new color pair %d.\n", new_color_pair_number);
    n_color_pairs_in_use++;
    if (n_color_pairs_availabe != 121)
    {
      memmove(&(color_pair_usage[1]), color_pair_usage,
          (n_color_pairs_in_use-1) * sizeof(short));
      color_pair_usage[0] = new_color_pair_number;
    }
  }
  else
  {
    new_color_pair_number = color_pair_usage[n_color_pairs_in_use-1];
    memmove(&(color_pair_usage[1]), color_pair_usage,
        (n_color_pairs_in_use) * sizeof(short));
    color_pair_usage[0] = new_color_pair_number;

    TRACE_LOG("Recycling oldest color pair %d.\n", new_color_pair_number);
  }

  TRACE_LOG("initpair: %d, %d, %d\n",
      new_color_pair_number,
      curses_foreground_color,
      curses_background_color);

  if (init_pair(
        new_color_pair_number,
        curses_foreground_color,
        curses_background_color) == ERR)
    i18n_translate_and_exit(
        fizmo_ncursesw_module_name,
        i18n_ncursesw_FUNCTION_CALL_P0S_ABORTED_DUE_TO_ERROR,
        -0x2000,
        "init_pair");

  TRACE_LOG("n_color pairs in use: %d.\n", n_color_pairs_in_use);

  dump_col_usage();
  return new_color_pair_number;
}


static void initialize_colors()
{
  start_color();

  // After implementing almost everything and assuming that the color pair
  // #0 always has the default colors (and running into strange problems)
  // I found the following note: "Note that color-pair 0 is reserved for
  // use by curses and should not be changed or used in application programs."
  // Thus, color pair 0 is not used here and the number of availiable colors
  // is set to COLOR_PAIRS - 1.
  //n_color_pairs_availabe = COLOR_PAIRS;
  n_color_pairs_availabe = COLOR_PAIRS - 1;

  if (n_color_pairs_availabe > 121)
    n_color_pairs_availabe = 121;

  TRACE_LOG("%d color pairs are availiable.\n", n_color_pairs_availabe);

  if (n_color_pairs_availabe < 121)
  {
    // In case not all color combinations are available, we'll have to
    // keep track when the colors were used last.
    color_pair_usage
      = (short*)fizmo_malloc(sizeof(short) * n_color_pairs_availabe);
    color_pair_usage[0] = 0;
  }

  n_color_pairs_in_use = 0;
  color_initialized = true;
}


static void set_colour(z_colour foreground, z_colour background)
{
  short color_pair_number;
  //attr_t attrs;

  TRACE_LOG("new colors: %d, %d\n", foreground, background);

  if (color_initialized == false)
    initialize_colors();

  if ((color_pair_number = get_color_pair(foreground, background))
      == -1)
  {
    if (bool_equal(dont_allocate_new_colour_pair, true))
      return;
    else
      i18n_translate_and_exit(
          fizmo_ncursesw_module_name,
          i18n_ncursesw_FUNCTION_CALL_P0S_ABORTED_DUE_TO_ERROR,
          -1,
          "curses_if_get_color_pair");
  }

  TRACE_LOG("setting colour pair %d.\n", color_pair_number);

  if (color_set(color_pair_number, NULL) == ERR)
    i18n_translate_and_exit(
        fizmo_ncursesw_module_name,
        i18n_ncursesw_FUNCTION_CALL_P0S_ABORTED_DUE_TO_ERROR,
        -0x2052,
        "color_set");

  bkgdset(' ' | COLOR_PAIR(color_pair_number));
}


#ifdef ENABLE_X11_IMAGES
static void display_X11_image_window(int image_no)
{
  struct z_story_blorb_image *image_blorb_index;

  image_blorb_index = get_image_blorb_index(active_z_story, image_no);

  /*
  if (x11_display == NULL)
    init_x11_display();
    */

  fseek(
      active_z_story->blorb_file,
      image_blorb_index->blorb_offset,
      SEEK_SET);

  if (image_blorb_index->type == Z_BLORB_IMAGE_PNG)
  {
    return;
  }
  else if (image_blorb_index->type == Z_BLORB_IMAGE_JPEG)
  {
    return;
  }
  else
  {
    //FIXME: Quit / Warn of unsupported (undefined?) image type.
    return;
  }
}
#endif // ENABLE_X11_IMAGES


static void link_interface_to_story(struct z_story *UNUSED(story))
{
  int flags;

  initscr();
  keypad(stdscr, TRUE);
  cbreak();
  noecho();

  // Create a new signalling pipe. This pipe is used by a select call to
  // detect an incoming time-signal for the input routine.
  if (pipe(ncursesw_if_signalling_pipe) != 0)
    i18n_translate_and_exit(
        fizmo_ncursesw_module_name,
        i18n_ncursesw_FUNCTION_CALL_P0S_RETURNED_ERROR_P1D_P2S,
        -0x2016,
        "pipe",
        errno,
        strerror(errno));

  // Get the current flags for the read-end of the pipe.
  if ((flags = fcntl(ncursesw_if_signalling_pipe[0], F_GETFL, 0)) == -1)
    i18n_translate_and_exit(
        fizmo_ncursesw_module_name,
        i18n_ncursesw_FUNCTION_CALL_P0S_RETURNED_ERROR_P1D_P2S,
        -0x2017,
        "fcntl / F_GETFL",
        errno,
        strerror(errno));

  // Add the nonblocking flag the read-end of the pipe, thus making incoming
  // input "visible" at once without having to wait for a newline.
  if ((fcntl(ncursesw_if_signalling_pipe[0], F_SETFL, flags|O_NONBLOCK)) == -1)
    i18n_translate_and_exit(
        fizmo_ncursesw_module_name,
        i18n_ncursesw_FUNCTION_CALL_P0S_RETURNED_ERROR_P1D_P2S,
        -0x2018,
        "fcntl / F_SETFL",
        errno,
        strerror(errno));

  ncursesw_interface_open = true;

#ifdef ENABLE_X11_IMAGES
  if (active_z_story->frontispiece_image_no >= 0)
    display_X11_image_window(active_z_story->frontispiece_image_no);
#endif // ENABLE_X11_IMAGES
}


static void reset_interface()
{
}


static int ncursw_close_interface(z_ucs *error_message)
{
  z_ucs *ptr;

  TRACE_LOG("Closing signalling pipes.\n");

  close(ncursesw_if_signalling_pipe[1]);
  close(ncursesw_if_signalling_pipe[0]);

  endwin();

  ncursesw_interface_open = false;

  if (error_message != NULL)
  {
    ptr = error_message;
    while (ptr != NULL)
    {
      ptr = z_ucs_string_to_wchar_t(
          wchar_t_buf,
          ptr,
          NCURSESW_WCHAR_T_BUF_SIZE);

      ncursesw_fputws(wchar_t_buf, stderr);
    }
  }

  return 0;
}


static attr_t ncursesw_z_style_to_attr_t(int16_t style_data)
{
  attr_t result = A_NORMAL;

  if ((style_data & Z_STYLE_REVERSE_VIDEO) != 0)
  {
    result |= A_REVERSE;
  }

  if ((style_data & Z_STYLE_BOLD) != 0)
  {
    result |= A_BOLD;
  }

  if ((style_data & Z_STYLE_ITALIC) != 0)
  {
    result |= A_UNDERLINE;
  }

  return result;
}


static void set_text_style(z_style style_data)
{
  attr_t attrs;

  TRACE_LOG("Output style, style_data %d.\n", style_data);

  attrs = ncursesw_z_style_to_attr_t(style_data);

  if ((int)attrset(attrs) == ERR)
  {
    i18n_translate_and_exit(
        fizmo_ncursesw_module_name,
        i18n_ncursesw_FUNCTION_CALL_P0S_ABORTED_DUE_TO_ERROR,
        -0x2fff,
        "wattrset");
  }
}


static void set_font(z_font UNUSED(font_type))
{
}


static void output_interface_info()
{
  (void)i18n_translate(
      fizmo_ncursesw_module_name,
      i18n_ncursesw_FIZMO_NCURSESW_VERSION_P0S,
      FIZMO_NCURSESW_VERSION);
  (void)streams_latin1_output("\n");
}


static void refresh_screen_size()
{
  getmaxyx(
      stdscr,
      ncursesw_interface_screen_height,
      ncursesw_interface_screen_width);
}


static int get_screen_width()
{
  if (ncursesw_interface_screen_width == -1)
    refresh_screen_size();
  return ncursesw_interface_screen_width;
}


static int get_screen_height()
{
  if (ncursesw_interface_screen_height == -1)
    refresh_screen_size();
  return ncursesw_interface_screen_height;
}


static void ncursesw_if_catch_signal(int sig_num)
{
  int bytes_written = 0;
  int ret_val;
  int ncursesw_if_write_buffer;

  // Note:
  // I think TRACE_LOGs in this function may cause a deadlock in case
  // they're called while a fflush for the tracelog is already underway.

  ncursesw_if_write_buffer = sig_num;

  //TRACE_LOG("Caught signal %d.\n", sig_num);

  while ((size_t)bytes_written < sizeof(int))
  {
    ret_val = write(
        ncursesw_if_signalling_pipe[1],
        &ncursesw_if_write_buffer,
        sizeof(int));

    if (ret_val == -1)
      i18n_translate_and_exit(
          fizmo_ncursesw_module_name,
          i18n_ncursesw_FUNCTION_CALL_P0S_RETURNED_ERROR_P1D_P2S,
          -0x2023,
          "write",
          errno,
          strerror(errno));

    bytes_written += ret_val;
  }

  //TRACE_LOG("Catch finished.\n");
}


static int get_next_event(z_ucs *z_ucs_input, int timeout_millis)
{
  int max_filedes_number_plus_1;
  int select_retval;
  fd_set input_selectors;
  int result = -1;
  int input_return_code;
  bool input_should_terminate = false;
  int bytes_read;
  wint_t input;
  int read_retval;
  int new_signal;
  //int screen_height, screen_width;

  FD_ZERO(&input_selectors);
  FD_SET(STDIN_FILENO, &input_selectors);
  FD_SET(ncursesw_if_signalling_pipe[0], &input_selectors);

  max_filedes_number_plus_1
    = (STDIN_FILENO < ncursesw_if_signalling_pipe[0]
        ? ncursesw_if_signalling_pipe[0]
        : STDIN_FILENO) + 1;

  if (timeout_millis > 0)
  {
    TRACE_LOG("input timeout: %d ms. (%d/%d)\n", timeout_millis,
        timeout_millis - (timeout_millis % 1000), 
        (timeout_millis % 1000) * 1000);
    timerval.it_value.tv_sec = timeout_millis - (timeout_millis % 1000);
    timerval.it_value.tv_usec = (timeout_millis % 1000) * 1000;
    timer_active = true;
    setitimer(ITIMER_REAL, &timerval, NULL);
  }

  while (input_should_terminate == false)
  {
    TRACE_LOG("current errno: %d.\n", errno);
    TRACE_LOG("setting up selectors.\n");

    FD_ZERO(&input_selectors);
    FD_SET(STDIN_FILENO, &input_selectors);
    FD_SET(ncursesw_if_signalling_pipe[0], &input_selectors);

    select_retval = select(
        max_filedes_number_plus_1,
        &input_selectors,
        NULL,
        NULL,
        NULL);

    if (select_retval > 0)
    {
      TRACE_LOG("select_retval > 0.\n");
      TRACE_LOG("current errno: %d.\n", errno);

      // something has changed in one of out input pipes.
      if (FD_ISSET(STDIN_FILENO, &input_selectors))
      {
        // some user input is waiting. we'll read until getch() returned
        // err, meaning "no more input availiable" in the nonblocking mode.
        input_return_code = get_wch(&input);
        if (input_return_code == ERR)
        {
        }
        else if (input_return_code == KEY_CODE_YES)
        {
          if (input == KEY_UP)
            result = EVENT_WAS_CODE_CURSOR_UP;
          else if (input == KEY_DOWN)
            result = EVENT_WAS_CODE_CURSOR_DOWN;
          else if (input == KEY_RIGHT)
            result = EVENT_WAS_CODE_CURSOR_RIGHT;
          else if (input == KEY_LEFT)
            result = EVENT_WAS_CODE_CURSOR_LEFT;
          else if (input == KEY_NPAGE)
            result = EVENT_WAS_CODE_PAGE_DOWN;
          else if (input == KEY_PPAGE)
            result = EVENT_WAS_CODE_PAGE_UP;
        }
        else if (input_return_code == OK)
        {
          if (input == 127)
            result = EVENT_WAS_CODE_BACKSPACE;
          else
          {
            result = EVENT_WAS_INPUT;
            *z_ucs_input = (z_ucs)input;
          }
        }

        input_should_terminate = true;
      }
      else if (FD_ISSET(ncursesw_if_signalling_pipe[0], &input_selectors))
      {
        TRACE_LOG("current errno: %d.\n", errno);
        // the signal handler has written to our curses_if_signalling_pipe.
        // ensure that errno is != 0 before reading from the pipe. this is
        // due to the fact that even a successful read may set errno.

        TRACE_LOG("pipe event.\n");

        if (errno != 0)
        {
          if (errno == EINTR)
          {
            errno = 0;
            continue;
          }
          else
          {
            i18n_translate_and_exit(
                fizmo_ncursesw_module_name,
                i18n_ncursesw_ERROR_P0D_OCCURED_BEFORE_READ_P1S,
                -0x203d,
                errno,
                strerror(errno));
          }
        }

        bytes_read = 0;

        while (bytes_read != sizeof(int))
        {
          read_retval = read(
              ncursesw_if_signalling_pipe[0],
              &new_signal,
              sizeof(int));

          if (read_retval == -1)
          {
            if (errno == EAGAIN)
            {
              errno = 0;
              continue;
            }
            else
            {
              i18n_translate_and_exit(
                  fizmo_ncursesw_module_name,
                  i18n_ncursesw_FUNCTION_CALL_P0S_RETURNED_ERROR_P1D_P2S,
                  -0x2041,
                  "read",
                  errno,
                  strerror(errno));
            }
          }
          else
          {
            bytes_read += read_retval;
          }
        }

        TRACE_LOG("bytes read: %d,signal code: %d\n", bytes_read, new_signal);

        if (new_signal == SIGALRM)
        {
          result = EVENT_WAS_TIMEOUT;
        }
        else if (new_signal == SIGWINCH)
        {
          TRACE_LOG("interface got SIGWINCH.\n");
          result = EVENT_WAS_WINCH;

          endwin();
          refresh();
          //getmaxyx(stdscr, screen_height, screen_width);
          refresh_screen_size();
          //TRACE_LOG("New dimensions: %dx%d.\n", screen_width, screen_height);
          //new_cell_screen_size(screen_height, screen_width);
        }
        else
        {
          i18n_translate_and_exit(
              fizmo_ncursesw_module_name,
              i18n_ncursesw_UNKNOWN_ERROR_CASE,
              -0x2041);
        }

        input_should_terminate = true;
      }
    }
    else
    {
      TRACE_LOG("select returned <=0, current errno: %d.\n", errno);
    }
  }

  sigaction(SIGWINCH, NULL, NULL);

  return result;
}


void update_screen()
{
  refresh();
}


void redraw_screen_from_scratch()
{
  redrawwin(stdscr);
  update_screen();
}


void copy_area(int dsty, int dstx, int srcy, int srcx, int height, int width)
{
  //REVISIT: Maybe implement reading whole line?
  //int errorcode;
  int ystart, yend, yincrement;
  int xstart, xend, xincrement;
  int y, x;
  cchar_t wcval;

  TRACE_LOG("copyarea:%d/%d/%d/%d/%d/%d\n", 
      srcy,
      srcx,
      dsty,
      dstx,
      height,
      width);

  if ( (width < 0) || (height < 0) )
  {
    i18n_translate_and_exit(
        fizmo_ncursesw_module_name,
        i18n_ncursesw_FUNCTION_CALL_P0S_ABORTED_DUE_TO_ERROR,
        -0x2000,
        "copy_area");
  }

  // Simply using copywin does not work, in my ncurses implementation it only
  // does the correct thing when moving things up the screen. Once dst
  // is below the src area and overlaps, copywin does not work correct
  // anymore.
  /*
  if ((errorcode = copywin(
          stdscr,
          stdscr,
          srcy - 1,
          srcx - 1,
          dsty - 1,
          dstx - 1,
          dsty + height - 2,
          dstx + width - 2,
          false)) == ERR)
  {
    i18n_translate_and_exit(
        fizmo_ncursesw_module_name,
        i18n_fizmo_FUNCTION_CALL_P0S_RETURNED_ERROR_CODE_P1D,
        -0x202b,
        "copywin",
        (long)errorcode);
  }
  */

  // To be safe, each line is copied separately in order to avoid
  // implementation-specific problems.
  if (dsty > srcy)
  {
    ystart = height - 1;
    yend = -1;
    yincrement = -1;
  }
  else
  {
    ystart = 0;
    yend = height;
    yincrement = 1;
  }

  if (dstx > srcx)
  {
    xstart = width - 1;
    xend = -1;
    xincrement = -1;
  }
  else
  {
    xstart = 0;
    xend = width;
    xincrement = 1;
  }

  for (y=ystart; y!=yend; y+=yincrement)
  {
    for (x=xstart; x!=xend; x+=xincrement)
    {
      /*
      TRACE_LOG("copychar: %d,%d -> %d,%d.\n",
          srcy + y - 1, srcx + x - 1, dsty + y - 1, dstx + x - 1);
      */

      mvin_wch(srcy + y - 1, srcx + x - 1, &wcval);
      mvadd_wch(dsty + y - 1, dstx + x - 1, &wcval);
    }
  }
}


void clear_to_eol()
{
  clrtoeol();
}


void clear_area(int startx, int starty, int xsize, int ysize)
{
  int x;

  TRACE_LOG("Clearing area %d,%d / %d,%d\n", startx, starty, xsize, ysize);

  while (ysize > 0)
  {
    x = xsize;
    while (x > 0)
    {
      mvaddch(starty + ysize - 2, startx + x - 2, ' ');
      x--;
    }
    ysize--;
  }
}


static void set_cursor_visibility(bool visible)
{
  if (ncursesw_interface_open == true)
  {
    if (visible == true)
      curs_set(1);
    else
      curs_set(0);
  }
}


static struct z_screen_cell_interface ncursesw_interface =
{
  &goto_yx,
  &z_ucs_output,
  &is_input_timeout_available,
  &turn_on_input,
  &turn_off_input,
  &get_next_event,
  &get_interface_name,
  &is_colour_available,
  &is_bold_face_available,
  &is_italic_available,
  &parse_config_parameter,
  &link_interface_to_story,
  &reset_interface,
  &ncursw_close_interface,
  &set_text_style,
  &set_colour,
  &set_font,
  &output_interface_info,
  &get_screen_width,
  &get_screen_height,
  &update_screen,
  &redraw_screen_from_scratch,
  &copy_area,
  &clear_to_eol,
  &clear_area,
  &set_cursor_visibility
};


static char *select_story_from_menu(char *fizmo_dir)
{
  z_colour foreground, background;
  int input;
  int storywin_height = -1;
  int storywin_width = -1;
  int storywin_y = 3;
  int infowin_y = 3;
  int storywin_x = 3;
  int story_title_length = -1;
  struct z_story_list *story_list;
  int i, j;
  int y, x;
  bool input_done = false;
  int selected = 0;
  int len;
  int scroll_index = 0;
  struct z_story_list_entry *entry;
  char *result;
  char *src;
  fd_set input_selectors;
  int max_filedes_number_plus_1;
  int select_retval;
  bool perform_init = true;
  int read_retval;
  int bytes_read;
  int new_signal;
  z_ucs *ptr;

  story_list
    = dont_update_story_list_on_start != true
    ? update_fizmo_story_list(fizmo_dir)
    : get_z_story_list();

  if ( (story_list == NULL) || (story_list->nof_entries < 1) )
  {
    //set_configuration_value("locale", fizmo_locale, "ncursesw");
    streams_latin1_output("\n");
    i18n_translate(
        fizmo_ncursesw_module_name,
        i18n_ncursesw_NO_STORIES_REGISTERED_PLUS_HELP);
    streams_latin1_output("\n\n");
    //set_configuration_value("locale", fizmo_locale, "fizmo");
    return NULL;
  }

  sigemptyset(&default_sigaction.sa_mask);
  default_sigaction.sa_flags = 0;
  default_sigaction.sa_handler = &ncursesw_if_catch_signal;
  sigaction(SIGWINCH, &default_sigaction, NULL);

  infowin_output_wordwrapper = wordwrap_new_wrapper(
      80,
      &infowin_z_ucs_output_wordwrap_destination,
      (void*)NULL,
      false,
      0,
      false,
      true);

  /*
  set_configuration_value(
      "locale", get_configuration_value("locale"), "ncursesw");
  */
  infowin_more = i18n_translate_to_string(
      fizmo_ncursesw_module_name,
      i18n_ncursesw_SPACE_FOR_NEXT_PAGE);
  infowin_back = i18n_translate_to_string(
      fizmo_ncursesw_module_name,
      i18n_ncursesw_B_FOR_LAST_PAGE);
  /*
  set_configuration_value(
      "locale", get_configuration_value("locale"), "fizmo");
  */

  while (input_done == false)
  {
    if (perform_init == true)
    {
      initscr();
      cbreak();
      noecho();
      keypad(stdscr, true);

      getmaxyx(stdscr, y, x);
      storywin_height = y - 5;

      storywin_width = x / 3;
      story_title_length = storywin_width - 9;

      infowin_height = storywin_height - 1;
      infowin_width = x - storywin_width - storywin_x - 9;
      infowin_topindex = 0;

      infowin = newwin(
          infowin_height + 1,
          infowin_width,
          infowin_y,
          storywin_width + storywin_x + 5);

      wordwrap_adjust_line_length(
          infowin_output_wordwrapper,
          x - storywin_width - storywin_x - 9);

      if (has_colors() == true)
      {
        start_color();

        if (
            (default_foreground_colour != Z_COLOUR_UNDEFINED)
            ||
            (default_background_colour != Z_COLOUR_UNDEFINED)
           )
        {
          pair_content(0, &foreground, &background);

          if (default_foreground_colour != Z_COLOUR_UNDEFINED)
            foreground = z_to_curses_colour(
                default_foreground_colour,
                NCURSESW_COLOUR_CLASS_FOREGROUND);

          if (default_background_colour != Z_COLOUR_UNDEFINED)
            background = z_to_curses_colour(
                default_background_colour,
                NCURSESW_COLOUR_CLASS_BACKGROUND);

          if (init_pair(1, foreground, background) == ERR)
            i18n_translate_and_exit(
                fizmo_ncursesw_module_name,
                i18n_ncursesw_FUNCTION_CALL_P0S_ABORTED_DUE_TO_ERROR,
                -0x2000,
                "init_pair");

          if (color_set(1, NULL) == ERR)
            i18n_translate_and_exit(
                fizmo_ncursesw_module_name,
                i18n_ncursesw_FUNCTION_CALL_P0S_ABORTED_DUE_TO_ERROR,
                -0x2052,
                "color_set");

          bkgdset(' ' | COLOR_PAIR(1));
          wbkgdset(infowin, ' ' | COLOR_PAIR(1));
        }
      }

      perform_init = false;
    }

    erase();

    attrset(A_BOLD);
    mvprintw(1, storywin_x + 7, "fizmo Z-Machine interpreter, Version %s\n",
        FIZMO_NCURSESW_VERSION);
    attrset(A_NORMAL);

    i = 0;
    while ( (i<storywin_height) && (i + scroll_index < story_list->nof_entries))
    {
      entry = story_list->entries[i + scroll_index];
      if (i + scroll_index == selected)
      {
        attrset(A_REVERSE);
        mvaddstr(storywin_y + i, storywin_x, "  ");
        printw("%3d  ", scroll_index + i + 1);
        len = (long)strlen(entry->title) > story_title_length
          ? story_title_length - 3 : story_title_length;
        addnstr(entry->title, len);
        if ((long)strlen(entry->title) > story_title_length)
          addstr("...");
        getyx(stdscr, y, x);
        j = storywin_x + storywin_width - x;
        while (j-- > 0)
          addstr(" ");
        attrset(A_NORMAL);
      }
      else
      {
        mvprintw(storywin_y + i, storywin_x + 2, "%3d  ", scroll_index + i + 1);
        len = (long)strlen(entry->title) > story_title_length
          ? story_title_length - 3 : story_title_length;
        addnstr(entry->title, len);
        if ((long)strlen(entry->title) > story_title_length)
          addstr("...");
      }

      i++;
    }

    werase(infowin);
    entry = story_list->entries[selected];

    wattrset(infowin, A_BOLD);
    wprintw(infowin, "%s\n", entry->title);
    wattrset(infowin, A_NORMAL);

    wprintw(infowin, "By: %s\n", entry->author);

    wprintw(infowin, "v%d / S:%s / R:%d / C:%d.\n", entry->z_code_version,
        entry->serial, entry->release_number, entry->checksum);

    if (infowin_topindex == 0)
      wprintw(infowin, "\n");
    else
    {
      waddstr(infowin, "[");
      ptr = infowin_back;
      while (ptr != NULL)
      {
        ptr = z_ucs_string_to_wchar_t(
            wchar_t_buf,
            ptr,
            NCURSESW_WCHAR_T_BUF_SIZE);

        if (waddwstr(infowin, wchar_t_buf) == ERR)
          i18n_translate_and_exit(
              fizmo_ncursesw_module_name,
              i18n_ncursesw_FUNCTION_CALL_P0S_ABORTED_DUE_TO_ERROR,
              -1,
              "waddwstr");
      }
      waddstr(infowin, "]\n");
    }

    infowin_lines_skipped = 0;
    infowin_skip_x = 0;
    infowin_full = false;
    //wordwrap_set_line_index(infowin_output_wordwrapper, 0);
    src = entry->description;
    do
    {
      src = utf8_string_to_zucs_string(
          z_ucs_t_buf, src, NCURSESW_Z_UCS_BUF_SIZE);
      wordwrap_wrap_z_ucs(infowin_output_wordwrapper, z_ucs_t_buf);
    }
    while (src != NULL);
    wordwrap_flush_output(infowin_output_wordwrapper);

    refresh();
    wrefresh(infowin);

    max_filedes_number_plus_1
      = (STDIN_FILENO < ncursesw_if_signalling_pipe[0]
          ? ncursesw_if_signalling_pipe[0]
          : STDIN_FILENO) + 1;

    FD_ZERO(&input_selectors);
    FD_SET(STDIN_FILENO, &input_selectors);
    FD_SET(ncursesw_if_signalling_pipe[0], &input_selectors);

    select_retval = select(
        max_filedes_number_plus_1,
        &input_selectors,
        NULL,
        NULL,
        NULL);

    if (select_retval > 0)
    {
      TRACE_LOG("select_retval > 0.\n");

      // something has changed in one of out input pipes.
      if (FD_ISSET(STDIN_FILENO, &input_selectors))
      {
        input = getch();

        if (input == '\n')
        {
          result = fizmo_strdup(story_list->entries[selected]->filename);
          input_done = true;
        }
        else if (input == KEY_UP)
        {
          if (selected > 0)
            selected--;
          if (selected < scroll_index)
            scroll_index--; 
          infowin_topindex = 0;
        }
        else if (input == KEY_DOWN)
        {
          if (selected  + 1 < story_list->nof_entries)
            selected++;
          if (selected == storywin_height + scroll_index)
            scroll_index++; 
          infowin_topindex = 0;
        }
        else if (input == 27)
        {
          input_done = true;
          result = NULL;
        }
        else if ( (input == ' ') && (infowin_full == true) )
        {
          infowin_topindex += (infowin_height - 5);
        }
        else if (input == 'b')
        {
          if ((infowin_topindex -= (infowin_height - 5)) < 0)
            infowin_topindex = 0;
        }
      }
      else
      {
        if (errno != 0)
        {
          if (errno == EINTR)
          { errno = 0; continue; }
          else
          {
            i18n_translate_and_exit(
                fizmo_ncursesw_module_name,
                i18n_ncursesw_ERROR_P0D_OCCURED_BEFORE_READ_P1S,
                -0x203c,
                errno,
                strerror(errno));
          }
        }

        bytes_read = 0;

        while (bytes_read != sizeof(int))
        {
          read_retval = read(
              ncursesw_if_signalling_pipe[0],
              &new_signal, 
              sizeof(int));

          if (read_retval == -1)
          {
            if (errno == EAGAIN)
            {
              errno = 0;
              continue;
            }
            else
            {
              i18n_translate_and_exit(
                  fizmo_ncursesw_module_name,
                  i18n_ncursesw_FUNCTION_CALL_P0S_RETURNED_ERROR_P1D_P2S,
                  -0x2041,
                  "read",
                  errno,
                  strerror(errno));
            }
          }
          else
          {
            bytes_read += read_retval;
          }
        }

        if (new_signal == SIGWINCH)
        {
          //exit(1);
          perform_init = true;
          endwin();
        }
      }
    }
  }

  free_z_story_list(story_list);
  delwin(infowin);
  erase();
  move(0,0);
  refresh();

  sigaction(SIGWINCH, NULL, NULL);

  endwin();
  wordwrap_destroy_wrapper(infowin_output_wordwrapper);

  return result;
}


/*
static int ncursesw_parse_config_parameter(char *key, char *value)
{
  //short color;
  //int int_value;

  if (strcasecmp(key, "bold-for-bright-foreground") == 0)
  {
    if ( (value != NULL) && (strcmp(value, "") != 0) )
      use_bold_for_bright_foreground = true;

    return 0;
  }
  else if (strcasecmp(key, "blink-for-bright-background") == 0)
  {
    if ( (value != NULL) && (strcmp(value, "") != 0) )
      use_blink_for_bright_background = true;

    return 0;
  }
#ifdef ENABLE_X11_IMAGES
  else if (strcasecmp(key, "enable-xterm-graphics") == 0)
  {
   if ( (value != NULL) && (strcmp(value, "") != 0) )
      enable_x11_graphics = true;

    return 0;
  }
#endif
  else if (strcasecmp(key, "dont-update-story-list") == 0)
  {
    if ( (value != NULL) && (strcmp(value, "") != 0) )
      dont_update_story_list_on_start = true;

    return 0;
  }
  else if (strcasecmp(key, "enable-xterm-title") == 0)
  {
    if ( (value != NULL) && (strcmp(value, "") != 0) )
      use_xterm_title = true;

    return 0;
  }

  return 1;
}
*/


void catch_signal(int sig_num)
{
  int bytes_written = 0;
  int ret_val;
  int ncursesw_if_write_buffer;

  // Note:
  // Look like TRACE_LOGs in this function may cause a deadlock in case
  // they're called while a fflush for the tracelog is already underway.

  ncursesw_if_write_buffer = sig_num;

  //TRACE_LOG("Caught signal %d.\n", sig_num);

  while ((size_t)bytes_written < sizeof(int))
  {
    ret_val = write(
        ncursesw_if_signalling_pipe[1],
        &ncursesw_if_write_buffer,
        sizeof(int));

    if (ret_val == -1)
      i18n_translate_and_exit(
          fizmo_ncursesw_module_name,
          i18n_ncursesw_FUNCTION_CALL_P0S_RETURNED_ERROR_P1D_P2S,
          -0x2023,
          "write",
          errno,
          strerror(errno));

    bytes_written += ret_val;
  }

  //TRACE_LOG("Catch finished.\n");
}


int main(int argc, char *argv[])
{
  int argi = 1;
  int story_filename_parameter_number = -1;
  int blorb_filename_parameter_number = -1;
  char *current_locale;
  char *current_locale_copy;
  char *index;
  char *story_file;
  int flags;
  char *value;
  //char *slash_index = NULL;
  char *fizmo_dir = NULL;
  //int len;
  int int_value;
  char *cwd = NULL;
  char *absdirname = NULL;
  size_t absdirname_len = 0;

#ifdef ENABLE_TRACING
  turn_on_trace();
#endif // ENABLE_TRACING

  current_locale = setlocale(LC_ALL, "");

  fizmo_register_screen_cell_interface(&ncursesw_interface);

#ifdef SOUND_INTERFACE_STRUCT_NAME
  //TRACE_LOG("sound: %p\n", &SOUND_INTERFACE_STRUCT_NAME);
  //TRACE_LOG("sound2: %p\n", &sound_interface_sdl);
  fizmo_register_sound_interface(&SOUND_INTERFACE_STRUCT_NAME);
  //TRACE_LOG("init_sound: %p\n", (&SOUND_INTERFACE_STRUCT_NAME)->init_sound);
  //(&sound_interface_sdl)->init_sound();
  //TRACE_LOG("close_sound: %p\n", SOUND_INTERFACE_STRUCT_NAME->close_sound);
#endif // SOUND_INTERFACE_STRUCT_NAME

  //TRACE_LOG("sound: %p\n", SOUND_INTERFACE_STRUCT_NAME);

  TRACE_LOG("current locale: \"%s\".\n", current_locale);

  // Parsing must occur after "fizmo_register_screen_cell_interface" so
  // that fizmo knows where to forward "parse_config_parameter" parameters
  // to.
  parse_fizmo_config_files();

  value = get_configuration_value("locale");

  if ( (current_locale != NULL) && (value == NULL) )
  {
    if ((current_locale_copy
          = (char*)malloc(strlen(current_locale) + 1)) == NULL)
    {
      //set_configuration_value("locale", DEFAULT_LOCALE, "fizmo");

      i18n_translate_and_exit(
          fizmo_ncursesw_module_name,
          i18n_ncursesw_FUNCTION_CALL_MALLOC_P0D_RETURNED_NULL_PROBABLY_OUT_OF_MEMORY,
          -0x010e,
          (long int)strlen(current_locale) + 1);
    }

    strcpy(current_locale_copy, current_locale);

    index = strchr(current_locale_copy, '.');
    if (index != NULL)
      *index = '\0';

    /*
    if (set_configuration_value("locale", current_locale_copy, "fizmo") == -1)
      set_configuration_value("locale", DEFAULT_LOCALE, "fizmo");
    else
      fizmo_locale = current_locale_copy;
      */
  }
  else
  {
    //set_configuration_value("locale", DEFAULT_LOCALE, "fizmo");
  }

  ncursesw_argc = argc;
  ncursesw_argv = argv;

  while (argi < argc)
  {
    if ((strcmp(argv[argi], "-p") == 0)
        || (strcmp(argv[argi], "--predictable") == 0))
    {
      set_configuration_value("random-mode", "predictable");
      argi += 1;
    }
    else if ((strcmp(argv[argi], "-fp") == 0)
        || (strcmp(argv[argi], "--force-predictable") == 0))
    {
      set_configuration_value("random-mode", "force-predictable");
      argi += 1;
    }
    else if ((strcmp(argv[argi], "-st") == 0)
        || (strcmp(argv[argi], "--start-transcript") == 0))
    {
      set_configuration_value("start-script-when-story-starts", "true");
      argi += 1;
    }
    else if ((strcmp(argv[argi], "-rc") == 0)
        || (strcmp(argv[argi], "--start-recording-commands") == 0))
    {
      set_configuration_value(
          "start-command-recording-when-story-starts", "true");
      argi += 1;
    }
    else if ((strcmp(argv[argi], "-if") == 0)
        || (strcmp(argv[argi], "--input-file") == 0))
    {
      set_configuration_value(
          "start-file-input-when-story-starts", "true");
      argi += 1;
    }
    else if ((strcmp(argv[argi], "-l") == 0)
        || (strcmp(argv[argi], "--set-locale") == 0))
    {
      if (++argi == argc)
      {
        return -1;
      }

      /*
      if (set_configuration_value("locale", argv[argi], "fizmo") == -1)
      {
        streams_latin1_output("\n");

        //set_configuration_value(
            //"locale", get_configuration_value("locale"), "ncursesw");

        i18n_translate(
            fizmo_ncursesw_module_name,
            i18n_ncursesw_INVALID_CONFIGURATION_VALUE_P0S_FOR_P1S,
            argv[argi],
            "locale");

        //set_configuration_value(
            //"locale", get_configuration_value("locale"), "fizmo");

        streams_latin1_output("\n");

        print_startup_syntax();
        exit(EXIT_FAILURE);
      }
      else
      {
        fizmo_locale = argv[argi];
        argi++;
      }
      */
    }
    else if (
        (
         (strcmp(argv[argi], "-b") == 0)
         ||
         (strcmp(argv[argi], "--background-color") == 0)
        )
        &&
        (argi+1 < argc)
        )
    {
      if (parse_config_parameter("background-color", argv[argi+1])
          != 0)
        exit(EXIT_FAILURE);

      argi += 2;
    }
    else if (
        (
         (strcmp(argv[argi], "-f") == 0)
         ||
         (strcmp(argv[argi], "--foreground-color") == 0)
        )
        &&
        (argi+1 < argc)
        )
    {
      if (parse_config_parameter("foreground-color", argv[argi+1])
          != 0)
        exit(EXIT_FAILURE);

      argi += 2;
    }
    else if (
        (strcmp(argv[argi], "-bf") == 0)
        ||
        (strcmp(argv[argi], "--bold-for-bright-foreground") == 0)
        )
    {
      use_bold_for_bright_foreground = true;
      argi ++;
    }
    else if (
        (strcmp(argv[argi], "-bb") == 0)
        ||
        (strcmp(argv[argi], "--blink-for-bright-background") == 0)
        )
    {
      use_blink_for_bright_background = true;
      argi ++;
    }
    else if (
        (strcmp(argv[argi], "-um") == 0)
        ||
        (strcmp(argv[argi], "--umem") == 0)
        )
    {
      set_configuration_value("force-quetzal-umem", "true");
      argi ++;
    }
#ifdef ENABLE_X11_IMAGES
    else if (
        (strcmp(argv[argi], "-x") == 0)
        ||
        (strcmp(argv[argi], "--enable-xterm-graphics") == 0)
        )
    {
      enable_x11_graphics = true;
      argi++;
    }
#endif
    else if (
        (strcmp(argv[argi], "-xt") == 0)
        ||
        (strcmp(argv[argi], "--enable-xterm-title") == 0)
        )
    {
      use_xterm_title = true;
      argi++;
    }
    else if (
        (strcmp(argv[argi], "-s8") == 0)
        ||
        (strcmp(argv[argi], "--force-8bit-sound") == 0))
    {
      set_configuration_value("force-8bit-sound", "true");
      argi += 1;
    }
    else if (
        (strcmp(argv[argi], "-ds") == 0)
        ||
        (strcmp(argv[argi], "--disable-sound") == 0))
    {
      set_configuration_value("disable-sound", "true");
      argi += 1;
    }
    else if (
        (strcmp(argv[argi], "-t") == 0)
        ||
        (strcmp(argv[argi], "--set-tandy-flag") == 0))
    {
      set_configuration_value("set-tandy-bit", "true");
      argi += 1;
    }
    else if (
        (strcmp(argv[argi], "-lm") == 0)
        ||
        (strcmp(argv[argi], "-rm") == 0)
        ||
        (strcmp(argv[argi], "--left-margin") == 0)
        ||
        (strcmp(argv[argi], "--right-margin") == 0)
        )
    {
      if (++argi == argc)
      {
        print_startup_syntax();
        exit(EXIT_FAILURE);
      }

      int_value = atoi(argv[argi]);

      if (
          ( (int_value == 0) && (strcmp(argv[argi], "0") != 0) )
          ||
          (int_value < 0)
         )
      {
        /*
        set_configuration_value(
            "locale", get_configuration_value("locale"), "ncursesw");
        */

        i18n_translate(
            fizmo_ncursesw_module_name,
            i18n_ncursesw_INVALID_CONFIGURATION_VALUE_P0S_FOR_P1S,
            argv[argi],
            argv[argi - 1]);

        /*
        set_configuration_value(
            "locale", get_configuration_value("locale"), "fizmo");
        */

        streams_latin1_output("\n");

        print_startup_syntax();
        exit(EXIT_FAILURE);
      }

      if (
          (strcmp(argv[argi - 1], "-lm") == 0)
          ||
          (strcmp(argv[argi - 1], "--left-margin") == 0)
         )
        set_custom_left_cell_margin(int_value);
      else
        set_custom_right_cell_margin(int_value);

      argi += 1;
    }
    else if (
        (strcmp(argv[argi], "-h") == 0)
        ||
        (strcmp(argv[argi], "--help") == 0)
        )
    {
      print_startup_syntax();
      exit(0);
    }
    else if (
        (strcmp(argv[argi], "-nu") == 0)
        ||
        (strcmp(argv[argi], "--dont-update-story-list") == 0))
    {
      dont_update_story_list_on_start = true;
      argi += 1;
    }
    else if (
        (strcmp(argv[argi], "-u") == 0)
        ||
        (strcmp(argv[argi], "--update-story-list") == 0))
    {
      update_fizmo_story_list(fizmo_dir);
      printf("\n");
      directory_was_searched = true;
      argi += 1;
    }
    else if (
        (strcmp(argv[argi], "-s") == 0)
        ||
        (strcmp(argv[argi], "--search") == 0))
    {
      if (++argi == argc)
      {
        print_startup_syntax();
        exit(EXIT_FAILURE);
      }

      if (strlen(argv[argi]) > 0)
      {
        if (argv[argi][0] == '/')
          search_directory(argv[argi], false);
        else
        {
          if (cwd == NULL)
            cwd = getcwd(NULL, 0);

          if (absdirname_len < strlen(cwd) + strlen(argv[argi]) + 2)
          {
            absdirname_len = strlen(cwd) + strlen(argv[argi]) + 2;
            absdirname = fizmo_realloc(absdirname, absdirname_len);
          }
          sprintf(absdirname ,"%s/%s", cwd, argv[argi]);
          search_directory(absdirname, false);
        }

        printf("\n");
        directory_was_searched = true;
      }

      argi += 1;
    }
    else if (
        (strcmp(argv[argi], "-rs") == 0)
        ||
        (strcmp(argv[argi], "--recursively-search") == 0))
    {
      if (++argi == argc)
      {
        print_startup_syntax();
        exit(EXIT_FAILURE);
      }

      if (strlen(argv[argi]) > 0)
      {
        if (argv[argi][0] == '/')
          search_directory(argv[argi], true);
        else
        {
          if (cwd == NULL)
            cwd = getcwd(NULL, 0);

          if (absdirname_len < strlen(cwd) + strlen(argv[argi]) + 2)
          {
            absdirname_len = strlen(cwd) + strlen(argv[argi]) + 2;
            absdirname = fizmo_realloc(absdirname, absdirname_len);
          }
          sprintf(absdirname ,"%s/%s", cwd, argv[argi]);
          search_directory(absdirname, true);
        }

        printf("\n");
        directory_was_searched = true;
      }

      argi += 1;
    }
    else if (
        (strcmp(argv[argi], "-sy") == 0)
        ||
        (strcmp(argv[argi], "--sync-transcript") == 0))
    {
      set_configuration_value("sync-transcript", "true");
      argi += 1;
    }
    else if (story_filename_parameter_number == -1)
    {
      story_filename_parameter_number = argi;
      argi++;
    }
    else if (blorb_filename_parameter_number == -1)
    {
      blorb_filename_parameter_number = argi;
      argi++;
    }
    else
    {
      // Unknown parameter:
      print_startup_syntax();
      exit(EXIT_FAILURE);
    }
  }

  if (cwd != NULL)
    free(cwd);

  if (absdirname != NULL)
    free(absdirname);

  if (directory_was_searched == true)
    exit(EXIT_SUCCESS);

  timerval.it_interval.tv_sec = 0;
  timerval.it_interval.tv_usec = 0;

  empty_timerval.it_interval.tv_sec = 0;
  empty_timerval.it_interval.tv_usec = 0;
  empty_timerval.it_value.tv_sec = 0;
  empty_timerval.it_value.tv_usec = 0;

  // Create a new signalling pipe. This pipe is used by a select call to
  // detect an incoming time-signal for the input routine.
  if (pipe(ncursesw_if_signalling_pipe) != 0)
    i18n_translate_and_exit(
        fizmo_ncursesw_module_name,
        i18n_ncursesw_FUNCTION_CALL_P0S_RETURNED_ERROR_P1D_P2S,
        -0x2016,
        "pipe",
        errno,
        strerror(errno));

  // Get the current flags for the read-end of the pipe.
  if ((flags = fcntl(ncursesw_if_signalling_pipe[0], F_GETFL, 0)) == -1)
    i18n_translate_and_exit(
        fizmo_ncursesw_module_name,
        i18n_ncursesw_FUNCTION_CALL_P0S_RETURNED_ERROR_P1D_P2S,
        -0x2017,
        "fcntl / F_GETFL",
        errno,
        strerror(errno));

  // Add the nonblocking flag the read-end of the pipe, thus making incoming
  // input "visible" at once without having to wait for a newline.
  if ((fcntl(ncursesw_if_signalling_pipe[0], F_SETFL, flags|O_NONBLOCK)) == -1)
    i18n_translate_and_exit(
        fizmo_ncursesw_module_name,
        i18n_ncursesw_FUNCTION_CALL_P0S_RETURNED_ERROR_P1D_P2S,
        -0x2018,
        "fcntl / F_SETFL",
        errno,
        strerror(errno));

  //signal(SIGALRM, catch_signal);
  //signal(SIGWINCH, ncursesw_if_catch_signal);

  /*
  timerval.it_interval.tv_sec = 0;
  // The timer must not be automatically restarted: In case verfication
  // routines take too long (or in case read is called recursively) the
  // verification routine might be called again while it has not finished.
  //timerval.it_interval.tv_usec = 100000;
  timerval.it_interval.tv_usec = 0;
  timerval.it_value.tv_sec = 0;
  timerval.it_value.tv_usec = 100000;

  empty_timerval.it_interval.tv_sec = 0;
  empty_timerval.it_interval.tv_usec = 0;
  empty_timerval.it_value.tv_sec = 0;
  empty_timerval.it_value.tv_usec = 0;
  */

  sigemptyset(&default_sigaction.sa_mask);
  default_sigaction.sa_flags = 0;
  default_sigaction.sa_handler = &catch_signal;
  sigaction(SIGALRM, &default_sigaction, NULL);
  //sigaction(SIGWINCH, &default_sigaction, NULL);

  if (story_filename_parameter_number == -1)
  {
    if ((story_file = select_story_from_menu(fizmo_dir)) == NULL)
      return 0;
  }
  else
    story_file = argv[story_filename_parameter_number];

  sigemptyset(&default_sigaction.sa_mask);
  default_sigaction.sa_flags = 0;
  default_sigaction.sa_handler = &ncursesw_if_catch_signal;
  sigaction(SIGWINCH, &default_sigaction, NULL);

  fizmo_start(
      story_file,
      (blorb_filename_parameter_number != -1
       ? argv[blorb_filename_parameter_number]
       : NULL),
      NULL);

  sigaction(SIGWINCH, NULL, NULL);

  if (story_filename_parameter_number == -1)
    free(story_file);

  TRACE_LOG("Closing signalling pipes.\n");

  close(ncursesw_if_signalling_pipe[1]);
  close(ncursesw_if_signalling_pipe[0]);

  return 0;
}


