LIBS=`sdl2-config --libs` -lSDL2_image 
CPPFLAGS=-I. -I objects -I fonts -Wall
ENGINE_OBJECTS=screen.o  
TARGET=scroll
all: $(TARGET)

scroll: scroll.o screen.o
	$(CXX) -O2 -Wall $^ -o scroll -Iinclude $(LIBS)

clean:
	@rm -vf *.o $(TARGET)

