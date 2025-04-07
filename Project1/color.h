#ifndef COLOR_H
#define COLOR_H

#include "vec3.h"
#include "common.h"
#include "interval.h"


using color = vec3;

inline double linear_to_gamma(double linear_component)
{
	if (linear_component > 0)
		return std::sqrt(linear_component);
	return 0;
}
void write_color(std::vector<unsigned char>& image_data, const color& pixel_color, int i, int j, int image_width, int image_height, int channels )
{
	auto r = pixel_color.x();
	auto g = pixel_color.y();
	auto b = pixel_color.z();

	r = linear_to_gamma(r);
	g = linear_to_gamma(g);
	b = linear_to_gamma(b);

	static const interval intensity(0.000, 0.999);
	int rbyte = int(256 * intensity.clamp(r));
	int gbyte = int(256 * intensity.clamp(g));
	int bbyte = int(256 * intensity.clamp(b));

	int pixel_index = (j * image_width + i) * channels;
	image_data[pixel_index + 0] = rbyte;
	image_data[pixel_index + 1] = gbyte;
	image_data[pixel_index + 2] = bbyte;

}

#endif