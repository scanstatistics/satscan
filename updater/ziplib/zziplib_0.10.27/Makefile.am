AUTOMAKE_OPTIONS = 1.4 foreign
AUTOTOOL_VERSION=autoconf-2.52 automake-1.4f libtool-1.4

bin_PROGRAMS = zzcat zzdir
noinst_PROGRAMS = zziptest
bin_SCRIPTS = zzip-config
lib_LTLIBRARIES = libzzip.la
include_HEADERS = $(libzzip_la_headers)
doc_FILES = README TODO COPYING.LIB COPYING.ZLIB \
	zziplib.html zzip-zip.html zzip-file.html zzip-index.html \
	zzip-sdl-rwops.html
#
VERSIONINFO=@VERSIONINFO@
RELEASEINFO=@RELEASEINFO@
THREAD_SAFE=@THREAD_SAFE@
#

libzzip_la_SOURCES = \
	zzip-zip.c \
	zzip-file.c \
	zzip-dir.c \
	zzip-stat.c \
	zzip-info.c \
	zzip-err.c 
libzzip_la_headers = \
	zzip-stdint.h \
	zzip-file.h \
	zzip.h \
	zziplib.h \
	zzipformat.h \
	zzip-conf.h \
	zzip-config.h \
	zzip-msvc.h
libzzip_la_LDFLAGS= @ZZIPLIB_LDFLAGS@ $(RELEASEINFO) $(VERSIONINFO) \
                    $(THREAD_SAFE)
libzzip_la_LIBADD= -lz @RESOLVES@

SDL_rwops_FILES = SDL_rwops_zzip.h SDL_rwops_zzip.c SDL_rwops_zzcat.c
EXTRA_DIST = test.zip test2.zip zzip-doc.pl $(doc_FILES) $(PACKAGE).spec \
	$(SDL_rwops_FILES)
CONFIG_CLEAN_FILES = zzip-config.h zzip-INTRO.html zzip-SFNET.html

zziptest_LDFLAGS = @ZZIPLIB_LDFLAGS@
zziptest_LDADD = libzzip.la @RESOLVES@ -lz
zzcat_LDFLAGS =  @ZZIPLIB_LDFLAGS@
zzcat_LDADD = libzzip.la @RESOLVES@ -lz
zzdir_LDFLAGS =  @ZZIPLIB_LDFLAGS@
zzdir_LDADD = libzzip.la @RESOLVES@ -lz

test1: zziptest$(EXEEXT)
	./zziptest$(EXEEXT) test.zip
test2: zziptest$(EXEEXT)
	./zziptest$(EXEEXT) test2.zip
test3: zzcat$(EXEEXT)
	./zzcat$(EXEEXT) test/README
test4: zzdir$(EXTEXT)
	./zzdir test2.zip
test: test1 test2$(EXEEXT) test3 test4 

SDL_rwops_zzip.$(OBJEXT) : SDL_rwops_zzip.c
	$(COMPILE) $< -c -o $@ `sdl-config --cflags` `zzip-config --cflags`  
SDL_rwops_zzcat.$(OBJEXT) : SDL_rwops_zzcat.c
	$(COMPILE) $< -c -o $@ `sdl-config --cflags` `zzip-config --cflags`  
testsdl: SDL_rwops_zzip.$(OBJEXT) SDL_rwops_zzcat.$(OBJEXT) 
	$(LINK) SDL_rwops_zzip.$(OBJEXT) SDL_rwops_zzcat.$(OBJEXT) \
		`sdl-config --libs` `zzip-config --libs` -o zzcatsdl$(EXEEXT)
	zzcatsdl$(EXEEXT) test/README

# -------------------------------------------------------------------
@MAINTAINER_MODE_TRUE@test.zip : README
@MAINTAINER_MODE_TRUE@	zip test.zip README

check: zzcat$(EXEEXT) test.zip
	@ echo checking ./zzcat$(EXEXT) test/README 
	@ ./zzcat$(EXEXT) test/README >test.out
	@ if diff test.out README >/dev/null \
	; then rm test.out ; echo check OK ; true \
	; else rm test.out ; echo check FAIL ; false ; fi

# -------------------------------------------------------------------
auto:
	aclocal && autoconf && autoheader && automake

boottrap:
	rm -rf .deps .libs
	rm -f config.guess config.sub stamp-h.in
	rm -f install-sh ltconfig ltmain.sh depcomp mkinstalldirs
	rm -f config.h config.h.in config.log config.cache configure
	rm -f aclocal.m4 Makefile Makefile.in
	aclocal 
	autoconf 
	autoheader 
	automake -a -c 

rpm: dist $(PACKAGE).spec
	rpm -ta $(PACKAGE)-$(VERSION).tar.gz

# -------------------------------------------------------------------
doc: $(doc_FILES) zzip-INTRO.html zzip-SFNET.html
zziplib.html : ${libzzip_la_SOURCES} $(PACKAGE).spec
	@PERL@ zzip-doc.pl ${libzzip_la_SOURCES}
zzip-INTRO.html : zzip-index.html
	@ echo $@ '<<' zzip-index.html \
	; D=`date +%Y-%m-%d` \
	; cat zzip-index.html \
	| sed -e "s/<!--VERSION-->/$(VERSION)/" \
	| sed -e "s/<!--DATE-->/$$D/" \
	> $@
zzip-SFNET.html : zzip-index.html
	@ echo $@ '<<' zzip-index.html \
	; D=`date +%Y-%m-%d` \
	; cat zzip-index.html \
	| sed -e "s/<!--VERSION-->/$(VERSION)/" \
	| sed -e "s/<!--DATE-->/$$D/" \
	| sed -e "/!SFNET!/d" \
	> $@

docdir=@docdir@
pkgdocdir=${docdir}/${PACKAGE}
DOCEXAMPLES = zzdir.c zzcat.c zziptest.c $(SDL_rwops_FILES)
install-doc: $(doc_FILES) zzip-INTRO.html
	$(mkinstalldirs) $(DESTDIR)$(pkgdocdir)
	$(INSTALL_DATA) $(doc_FILES) $(DESTDIR)$(pkgdocdir)
	$(INSTALL_DATA) $(DOCEXAMPLES) $(DESTDIR)$(pkgdocdir)
	$(INSTALL_DATA) zzip-INTRO.html $(DESTDIR)$(pkgdocdir)/index.html

SFNETDIR=/groups/z/zz/zziplib/htdocs
SFNETDOC=${datadir}${SFNETDIR}
install-sfnet: $(doc_FILES) zzip-SFNET.html
	$(mkinstalldirs) $(DESTDIR)$(SFNETDOC)
	$(INSTALL_DATA) $(doc_FILES) $(DESTDIR)$(SFNETDOC)
	$(INSTALL_DATA) $(DOCEXAMPLES) $(DESTDIR)$(SFNETDOC)
	$(INSTALL_DATA) Makefile.am ChangeLog $(DESTDIR)$(SFNETDOC)
	$(INSTALL_DATA) zzip-SFNET.html $(DESTDIR)$(SFNETDOC)/index.html

# --------------------------------------------------------------------

install-zip installzip : all-am
	mkdir /tmp/$(PACKAGE)-$(VERSION)-`date +%Y-%m-%d-%H%M`
	@ DST=/tmp/$(PACKAGE)-$(VERSION)-`date +%Y-%m-%d-%H%M` \
	; echo .. install DESTDIR=$$DST \
	; $(MAKE) install DESTDIR=$$DST \
	; ZIP=`pwd`/$(PACKAGE)-$(VERSION).zip \
	; (cd $$DST && zip -9r $$ZIP .) \
	; echo rm -rf $$DST ; rm -rf $$DST
