OS = $(shell uname -s | tr '[[:upper:]]' '[[:lower:]]')

ifeq ($(OS), darwin)
	CXX = xcrun -sdk macosx clang++
else
	CXX = g++
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
          -Wl,-w \
          $(GSL_LIBS) $(PNG_LIBS) $(SDL_LIBS)

OBJECTS = main.o CNN.o imgproc.o template.o


all: CNN

CNN: $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.cc
	$(CXX) $(CXFLAGS) -o $@ $<

install: CNN
	cp $< /usr/local/bin/

clean:
	rm -f $(OBJECTS) CNN

.PHONY: all clean
