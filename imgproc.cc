//
// imgproc.cc
// CNNSim, a simple CNN simulator
//
// Created by Arpad Goretity on 17/03/2016
//
// Licensed under the 2-clause BSD License
//


#include <cmath>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cassert>
#include <algorithm>

#include <png.h>

#include "imgproc.hh"


static png_image make_png_image()
{
	png_image img;
	std::memset(&img, 0, sizeof img);
	img.version = PNG_IMAGE_VERSION;
	return img;
}


// Reading images into raw memory buffers

static void complete_read(GrayscaleImage *buf, png_image *img)
{
	if (PNG_IMAGE_FAILED(*img)) {
		buf->clear();
		png_image_free(img);
		return;
	}

	img->format = PNG_FORMAT_LINEAR_Y;

	std::ptrdiff_t stride = PNG_IMAGE_ROW_STRIDE(*img);
	std::ptrdiff_t u16_bufsize = PNG_IMAGE_BUFFER_SIZE(*img, stride);
	std::vector<std::uint16_t> u16_buf(u16_bufsize / sizeof(std::uint16_t));

	int success = png_image_finish_read(img, nullptr, &u16_buf[0], stride, nullptr);

	if (success == 0) {
		buf->clear();
		png_image_free(img);
		return;
	}

	buf->width = img->width;
	buf->height = img->height;
	buf->buf.resize(u16_buf.size());

	assert(buf->buf.size() == buf->width * buf->height);

	std::transform(
		u16_buf.begin(), u16_buf.end(),
		buf->buf.begin(),
		[](auto pixel) { return 1.0 - 2.0 * pixel / UINT16_MAX; }
	);

	png_image_free(img);
}

void GrayscaleImage::clear()
{
	buf.clear();
	width = 0;
	height = 0;
}

GrayscaleImage load_png_file(const char *fname)
{
	GrayscaleImage buf;
	png_image img = make_png_image();
	png_image_begin_read_from_file(&img, fname);
	complete_read(&buf, &img);
	return buf;
}

GrayscaleImage load_png_handle(std::FILE *file)
{
	GrayscaleImage buf;
	png_image img = make_png_image();
	png_image_begin_read_from_stdio(&img, file);
	complete_read(&buf, &img);
	return buf;
}

GrayscaleImage load_png_memory(const void *data, std::ptrdiff_t size)
{
	GrayscaleImage buf;
	png_image img = make_png_image();
	png_image_begin_read_from_memory(&img, data, size);
	complete_read(&buf, &img);
	return buf;
}


// Writing files from raw memory buffers

static void init_png_write_state(
	const GrayscaleImage &buf,
	png_image *img,
	std::vector<std::uint16_t> *u16_buf
)
{
	*img = make_png_image();
	img->width = buf.width;
	img->height = buf.height;
	img->format = PNG_FORMAT_LINEAR_Y;

	u16_buf->resize(buf.buf.size());

	std::transform(
		buf.buf.begin(), buf.buf.end(),
		u16_buf->begin(),
		[](auto pixel) { return std::uint16_t((1.0 - pixel) / 2.0 * UINT16_MAX); }
	);
}

bool save_png_file(const char *fname, const GrayscaleImage &buf)
{
	png_image img;
	std::vector<std::uint16_t> u16_buf;
	init_png_write_state(buf, &img, &u16_buf);
	int status = png_image_write_to_file(&img, fname, true, &u16_buf[0], 0, nullptr);
	png_image_free(&img);
	return status != 0;
}

bool save_png_handle(std::FILE *file, const GrayscaleImage &buf)
{
	png_image img;
	std::vector<std::uint16_t> u16_buf;
	init_png_write_state(buf, &img, &u16_buf);
	int status = png_image_write_to_stdio(&img, file, true, &u16_buf[0], 0, nullptr);
	png_image_free(&img);
	return status != 0;
}
