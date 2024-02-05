#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#define N 1000000000

void assign_colors(unsigned char *arr, int start, int bpp, int *r, int *g, int *b, int *a, int *x) {
	switch (bpp) {
		// Only handles 24 bits per prixel now
		case 3:
			(*r) = arr[start+2];
			(*g) = arr[start+1];
			(*b) = arr[start];
			(*a) = 255;
			break;
	}
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];

    FILE *file = fopen(filename, "rb");

    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

	SDL_Window* window;
	SDL_Renderer* renderer;

    unsigned char *buffer;
	buffer = (unsigned char *)malloc(N);

    size_t bytesRead = fread(buffer, 1, N, file);

    if (bytesRead > 0) {
    	// Location of the first bit of actual data
		int data_offset = buffer[10] | (buffer[11] << 8) | (buffer[12] << 16) | (buffer[13] << 24);
		if (buffer[30] != 0) {
			perror("Format is not BI_RGB");
		} else {
			// Width and height of the image, in pixels (4 bytes little endian)
			int width = buffer[18] | (buffer[19] << 8) | (buffer[20] << 16) | (buffer[21] << 24);
			int height = buffer[22] | (buffer[23] << 8) | (buffer[24] << 16) | (buffer[25] << 24);
			// Number of bits per pixel (should be 24 (1 byte per color channel RGB))
			int bpp = buffer[28]/8;
			// Padding needed if the number of bytes per row (bpp*width) is not a multiple of 4
			int padding_size = (4-bpp*width%4)%4;
			int row_size = bpp*width + padding_size;
			int start_row, end_row_data;
		    window = SDL_CreateWindow("BMP Viewer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
		    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
			for (int i=0; i<height; i++) {
				start_row = data_offset+i*row_size;
				end_row_data = data_offset+i*row_size+bpp*width;
				for (int j=start_row; j<end_row_data; j+=bpp) {
					int r, g, b, a, x;
					assign_colors(buffer, j, bpp, &r, &g, &b, &a, &x);
					SDL_SetRenderDrawColor(renderer, r, g, b, a);
					// BMP stores pixel left to right and bottom to top
		    		SDL_RenderDrawPoint(renderer, ((j-data_offset)%row_size)/bpp, height-i);
				}
				// Ditch the padding bits
				for (int j=end_row_data; j<end_row_data+padding_size; j++) {
					printf("Padding: %02X ", buffer[j]);
				}
			}
		}
    } else {
        perror("Error reading file");
    }

    fclose(file);
    free(buffer);

	SDL_RenderPresent(renderer);

	// Wait until user closes the window
    SDL_Event event;
    int quit = 0;
    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = 1;
            }
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
