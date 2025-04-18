#ifndef HITTABLE_H
#define HITTABLE_H

#include "common.h"

class material;
class hit_record {
public:
	point3 p;
	vec3 normal;
	shared_ptr<material> mat;
	double t;
	bool front_face;

	void set_face_normal(const ray& r, const vec3& outward_normal)
	{
		if (dot(r.direction(), outward_normal) > 0.0)
		{
			normal = -outward_normal;
			front_face = false;
		}
		else
		{
			normal = outward_normal;
			front_face = true;
		}
	}
};

class hittable {
public:
	virtual ~hittable() = default;
	virtual bool hit(const ray& r,interval ray_t, hit_record& rec) const = 0;
};
#endif