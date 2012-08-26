
# AUTOMAKE_OPTIONS = -Wno-override

localedir = $(datarootdir)/fizmo/locales

# if FIZMO_BUILD_PREFIX_EMPTY
#   fizmo_build_prefix=$(DESTDIR)$(prefix)
# endif

# PKG_DIR = $(fizmo_build_prefix)/lib/pkgconfig
# PKGFILE = $(PKG_DIR)/libfizmo.pc

SUBDIRS = src/fizmo-ncursesw

install-locales::
	mkdir -p $(DESTDIR)$(localedir)
	for l in `cd src/locales ; ls -d ??_??`; \
	  do \
	  mkdir -p $(DESTDIR)$(localedir)/$$l; \
	  cp src/locales/$$l/*.txt $(DESTDIR)$(localedir)/$$l; \
	done

