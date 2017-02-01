#DEBUG=1

ifdef DEBUG
CFLAGS = -Wall -g -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -Wno-write-strings -Wno-format-security -U_FORTIFY_SOURCE
LFLAGS = -g
else
CFLAGS = -Wall -O3 -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -Wno-write-strings -Wno-format-security -U_FORTIFY_SOURCE
LFLAGS = -O3
endif

#G_INCL_PATHS=-I../include
MDEP_VARS=-- $(CFLAGS) 

OBJS=ndata.o Ndir32.o cmd_line.o Filelist.o dirtree.o diskparm.o \
	Fileread.o win2unix.o i64tostr.o Ndisplay.o nsort.o treelist.o 

SRCS=ndata.cpp Ndir32.cpp cmd_line.cpp Filelist.cpp dirtree.cpp diskparm.cpp \
	Fileread.cpp win2unix.cpp i64tostr.cpp Ndisplay.cpp nsort.cpp treelist.cpp 

#*************************************************************************
%.o: %.cpp
	g++ $(CFLAGS) -c $<

all: undir

clean:
	-rm -f *.o undir *~

lint:
	cmd /C "c:\lint9\lint-nt +v -width(160,4) $(LiFLAGS) -ic:\lint9 co-gcc.lnt -os(_lint.tmp) lintdefs.cpp $(SRCS)"

lint8:
	cmd /C "c:\lint8\lint-nt +v -width(160,4) $(LiFLAGS) -ic:\lint8 co-gcc.lnt -os(_lint.tmp) lintdefs.cpp $(SRCS)"

dist:
	rm -f *.zip
	zip undir.zip undir ndir.ini
	zip undirsrc.zip *.cpp *.hpp makefile *.ini *.txt

remod:
	chmod 644 *.cpp *.hpp makefile *.ini *.txt

undir: $(OBJS)
	g++ $(OBJS) $(LFLAGS) -s $(LIBS) -o $@

depend:
	makedepend $(MDEP_VARS) $(SRCS)

# DO NOT DELETE THIS LINE -- make depend depends on it

ndata.o: ndata.hpp win2unix.hpp
Ndir32.o: ndata.hpp win2unix.hpp dirtree.hpp
cmd_line.o: ndata.hpp win2unix.hpp dirtree.hpp
Filelist.o: win2unix.hpp i64tostr.hpp ndata.hpp
dirtree.o: dirtree.hpp
diskparm.o: ndata.hpp win2unix.hpp i64tostr.hpp
Fileread.o: ndata.hpp win2unix.hpp dirtree.hpp
i64tostr.o: i64tostr.hpp
Ndisplay.o: ndata.hpp win2unix.hpp i64tostr.hpp
nsort.o: ndata.hpp win2unix.hpp
treelist.o: ndata.hpp win2unix.hpp i64tostr.hpp dirtree.hpp
