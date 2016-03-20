CXX = xcrun -sdk macosx clang++
LD = $(CXX)

CXFLAGS = -c \
          -std=c++11 \
          -Wall \
          -O3 \
          -flto \
          -I /usr/local/Cellar/gsl/1.16/include/ \
          -I /usr/local/include

LDFLAGS = -O3 \
          -flto \
          -Wl,-w \
          -L /usr/local/Cellar/gsl/1.16/lib/ \
          -L /usr/local/lib \
          -lgsl \
          -lpng \
          -lsdl2

OBJECTS = main.o CNN.o imgproc.o template.o


all: CNN

CNN: $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.cc
	$(CXX) $(CXFLAGS) -o $@ $<

clean:
	rm -f $(OBJECTS) CNN

.PHONY: all clean
