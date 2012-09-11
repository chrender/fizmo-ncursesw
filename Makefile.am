
# AUTOMAKE_OPTIONS = -Wno-override

localedir = $(datarootdir)/fizmo/locales

SUBDIRS = src/fizmo-ncursesw

install-locales::
	mkdir -p $(DESTDIR)$(localedir)
	for l in `cd src/locales ; ls -d ??_??`; \
	  do \
	  mkdir -p $(DESTDIR)$(localedir)/$$l; \
	  cp src/locales/$$l/*.txt $(DESTDIR)$(localedir)/$$l; \
	done

