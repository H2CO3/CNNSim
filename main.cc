//
// main.cc
// CNNSim, a simple CNN simulator
//
// Created by Arpad Goretity on 17/03/2016
//
// Licensed under the 2-clause BSD License
//

#include <cstdio>
#include <chrono>

#include <SDL2/SDL.h>

#include "CNN.hh"
#include "template.hh"
#include "imgproc.hh"
#include "3rdparty/optionparser.h"


enum CNNOpt {
	Invalid,
	State,
	Input,
	Templ,
	Duration,
	Output,
	RelTol,
	AbsTol,
};


static option::ArgStatus required_arg(const option::Option& option, bool msg)
{
	if (option.arg) {
		return option::ARG_OK;
	}

	if (msg) {
		std::fprintf(stderr, "Error: option '%.*s' requires an argument\n", option.namelen, option.name);
	}

	return option::ARG_ILLEGAL;
}

int main(int argc, char *argv[])
{
	// CNN parameters
	GrayscaleImage x; // initial state
	GrayscaleImage u; // input
	Template tem;
	double t_max = 0.0; // duration
	double rel_tol = 1e-3;
	double abs_tol = 1e-3;

	// output configuration
	const char *out_file = nullptr;
	GrayscaleImage out_image;

	// Command-line options
	const option::Descriptor desc[] = {
		{ CNNOpt::Invalid,  0, "",  "",         option::Arg::None, "Usage: CNN <options>\n\nOptions:\n"    },
		{ CNNOpt::State,    0, "s", "state",    required_arg,      "   -s, --state    Initial state image" },
		{ CNNOpt::Input,    0, "i", "input",    required_arg,      "   -i, --input    Input image"         },
		{ CNNOpt::Templ,    0, "t", "template", required_arg,      "   -t, --template Template file"       },
		{ CNNOpt::Duration, 0, "d", "duration", required_arg,      "   -d, --duration Simulation time"     },
		{ CNNOpt::Output,   0, "o", "outfile",  required_arg,      "   -o, --outfile  Output image file"   },
		{ CNNOpt::RelTol,   0, "r", "rel-tol",  required_arg,      "   -r, --rel-tol  Relative tolerance"  },
		{ CNNOpt::AbsTol,   0, "a", "abs-tol",  required_arg,      "   -a, --abs-tol  Absolute tolerance"  },
		{ 0,                0, nullptr, nullptr,     nullptr,      nullptr }
	};

	std::vector<option::Option> options(argc);
	std::vector<option::Option> buffer(argc);

	argc--;
	argv++;

	option::Parser parser(true, desc, argc, argv, &options[0], &buffer[0]);

	if (parser.error() || argc <= 0) {
		option::printUsage(std::cerr, desc);
		return 1;
	}

	if (auto opt = options[CNNOpt::Invalid]) {
		std::fprintf(stderr, "unrecognized option: '%.*s'\n", opt.namelen, opt.name);
		return 1;
	}

	if (auto opt = options[CNNOpt::State]) {
		x = load_png_file(opt.last()->arg);
	} else {
		std::fprintf(stderr, "Must specify initial state\n");
		return 1;
	}

	if (auto opt = options[CNNOpt::Input]) {
		u = load_png_file(opt.last()->arg);
	} else {
		std::fprintf(stderr, "Must specify input image\n");
		return 1;
	}

	if (auto opt = options[CNNOpt::Templ]) {
		tem = load_template_file(opt.last()->arg);
	} else {
		std::fprintf(stderr, "Must specify template\n");
		return 1;
	}

	if (auto opt = options[CNNOpt::Duration]) {
		t_max = std::strtod(opt.last()->arg, nullptr);
	} else {
		std::fprintf(stderr, "Must specify duration of simulation\n");
		return 1;
	}

	if (auto opt = options[CNNOpt::Output]) {
		out_file = opt.last()->arg;
	}

	if (auto opt = options[CNNOpt::RelTol]) {
		rel_tol = std::strtod(opt.last()->arg, nullptr);
	}

	if (auto opt = options[CNNOpt::AbsTol]) {
		abs_tol = std::strtod(opt.last()->arg, nullptr);
	}

	// Construct simulator
	CNN cnn(
		x.width,
		x.height,
		x.buf,
		u.buf,
		tem,
		t_max,
		rel_tol,
		abs_tol
	);

	auto conv_state_to_output = [](GrayscaleImage *dst, const CNN &src) {
		dst->width = src.width;
		dst->height = src.height;
		dst->buf.resize(src.dimension);
		std::transform(src.state().begin(), src.state().end(), dst->buf.begin(), y);
	};

	auto stopwatch = [](auto fn) {
		auto t0 = std::chrono::steady_clock::now();
		fn();
		auto t1 = std::chrono::steady_clock::now();

		auto dt = std::chrono::duration<double>(t1 - t0).count();
		std::printf("Simulation completed in %.3f seconds\n", dt);
	};

	// If an output file is specified, write final output into it and exit.
	if (out_file) {
		stopwatch([&]{ cnn.run(); });
		conv_state_to_output(&out_image, cnn);
		return save_png_file(out_file, out_image) ? 0 : 1;
	}

	// Otherwise, render CNN state step by step.
	SDL_Init(SDL_INIT_VIDEO);

	const int pixel_size = 3;

	auto *window = SDL_CreateWindow(
		"CNN Output",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		cnn.width * pixel_size,
		cnn.height * pixel_size,
		SDL_WINDOW_SHOWN
	);
	auto *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	stopwatch([&]{
		std::size_t step = 0;

		cnn.run_with_handler([&](double) {
			// draw every 10th frame only because drawing appears to be slow
			if (++step % 10) {
				return true;
			}

			conv_state_to_output(&out_image, cnn);

			SDL_RenderClear(renderer);

			for (std::ptrdiff_t r = 0; r < out_image.height; r++) {
				for (std::ptrdiff_t c = 0; c < out_image.width; c++) {
					Uint8 g = (1 - out_image.buf[to_index(r, c, out_image.width)]) / 2 * 255;
					SDL_SetRenderDrawColor(renderer, g, g, g, 255);
					SDL_Rect R;
					R.x = c * pixel_size;
					R.y = r * pixel_size;
					R.w = pixel_size;
					R.h = pixel_size;
					SDL_RenderFillRect(renderer, &R);
				}
			}

			SDL_RenderPresent(renderer);

			// Stay responsive
			SDL_Event event;
			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_QUIT) {
					return false;
				}
			}

			return true;
		});
	});

	bool run = true;
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
