//
// imgproc.hh
// CNNSim, a simple CNN simulator
//
// Created by Arpad Goretity on 17/03/2016
//
// Licensed under the 2-clause BSD License
//

#ifndef CNNSIM_IMGPROC_HH
#define CNNSIM_IMGPROC_HH

#include <cstddef>
#include <vector>


struct GrayscaleImage {
	std::vector<double> buf;
	std::ptrdiff_t width;
	std::ptrdiff_t height;

	void clear();
};


GrayscaleImage load_png_file(const char *fname);
GrayscaleImage load_png_handle(std::FILE *file);
GrayscaleImage load_png_memory(const void *data, std::ptrdiff_t size);

// Compute a flat index from row major format
static inline std::ptrdiff_t to_index(std::ptrdiff_t i, std::ptrdiff_t j, std::ptrdiff_t width)
{
	return i * width + j;
}

#endif // CNNSIM_IMGPROC_HH
