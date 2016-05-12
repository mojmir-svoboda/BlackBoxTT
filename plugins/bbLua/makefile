# 
#  GCC MakeFile For bbLua
#  Would be called makefile-gcc if I wasn't so sodding lazy
#

NAME = bbLua
CC = g++
CFLAGS = -Os -Wall -s
CPPFLAGS = -I../../blackbox -I. -I./include -mwindows -mno-cygwin 
LDFLAGS = $(CFLAGS)
TARGETS = bbLua.exe bbLua.dll
LIBSEXE = lib/liblua.a lib/liblualib.a ../../blackbox/libBlackbox.a
OBJSEXE = functions.o main.o bblua.o
LIBSDLL = lib/liblua.a lib/liblualib.a ../../blackbox/libBlackbox.a
OBJSDLL = functions.o bblua.o bbplugin.o

LUALIBS = winshell

all: build
	@for i in $(LUALIBS); do \
      (cd lualibs/$$i; make all); done

build: $(TARGETS) 

bbLua.dll: $(OBJSDLL)
	$(CC) $(CFLAGS) -o $@ -shared $^ $(LIBSDLL)
	ls -al $@
	
bbLua.exe: $(OBJSEXE)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBSEXE)
	ls -al $@

%.o: %.cpp
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $^

clean:
	rm -f $(TARGETS) `find -name '*.o' -or -name '*~'`