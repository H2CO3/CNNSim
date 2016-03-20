#include <cstdio>
#include <chrono>

#include <SDL2/SDL.h>

#include "3rdparty/optionparser.h"

#include "CNN.hh"
#include "template.hh"
#include "imgproc.hh"


int main(int argc, char *argv[])
{
	// Load input image
	auto img = load_png_file(argv[1]);

	// Load hole filler template
	auto tem = load_template_file(argv[2]);

	// Generate Initial State
	std::vector<double> initState(img.width * img.height, 1);
	for (std::ptrdiff_t r = 0; r < img.height; r++) {
		for (std::ptrdiff_t c = 0; c < img.width; c++) {
			if (r == 0 || c == 0 || r == img.height - 1 || c == img.width - 1) {
				initState[to_index(r, c, img.width)] = 0;
			}
		}
	}

	// Construct simulator
	CNN cnn(
		img.width,
		img.height,
		img.buf, // initState,
		img.buf,
		tem,
		500
	);


	const int pixel_size = 3;

	SDL_Init(SDL_INIT_VIDEO);

	auto *window = SDL_CreateWindow(
		"CNN State",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		img.width * pixel_size,
		img.height * pixel_size,
		SDL_WINDOW_SHOWN
	);
	auto *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	std::printf("Press any key to start simulation...\n");

	bool start = false;
	bool run = true;

	while (not start) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_KEYUP) {
				start = true;
			}
		}
	}

	// Start simulation, draw every 10th frame
	double t = 0;
	int step = 0;

	auto t0 = std::chrono::steady_clock::now();

	while (run && cnn.step(&t)) {
		// Stay responsive
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				run = false;
			}
		}

		if (step++ % 10) {
			continue;
		}

		std::transform(cnn.state().begin(), cnn.state().end(), img.buf.begin(), y);

		SDL_RenderClear(renderer);

		for (std::ptrdiff_t y = 0; y < img.height; y++) {
			for (std::ptrdiff_t x = 0; x < img.width; x++) {
				Uint8 g = (1 - img.buf[to_index(y, x, img.width)]) / 2 * 255;
				SDL_SetRenderDrawColor(renderer, g, g, g, 255);
				SDL_Rect r;
				r.x = x * pixel_size;
				r.y = y * pixel_size;
				r.w = pixel_size;
				r.h = pixel_size;
				SDL_RenderFillRect(renderer, &r);
			}
		}

		SDL_RenderPresent(renderer);
	}

	auto t1 = std::chrono::steady_clock::now();
	auto dt = std::chrono::duration<double>(t1 - t0).count();
	std::printf("Simulation finished in %.3f seconds\n", dt);

	while (run) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				run = false;
			}
		}
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
