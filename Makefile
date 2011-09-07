# $Id: Makefile 8169 2011-01-07 17:32:36Z luigi $

CC=gcc
STRIP=strip
CFLAGS = -O3 -march=armv6j -fomit-frame-pointer -mapcs-frame -Wall -I ./include/ -I ./include/freetype2/  
# files to publish
HEADERS = im.h pixop.h screen.h
HEADERS += linux/
ALLSRCS = im.cc screen.c dbus.c ttf.cc

OBJS = im.o screen.o dbus.o ttf.o

scpim: im
	strip im
	scp im root@kindle:~
im: $(OBJS)
	g++ $(CFLAGS) -o im $(OBJS) -lchewing -ldbus-1 -L./lib-bin -lutil lib-bin/libfreetype.so.6 
 
im.o: im.cc im.h

ttf.o: ttf.cc im.h
	g++ $(CFLAGS) -c ttf.cc
clean:
	rm -rf im *.o *.core

# conversion
# hexdump -e '"\n\t" 8/1 "%3d, "'
# DO NOT DELETE
