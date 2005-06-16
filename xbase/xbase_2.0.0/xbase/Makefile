#
#   Zd540 Makefile
#   Modified to have everything alphabetic and to have all cpp's 
#     dependent on all h's compiles very quickly anyway
#

#COMPILE = ${CC} -c 
CC = gcc
CXX=g++
DEPEND = ${CC} -m32 -Wno-deprecated -MM -E -I../ -I./
#LFLAGS = -lstdc++ -Lzlib -lz
LFLAGS = -m32 -lstdc++


SOURCEFILES = \
   xdate.o \
   exp.o \
   expfunc.o \
   expproc.o \
   fields.o \
   html.o \
   index.o \
   lock.o \
   memo.o \
   ndx.o \
   ntx.o \
   stack.o \
   xbase.o \
   xbexcept.o \
   xbfilter.o \
   xbstring.o \
   dbf.o


.SUFFIXES : .cpp .d .h ${SUFFIXES}

# This makes sure that the .cpp date is later than the .h date. Used for
# dependency generation.
.h.cpp:
	chmod 644 $@
	touch $@

# This makes sure that the .cpp date is later than the .cc date. Used for
# dependency generation.
.cc.cpp:
	chmod 644 $@
	touch $@

.cpp.d :
	${DEPEND} $< > $@
	
#--- Ordinary rules
default : libxbase.a	

libxbase.a   : Makedepend
#	${CC} -DPREPROCESS_ONLY -D__cplusplus -P -E zd.h > zd.i
#	cat zdheaderbegin.h zdstd.h zdvermacs.h zdmacros.h zd.i zdheaderend.h > zd540.h
#	rm -f zd.i
	${MAKE} -r -f Makedepend
	ar r $@ ${SOURCEFILES}
#	cp libzd540.a ${ZDDIR}/libzd540g.a
	cp libxbase.a libxbaseg.a
	strip -x libxbase.a
	ar -ts libxbase.a
#	cp libzd540.a ${ZDDIR}/libzd540.a
#	chgrp zddev ${ZDDIR}/libzd540.a
#	chgrp zddev ${ZDDIR}/zd540.h
#	chmod 664 ${ZDDIR}/libzd540.a
#	chmod 664 ${ZDDIR}/zd540.h
#	chgrp zddev ./libzd540.a
#	chgrp zddev ./zd540.h
#	chmod 664 ./libzd540.a
#	chmod 664 ./zd540.h
	rm Makedepend
	
clean :
	rm *.d
	rm *.o
	
Makedepend : Makedepend.stub ${SOURCEFILES:o=d}
	echo "default : ${SOURCEFILES}" > Makedepend.1
	cat Makedepend.stub Makedepend.1 ${SOURCEFILES:o=d} > Makedepend
	rm Makedepend.1
	

