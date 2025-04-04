#ifndef COLOR_H
#define COLOR_H

#include "vec3.h"
#include "common.h"


using color = vec3;

void write_color(std::vector<unsigned char>& image_data, const color& pixel_color, int i, int j, int image_width, int image_height, int channels )
{
	auto r = pixel_color.x();
	auto g = pixel_color.y();
	auto b = pixel_color.z();

	int rbyte = int(255.999 * r);
	int gbyte = int(255.999 * g);
	int bbyte = int(255.999 * b);

	int pixel_index = (j * image_width + i) * channels;
	image_data[pixel_index + 0] = rbyte;
	image_data[pixel_index + 1] = gbyte;
	image_data[pixel_index + 2] = bbyte;

}

#endif