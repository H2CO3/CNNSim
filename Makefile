OS = $(shell uname -s | tr '[[:upper:]]' '[[:lower:]]')

ifeq ($(OS), darwin)
	CXX = xcrun -sdk macosx clang++
	LIBNAME = libCNN.dylib
	LIB_CXFLAGS =
	LIB_LDFLAGS = -dynamiclib -install_name /usr/local/lib/$(LIBNAME)
else
	CXX = g++-5
	LIBNAME = libCNN.so
	LIB_CXFLAGS = -fPIC
	LIB_LDFLAGS = -fPIC -shared -Wl,-soname,/usr/local/lib/$(LIBNAME)
endif

LD = $(CXX)

GSL_CFLAGS = $(shell pkg-config gsl    --cflags)
PNG_CFLAGS = $(shell pkg-config libpng --cflags)
SDL_CFLAGS = $(shell pkg-config sdl2   --cflags)

GSL_LIBS = $(shell pkg-config gsl    --libs)
PNG_LIBS = $(shell pkg-config libpng --libs)
SDL_LIBS = $(shell pkg-config sdl2   --libs)


CXFLAGS = -c \
          -std=c++14 \
          -Wall \
          -O3 \
          -flto \
          -I /usr/local/include \
          -UNDEBUG \
          $(GSL_CFLAGS) $(PNG_CFLAGS) $(SDL_CFLAGS)

LDFLAGS = -O3 \
          -flto \
          -Wl,-w

LIB_OBJECTS = CNN.o imgproc.o template.o

all: CNN

CNN: main.o $(LIBNAME)
	$(LD) -o $@ main.o -L. -lCNN $(SDL_LIBS)

$(LIBNAME): $(LIB_OBJECTS)
	$(LD) -o $@ $^ $(LDFLAGS) $(LIB_LDFLAGS) $(GSL_LIBS) $(PNG_LIBS)

main.o: main.cc
	$(CXX) $(CXFLAGS) -o $@ $<

%.o: %.cc
	$(CXX) $(CXFLAGS) $(LIB_CXFLAGS) -o $@ $<

install: CNN $(LIBNAME)
	cp CNN /usr/local/bin/
	cp $(LIBNAME) /usr/local/lib/

clean:
	rm -f *.o CNN $(LIBNAME)

.PHONY: all clean install
