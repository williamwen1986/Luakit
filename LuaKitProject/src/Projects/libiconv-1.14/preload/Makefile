# Makefile for libiconv/preload

#### Start of system configuration section. ####

# Directories used by "make":
srcdir = .

# Directories used by "make install":
prefix = /usr/local
local_prefix = /usr/local
exec_prefix = ${prefix}
libdir = ${exec_prefix}/lib

# Programs used by "make":
CC = gcc
CFLAGS = -g -O2 -fvisibility=hidden
CPPFLAGS = 
LDFLAGS = 
INCLUDES = -I../lib -I$(srcdir)/../lib -I../include -I$(srcdir)/../include -I.. -I$(srcdir)/..
# -DBUILDING_LIBICONV: Change expansion of LIBICONV_DLL_EXPORTED macro.
# -DBUILDING_DLL: Change expansion of RELOCATABLE_DLL_EXPORTED macro.
DEFS = -DLIBDIR=\"$(libdir)\" -DBUILDING_LIBICONV -DBUILDING_DLL \
-DENABLE_RELOCATABLE=1 -DIN_LIBRARY -DINSTALLDIR=\"$(libdir)\" -DNO_XMALLOC \
-Dset_relocation_prefix=libiconv_set_relocation_prefix \
-Drelocate=libiconv_relocate -DPACKAGE_NAME=\"\" -DPACKAGE_TARNAME=\"\" -DPACKAGE_VERSION=\"\" -DPACKAGE_STRING=\"\" -DPACKAGE_BUGREPORT=\"\" -DPACKAGE_URL=\"\" -DINSTALLPREFIX=\"/usr/local\" -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -D__EXTENSIONS__=1 -D_ALL_SOURCE=1 -D_GNU_SOURCE=1 -D_POSIX_PTHREAD_SEMANTICS=1 -D_TANDEM_SOURCE=1 -DHAVE_DLFCN_H=1 -DLT_OBJDIR=\".libs/\" -DHAVE_VISIBILITY=1
LIBTOOL = /bin/sh $(top_builddir)/libtool
LIBTOOL_COMPILE = $(LIBTOOL) --mode=compile
LIBTOOL_LINK = $(LIBTOOL) --mode=link
LN_S = ln -s
RM = rm -f

# Programs used by "make install":
INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644
mkinstalldirs = $(SHELL) ./build-aux/mkinstalldirs

#### End of system configuration section. ####

SHELL = /bin/sh

# Needed by $(LIBTOOL).
top_builddir = .

SOURCES = $(srcdir)/../lib/iconv.c $(srcdir)/../libcharset/lib/localcharset.c $(srcdir)/../lib/relocatable.c

all : 

preloadable_libiconv.so : $(SOURCES)
	$(RM) -r objects
	mkdir objects && \
	for f in $(SOURCES); do \
	  $(LIBTOOL_COMPILE) $(CC) $(INCLUDES) $(CPPFLAGS) $(CFLAGS) $(DEFS) -DLIBICONV_PLUG -c $$f -o objects/`basename $$f | sed -e 's,\.c$$,.o,'` || exit 1; \
	done && \
	$(LIBTOOL_LINK) $(CC) $(LDFLAGS) $(CFLAGS) -o libiconv.la -rpath $(libdir) -no-undefined objects/*.lo && \
	$(RM) -r objects
	cp .libs/libiconv.so preloadable_libiconv.so

install : all force
	if [ ! -d $(DESTDIR)$(libdir) ] ; then $(mkinstalldirs) $(DESTDIR)$(libdir) ; fi
	if test -n ""; then $(INSTALL_DATA)  $(DESTDIR)$(libdir)/.new && mv $(DESTDIR)$(libdir)/.new $(DESTDIR)$(libdir)/ ; fi

install-strip : install

installdirs : force
	if [ ! -d $(DESTDIR)$(libdir) ] ; then $(mkinstalldirs) $(DESTDIR)$(libdir) ; fi

uninstall : force
	if test -n ""; then $(RM) $(DESTDIR)$(libdir)/ ; fi

check : all

mostlyclean : clean

clean : force
	$(RM) *.o *.lo *.a *.la preloadable_libiconv* core *.stackdump so_locations
	$(RM) -r .libs _libs objects

distclean : clean
	$(RM) config.status config.log config.cache Makefile libtool

maintainer-clean : distclean

force :

