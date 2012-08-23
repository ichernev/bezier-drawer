CC = g++ -Wall # -ansi

# pnt.o: pnt.c pnt.h
# 	$(CC) -co pnt.o pnt.c

%.o: %.c %.h
	$(cc) -co %.o %.c

bezier: bezier.cpp pnt.o util.o rational_bezier.o
	$(CC) bezier.cpp -o bezier pnt.o util.o rational_bezier.o -lGL -lGLU `sdl-config --cflags --libs`

all: bezier

clean:
	@echo Cleaning up...
	@rm -fv bezier *.o
	@echo Done.
