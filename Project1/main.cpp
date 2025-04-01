#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <vector>
#include "color.h"
#include "vec3.h"
#include "ray.h"

double hit_sphere(const point3& center, double radius, const ray& r)
{
	vec3 oc = center - r.origin();
	auto a = dot(r.direction(), r.direction());
	auto h =dot(r.direction(), oc);
	auto c = dot(oc, oc) - radius * radius;
	auto discriminant = h*h - a * c;
	if (discriminant < 0)
	{
		return -1.0;
	}
	else
	{
		return (h - std::sqrt(discriminant)) / a;
	}
	
}
color ray_color(const ray& r)
{
	auto t = hit_sphere(point3(0, 0, -1), 0.5, r);
	if (t > 0.0)
	{
		vec3 N = unit_vector(r.at(t) - vec3(0, 0, -1));
		return 0.5 * color(N.x() + 1, N.y() + 1, N.z() + 1);
	}
	vec3 unit_direction = unit_vector(r.direction());
	auto a = 0.5 * (unit_direction.y() + 1.0);
	return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
}
int main()
{
	auto aspect_ratio = 16.0 / 9.0;
	int  image_width = 400;

	int image_heigth = int(image_width / aspect_ratio);
	image_heigth = (image_heigth < 1) ? 1 : image_heigth;
	const int channels = 3;

	//Camera
	auto focal_length = 1.0;
	auto viewport_height = 2.0;
	auto viewport_width = viewport_height * (double(image_width) / image_heigth);
	auto camera_center = point3(0, 0, 0);

	//计算视口中的向量，用于划分视口中的像素
	auto viewport_u = vec3(viewport_width, 0, 0);
	auto viewport_v = vec3(0, -viewport_height, 0);

	//划分像素，把视口划分成一格一格相等的像素格
	auto pixel_delta_u = viewport_u / image_width;
	auto pixel_delta_v = viewport_v / image_heigth;

	auto viewport_upper_left = camera_center - vec3(0, 0, focal_length) - viewport_u / 2 - viewport_v / 2;
	auto pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

	std::vector<unsigned char> image_data(image_width * image_heigth * channels);

	for (int j = 0; j < image_heigth; j++)
	{
		std::clog << "\rScanlines remaining: " << (image_heigth - j) << ' ' << std::flush;
		for (int i = 0; i < image_width; i++)
		{
			auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
			auto ray_direction = pixel_center - camera_center;
			ray r(camera_center, ray_direction);
			
			color pixel_color = ray_color(r);
			write_color(image_data, pixel_color, i, j, image_width, image_heigth, channels);
			
		}
	}
	std::clog << "\rDone.                        \n";
	stbi_write_png("image.png", image_width, image_heigth, channels, image_data.data(), image_width * channels);
	std::cout << "Image saved to output.png\n";
	return 0;
}