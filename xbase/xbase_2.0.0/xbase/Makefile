
COMPILATION = -m32
CC = g++
DEBUG = -gstabs+
DEPEND = ${CC} ${COMPILATION} -Wno-deprecated -MM -E -I../ -I./

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
.h.cpp:	chmod 644 $@
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
	${MAKE} -r -f Makedepend COMPILATION=${COMPILATION} DEBUG=${DEBUG}
	ar r $@ ${SOURCEFILES}
	cp libxbase.a libxbaseg.a
	strip -x libxbase.a
	ar -ts libxbase.a
	rm Makedepend
	
clean :
	rm -f *.d
	rm -f *.o
	
Makedepend : Makedepend.stub ${SOURCEFILES:o=d}
	echo "default : ${SOURCEFILES}" > Makedepend.1
	cat Makedepend.stub Makedepend.1 ${SOURCEFILES:o=d} > Makedepend
	rm Makedepend.1
	

 