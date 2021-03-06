


   **Version 0.7.15 — Febuary 19, 2019**

 - Fixed potential compile errors due to relative paths in autoconf parameters.
 - Replaced obsolete AC\_TRY\_LINK with AC\_LINK\_IFELSE.
 - Updated tiny-xml-doc-tools.
 - Fixed underscores in markdown files.

---


   **Version 0.7.14 — September 3, 2017**

 - Renamed copyright files to “license” for github license detection compatibility, see [Github's “Licensing a repository”](https://help.github.com/articles/licensing-a-repository/) for further reference.
 - Added missing contributor phrasing to BSD-3 clause. The resulting license now exactly matches the wording used on Github and so also makes the license detection work.
 - Updated man page.

---


   **Version 0.7.13 — April 9, 2017**

 - Show error message in case no installed locales can be found.
 - Adapted to replacement of en\_US locale with en\_GB from libfizmo.

---


   **Version 0.7.12 — October 9, 2016**

 - Adapted to new split-library bulids.
 - Updated man page.

---


   **Version 0.7.11 — August 31, 2016**

 - Fixed typo in manpage.
 - Used tiny-xml-doc-tools for documentation.
 - Adapted to renaming “libcellif” to “libmonospaceif”.

---


   **Version 0.7.10 — July 28, 2016**

 - Updated manpage and coyright data for version 0.8.0 release.

---


   **Version 0.7.9 — October 9, 2015**

 - No more strict dependency on libncursesw since libncurses works equally well.
 - Added forgotten configuration options “with-ncurses-includedir”, “with-ncursesw-libdir” and “with-ncurses-libdir”.
 - Minor autoconf-related changes for fizmo-dist-0.7.10-compatibility.

---


   **Version 0.7.8 — March 19, 2014**

 - Adapted to automake v1.14 “subdir-objects” option.
 - Added missing license/copyright information.

---


   **Version 0.7.7 — June 5, 2013**

 - Adapted to new “fizmo\_register\_screen\_interface” and “fizmo\_start” functions.
 - Fix build error for some systems due to missing wchar.h include – thanks to Nikos Chantziaras.
 - Added default terminal color evluation.
 - Adapted to new “install-data-local” build target.

---


   **Version 0.7.6 — December 27, 2012**

 - Improved configure-detection for libncursesw for systems without pkg-config's ncursesw.pc.
 - Adapted to new “disable-x11” and “disable-sound” configuration.

---


   **Version 0.7.5 — November 30, 2012**

 - Fixed missing delete input.
 - Fixed manpage typos, thanks to Johan Ljunglid.
 - Implemented redraw via CTRL-R.

---


   **Version 0.7.4 — September 17, 2012**

 - Adapted to new autoconf/automake build system.

---


   **Version 0.7.3 — August 1, 2012**

 - Merged Andrew Plotkin's iOS-fizmo changes: Adapted to “prompt\_for\_filename” function in screen-cell-interface.
 - Minor memory leak cleanups.

---


   **Version 0.7.2 — March 9, 2012**

 - Fixed missing $(DESTDIR) variable – should already have been present since version 0.7.1.

---


   **Version 0.7.1 — November 6, 2011**

 - Fixed missing “override” statments for CFLAGS in Makefiles.
 - Fixed $(DESTDIR) evaluation in config.[default|macports].mk.
 - Changed manpage install dir from “man” to “share/man”.
 - Adapted Makefiles and configuration to use standard GNU Makefile variables instead of INSTALL\_PATH and FIZMO\_BIN\_DIR.
 - Respect LD\_FLAGS for linking.

---


   **Version 0.7.0 — September 18, 2011**

 - The “fizmo-ncursesw” interface has been re-implemented for version 0.7. It now makes use of libcellif for output. It uses libfizmo's new word hyphenation and implements better X11-output through libdrilbo.
 - X11 frontispiece display is now by default in a separate window, which, contrary to the older implementation that simply invoked a XCopyArea directly into the terminal's X window, should work reliably. For terminals which by chance support it, the old behaviour is still available by using the “display-x11-inline-image” config option. The inline display has been changed to no longer write directly into the terminal window, but instead to use a subwindow instead.
 - Adapted to version 0.7 locale system and the new build process.

---


   **Version 0.6.9 — April 24, 2011**

 - Added NCURSESW\_INCLUDE\_FROM\_W\_DIRNAME to configs to allow usage of Apple's ncurses implementation, added GDKPIXBUF\_INC\_DIR configuration variable.

---


   **Version 0.6.4 — May 24, 2009**

 - Bugfix release: Definitely recommend for read\_char fix.
 - Fixed interface to supply default colors in case ncurses' “pair\_content” returned invalid colors (thanks to Samuel Verschelde).
 - Input from read\_char is no longer re-echoed to the screen (thanks to Samnuel Verschelde).

---


   **Version 0.6.0 — March 25, 2009**

 - This marks the point for the first public beta release.
 - Implemented conversion of font 3 (charachter font) to unicode. This appears to make Beyond Zork's map appear at least readable.
 - Implemented recording and replaying of timed input.
 - The filename input for save and restore may now be cancelled using the escape key.
 - Pressing arrow up/down during filename input no longer displays the command history.
 - Implemented wordwrapper / ncursesw margins.
 - Implemented “no-update” command line flag and config option to avoid long startup scan times (for example on slow notebook drivers with many story files).
 - Implemented “--search” and “--recursively-search” command line invocations.
 - Fixed bug in scrollback parapgraph position cache.
 - Fixed scroll-down display for scrolling to bottom.
 - Fixed crash that occured when a read-instruction was recursively called from a timed input verification-routine.
 - Fixed display error on refresh of preloaded input from history.
 - Implemented forced predictable mode (ignores seeding random generator).
 - Many, many other minor fixes and extensions.

---


   **Version 0.5.3 — March 11, 2009**

 - Re-wrote Makefile system. Releveant module defaults are now kept in separate .mk-files.
 - Added support for extra-blorb files (these are specified on the command line directly after the story file name).
 - Added “-Wextra” flag to CFLAGS and cleaned up new warnings caused by the new flag.
 - Many minor fixes (option system, running X11-enabled fizmo in text-only console, etc).

---


   **Version 0.5.2 — January 17, 2009**

 - Implemented story selection menu in ncursesw interface.
 - Implemented parsing of rc file.
 - Added bold-for-bright-foreground and blink-for-bright-background terminal options.
 - Implemented dont-use-color and force-color option.
 - Fixed interface for > 121 colors.
 - Added “help” command.
 - Implemented text paragraph position cache to speed up scrollback for large amounts of text (not elegant, but works).
 - Implemened “force-quetzal-umem” config option from the command line.
 - Added manpage.
 - Implemented basic blorb support (meaning that blorb files are detected, information about pictures and sound is parsed and the first found “ZCOD” chunk is executed).
 - Stories may now be started from the command line using their “real name” in case they are stored in the story-list. Example: “fizmo sorcerer”.
 - Many, many small fixes (scrollback, winch-redisplay, negative score, undo, crash on large scrollbacks, etc).

---


   **Version 0.5.1 — November 4, 2008**

 - Fixed scrollback and i18n-exit-translation.
 - Fixed libfizmo to also use setitimer/sigaction (makes fizmo work better on linux).
 - Implemented foreground/background color command line parameters for ncursesw interface.
 - Fixed get-cursor-[column|row] for ncursesw (makes PRINT\_TABLE and Sherlock work).
 - Implemented restore from the command line: In case a quetzal-savegame containing the non-standard-chunk “FILE” is given on the command line, fizmo will try to restore the savegame using the story filename stored in this chunk.

---


   **Version 0.5.0 — October 30, 2008**

 - This version is now definitely usable to play all non-v6 games. Did extensive testing using Borderzone, “LostPig.z8”, “Zokoban.z5”, “crashme.z5”, “etude.z5”, “paint.z5”, “random.z5”, “reverzi.z5” and “unicode.z5”. Only two minor known bugs remain: Scrollback sometimes miscalculates the current row after a lot of scrolling back and forth (which is always “fixable” to pressing any-key which correctly rebuilds the current output page, and a display anomaly on the frontpage of “vampire.z8” which I intended to keep after a lot of code-inspection (since fizmo appears to be implementing the screen modell correctly and fixing this display problem breaks a lot of other games). This version has been tested on Linux, Darwin (Mac Os X) and a little bit on XP/Cygwin (using a self-built ncursesw).
 - Fixed timed-input detection for “read” opcode. This fixes the crash in ZTUU when the lantern goes out.
 - Ran fizmo through (sp)lint, fixed memory leaks, minor bugs, many typecasts, some double and inconsistent definitions, cleanup up code.
 - Fixed Makefile dependencies.
 - Implemened scrollback for ncursesw interface, added support method for scrollback in “history.c”.
 - Added configuration system.
 - Some split-window / set\_window / set\_cursor fixed for ncursesw-interface.
 - Substitued ualarm/signal with setitimer/sigaction since the first one won't catch SIGALRM on linux and manpage says that ualarm is obsolete.
 - Fixed color management so that ncurses color pair is avaiable for reading.
 - Many, many minor bugfixes.

---


   **Version 0.4.1 — November 14, 2007**

 - Separated code in core, interface, c and cpp interfaces.
 - Improved upper window handling (trinity sundial) and blockbuffer (zork1.z5).


