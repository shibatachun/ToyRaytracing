#ifndef CAMERA_H
#define CAMERA_H


#include "stb_image_write_adapter.h"
#include "hittable.h"
#include <thread>

class camera {
public:
	double aspect_ratio = 16.0 / 9.0;
	int image_width = 400;
	int channels = 3;
	int samples_per_pixel = 10;
	int max_depth = 10;
	std::vector<std::thread> threads;
	
	void render(const hittable& world)
	{
		initialize();
		
		std::vector<unsigned char> image_data(image_width * image_height * channels);
		for (int j = 0; j < image_height; j++)
		{
			std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
			//std::thread temp(&camera::render_row,this, j, std::ref(image_data), std::ref(world)); ��ֵ���ã�����ʹ�ã���
			threads.emplace_back(std::thread(&camera::render_row, this, j, std::ref(image_data), std::ref(world)));
			
		}
		for (auto& t : threads)
		{
			t.join();
		}
		std::clog << "\rDone.                        \n";
		stbi_write_png("image.png", image_width, image_height, channels, image_data.data(), image_width * channels);
		std::cout << "Image saved to output.png\n";
		
	}
	void render_row(int j, std::vector<unsigned char>& image_data, const hittable& world)
	{
		for (int i = 0; i < image_width; ++i)
		{
			color pixel_color(0, 0, 0);
			for (int sample = 0; sample < samples_per_pixel; ++sample)
			{
				ray r = get_ray(i, j);
				pixel_color += ray_color(r, max_depth, world);
			}
			write_color(image_data, pixel_samples_scale * pixel_color, i, j, image_width, image_height, channels);
		}
	}
	
private:
	int image_height;
	double pixel_samples_scale;
	point3 center;
	point3 pixel00_loc;
	vec3 pixel_delta_u;
	vec3 pixel_delta_v;
	
	void initialize()
	{
		image_height = int(image_width / aspect_ratio);
		image_height = (image_height < 1) ? 1 : image_height;

		pixel_samples_scale = 1.0 / samples_per_pixel;
		center = point3(0, 0, 0);

		//�ж�viewport�Ĵ�С
		auto focal_length = 1.0;
		auto viewport_height = 2.0;
		auto viewport_width = viewport_height * (double(image_width) / image_height);

		//����viewport�߳�������
		auto viewport_u = vec3(viewport_width, 0, 0);
		auto viewport_v = vec3(0, -viewport_height, 0);

		pixel_delta_u = viewport_u / image_width;
		pixel_delta_v = viewport_v / image_height;

		auto viewport_upper_left = center - vec3(0, 0, focal_length) - viewport_u / 2 - viewport_v / 2;
		pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);
		
	}
	ray get_ray(int i, int j) const
	{
		auto offset = sample_square();
		auto pixel_sample = pixel00_loc
			+ ((i + offset.x()) * pixel_delta_u)
			+ ((j + offset.y()) * pixel_delta_v);
		auto ray_origin = center;
		auto ray_direction = pixel_sample - ray_origin;
		return ray(ray_origin, ray_direction);
	}

	vec3 sample_square() const
	{
		return vec3(random_double() - 0.5, random_double() - 0.5, 0);
	}
	color ray_color(const ray& r,int depth, const hittable& world) const
	{
		if (depth <= 0)
		{
			return color(0, 0, 0);
		}
		hit_record rec;
		if (world.hit(r, interval(0, infinity), rec))
		{
			vec3 direction = random_on_hemisphere(rec.normal);
			return 0.5* ray_color(ray(rec.p, direction), depth-1,world);
		}

		vec3 unit_diretion = unit_vector(r.direction());
		auto a = 0.5 * (unit_diretion.y() + 1.0);
		return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);

	}
};
#endif