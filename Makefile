open-bmp:
	gcc open-bmp.c -I/usr/include/SDL2 -D_REENTRANT -lSDL2 -o open-bmp

clean:
	rm open-bmp
