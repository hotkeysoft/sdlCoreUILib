.DEFAULT_GOAL := digilib
CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=-O2 -std=c++11 -I. -I/usr/include/SDL2 -Wall -fPIC -Wno-unknown-pragmas -Wno-reorder
LDFLAGS=-shared -Wl,-soname,libcoreui.so.1
LDLIBS=

SUBDIRS = . Core Util Widgets
SUBDIRSCLEAN=$(addsuffix clean,$(SUBDIRS))

RES_TTF:=$(wildcard Resources/*.ttf)
RES_PNG:=$(wildcard Resources/*.png)
SRCS:=$(wildcard *.cpp) $(wildcard */*.cpp)
SRCS:=$(filter-out dllmain.cpp Util/WinResource.cpp, $(SRCS))
OBJS=$(subst .cpp,.o,$(SRCS))
TTF_OBJS=$(subst .ttf,.o$,$(RES_TTF))
PNG_OBJS=$(subst .png,.o$,$(RES_PNG))

-include $(OBJS:.o=.d)

#link
digilib: $(OBJS) $(TTF_OBJS) $(PNG_OBJS)
	$(CXX) $(LDFLAGS) -o libcoreui.so.1.0 $(OBJS) $(TTF_OBJS) $(PNG_OBJS)
	ln -fs libcoreui.so.1.0 libcoreui.so.1
	ln -fs libcoreui.so.1 libcoreui.so

%.o: %.cpp
	$(CXX) $(CPPFLAGS) -g -o $*.o -c $*.cpp
	$(CXX) $(CPPFLAGS) -MM $*.cpp > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

%.o: %.ttf
	ld -r -b binary -o $*.o $*.ttf	
	
%.o: %.png
	ld -r -b binary -o $*.o $*.png	
	
clean: $(SUBDIRSCLEAN)

clean_curdir:
	$(RM) *.o *.d *~ *.so

%clean: %
	$(MAKE) -C $< -f $(PWD)/makefile clean_curdir
	