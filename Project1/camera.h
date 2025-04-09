#ifndef CAMERA_H
#define CAMERA_H


#include "stb_image_write_adapter.h"
#include "hittable.h"
#include "material.h"
#include "Threadpool.h"


class camera {
public:
	double aspect_ratio = 16.0 / 9.0;
	int image_width = 400;
	int channels = 3;
	int samples_per_pixel = 10;
	int max_depth = 10;

	double vfov = 90;
	point3 lookfrom = point3(0, 0, 0);
	point3 lookat = point3(0, 0, -1);
	vec3 vup = vec3(0, 1, 0);

	double defocus_angle = 0;
	double focus_dist = 0;

	
	ThreadPool pool;
	std::vector<std::future<void>> futures;

	camera() : pool(7), aspect_ratio(16.0/9.0), image_width(400), channels(3), samples_per_pixel(10),max_depth(10)
		,vfov(90),lookfrom(point3(0,0,0)), lookat(point3(0,0,-1)), vup(vec3(0,1,0)),defocus_angle(0),focus_dist(0)
	{

	}
	
	void render(const hittable& world)
	{
		initialize();
		
		std::vector<unsigned char> image_data(image_width * image_height * channels);
		for (int j = 0; j < image_height; j++)
		{
			std::atomic<int> remaining = image_height;
			
			//std::thread temp(&camera::render_row,this, j, std::ref(image_data), std::ref(world)); 左值引用，不能使用！！
			//threads.emplace_back(std::thread(&camera::render_row, this, j, std::ref(image_data), std::ref(world)));
			futures.emplace_back(pool.enqueue([&, j] {
				this->render_row(j, std::ref(image_data), std::ref(world));
				std::clog << "\rScanlines remaining: " << --remaining << ' ' << std::flush;
				}));
			
			//render_row(j, image_data, world);
			
		}
		for (auto& f : futures)
			f.get();  // 等任务完成
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
	vec3 u, v, w;
	vec3 defocus_disk_u;
	vec3 defocus_disk_v;
	
	void initialize()
	{
		image_height = int(image_width / aspect_ratio);
		image_height = (image_height < 1) ? 1 : image_height;

		pixel_samples_scale = 1.0 / samples_per_pixel;
		center = lookfrom;

		//判断viewport的大小
		
		auto theta = degrees_to_raians(vfov);
		auto h = std::tan(theta / 2);
		auto viewport_height = 2 * h * focus_dist;
		auto viewport_width = viewport_height * (double(image_width) / image_height);

		w = unit_vector(lookfrom - lookat);
		u = unit_vector(cross(vup, w));
		v = cross(w, u);
		//计算viewport边长的向量
		auto viewport_u = viewport_width * u;
		auto viewport_v = viewport_height * -v;

		pixel_delta_u = viewport_u / image_width;
		pixel_delta_v = viewport_v / image_height;

		auto viewport_upper_left = center - (focus_dist * w) - viewport_u/2 - viewport_v/2;
		pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);
		
		auto defocus_radius = focus_dist * std::tan(degrees_to_raians(defocus_angle / 2));
		defocus_disk_u = u * defocus_radius;
		defocus_disk_v = v * defocus_radius;
	}
	ray get_ray(int i, int j) const
	{
		auto offset = sample_square();
		auto pixel_sample = pixel00_loc
			+ ((i + offset.x()) * pixel_delta_u)
			+ ((j + offset.y()) * pixel_delta_v);
		auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
		auto ray_direction = pixel_sample - ray_origin;
		return ray(ray_origin, ray_direction);
	}

	vec3 sample_square() const
	{
		return vec3(random_double() - 0.5, random_double() - 0.5, 0);
	}

	point3  defocus_disk_sample() const
	{
		auto p = random_in_unit_disk();
		return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
	}
	color ray_color(const ray& r,int depth, const hittable& world) const
	{
		if (depth <= 0)
		{
			return color(0, 0, 0);
		}
		hit_record rec;
		if (world.hit(r, interval(0.000000000000001, infinity), rec))
		{
			ray scattered;
			color attenuation;
			if (rec.mat->scatter(r, rec, attenuation, scattered))
				return attenuation * ray_color(scattered, depth - 1, world);
		
			return color(0,0,0);
		}

		vec3 unit_diretion = unit_vector(r.direction());
		auto a = 0.5 * (unit_diretion.y() + 1.0);
		return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);

	}
};
#endif