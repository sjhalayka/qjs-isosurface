// Source code by Shawn Halayka
// Source code is in the public domain

#ifndef PRIMITIVES_H
#define PRIMITIVES_H


#include <cmath>
#include <cstddef> // Include this for the sake of g++, or it will not recognize size_t


class vertex_3
{
public:
	inline vertex_3(void) : x(0.0f), y(0.0f), z(0.0f) {}
	inline vertex_3(const float src_x, const float src_y, const float src_z) : x(src_x), y(src_y), z(src_z) {}
	inline vertex_3(const vertex_3 &rhs) : x(rhs.x), y(rhs.y), z(rhs.z) {}

	inline bool operator==(const vertex_3 &right) const
	{
		if(x == right.x && y == right.y && z == right.z)
			return true;

		return false;
	}

	inline bool operator<(const vertex_3 &right) const
	{
		if(x < right.x)
			return true;
		else if(x > right.x)
			return false;

		if(y < right.y)
			return true;
		else if(y > right.y)
			return false;

		if(z < right.z)
			return true;
		else if(z > right.z)
			return false;

		return false;
	}

	inline bool operator>(const vertex_3 &right) const
	{
		if(x > right.x)
			return true;
		else if(x < right.x)
			return false;

		if(y > right.y)
			return true;
		else if(y < right.y)
			return false;

		if(z > right.z)
			return true;
		else if(z < right.z)
			return false;

		return false;
	}


	inline vertex_3& operator+=(const vertex_3 &right)
	{
		x += right.x;
		y += right.y;
		z += right.z;

		return *this;
	}

	inline vertex_3& operator*=(const float &right)
	{
		x *= right;
		y *= right;
		z *= right;

		return *this;
	}

	inline vertex_3& operator=(const vertex_3 &right)
	{
		x = right.x;
		y = right.y;
		z = right.z;

		return *this;
	}

	inline vertex_3 operator-(const vertex_3 &right) const
	{
		vertex_3 temp;

		temp.x = x - right.x;
		temp.y = y - right.y;
		temp.z = z - right.z;

		return temp;
	}

	inline vertex_3 operator+(const vertex_3 &right) const
	{
		vertex_3 temp;

		temp.x = x + right.x;
		temp.y = y + right.y;
		temp.z = z + right.z;

		return temp;
	}

	inline vertex_3 operator*(const float &right) const
	{
		vertex_3 temp;

		temp.x = x * right;
		temp.y = y * right;
		temp.z = z * right;

		return temp;
	}

	inline vertex_3 operator/(const float &right) const
	{
		vertex_3 temp;

		temp.x = x / right;
		temp.y = y / right;
		temp.z = z / right;

		return temp;
	}

	inline vertex_3 cross(const vertex_3 &right) const
	{
		vertex_3 temp;

		temp.x = y*right.z - z*right.y;
		temp.y = z*right.x - x*right.z;
		temp.z = x*right.y - y*right.x;

		return temp;
	}

	inline float dot(const vertex_3 &right) const
	{
		return x*right.x + y*right.y + z*right.z;
	}

	inline float self_dot(void) const
	{
		return x*x + y*y + z*z;
	}

	inline float length(void) const
	{
		return sqrt(self_dot());
	}

	inline float distance(const vertex_3 &right) const
	{
		return sqrt((right.x - x)*(right.x - x) + (right.y - y)*(right.y - y) + (right.z - z)*(right.z - z));
	}

	inline float distance_sq(const vertex_3 &right) const
	{
		return (right.x - x)*(right.x - x) + (right.y - y)*(right.y - y) + (right.z - z)*(right.z - z);
	}

	inline void normalize(void)
	{
		float len = length();

		if(0.0f != len)
		{
			x /= len;
			y /= len;
			z /= len;
		}
	}

	inline void zero(void)
	{
		x = y = z = 0;
	}

	inline void rotate_x(const float &radians)
	{
		float t_y = y;

		y = t_y*cos(radians) + z*sin(radians);
		z = t_y*-sin(radians) + z*cos(radians);
	}

	inline void rotate_y(const float &radians)
	{
		float t_x = x;

		x = t_x*cos(radians) + z*-sin(radians);
		z = t_x*sin(radians) + z*cos(radians);
	}

	float x, y, z;
};

class indexed_vertex_3 : public vertex_3
{
public:
	inline indexed_vertex_3(void) { x = y = z = 0; index = 0; }
	inline indexed_vertex_3(const float src_x, const float src_y, const float src_z, const size_t src_index ) { x = src_x; y = src_y; z = src_z; index = src_index; }
	inline indexed_vertex_3(const float src_x, const float src_y, const float src_z) { x = src_x; y = src_y; z = src_z; index = 0; }

	inline bool operator<(const vertex_3 &right) const
	{
		if(x < right.x)
			return true;
		else if(x > right.x)
			return false;

		if(y < right.y)
			return true;
		else if(y > right.y)
			return false;

		if(z < right.z)
			return true;
		else if(z > right.z)
			return false;

		return false;
	}

	inline bool operator>(const vertex_3 &right) const
	{
		if(x > right.x)
			return true;
		else if(x < right.x)
			return false;

		if(y > right.y)
			return true;
		else if(y < right.y)
			return false;

		if(z > right.z)
			return true;
		else if(z < right.z)
			return false;

		return false;
	}

	inline indexed_vertex_3& operator=(const indexed_vertex_3 &right)
	{
		x = right.x;
		y = right.y;
		z = right.z;
		index = right.index;

		return *this;
	}

	size_t index;
};

class indexed_triangle
{
public:
	size_t vertex_indices[3];

	inline bool operator==(const indexed_triangle &right) const
	{
		if( right.vertex_indices[0] == vertex_indices[0] && 
			right.vertex_indices[1] == vertex_indices[1] &&
			right.vertex_indices[2] == vertex_indices[2] )
		{
			return true;
		}

		return false;
	}
};

class triangle
{
public:
	vertex_3 vertex[3];
};

class ordered_indexed_edge
{
public:
	ordered_indexed_edge(const indexed_vertex_3 &a, const indexed_vertex_3 &b)
	{
		if(a.index < b.index)
		{
			indices[0] = a.index;
			indices[1] = b.index;
		}
		else
		{
			indices[0] = b.index;
			indices[1] = a.index;
		}
	}

	bool operator<(const ordered_indexed_edge &right) const
	{
		if(indices[0] < right.indices[0])
			return true;
		else if(indices[0] > right.indices[0])
			return false;

		if(indices[1] < right.indices[1])
			return true;
		else if(indices[1] > right.indices[1])
			return false;

		return false;
	}

	size_t indices[2];
	size_t id;
};



class quaternion
{
public:
	inline quaternion(void) : x(0.0f), y(0.0f), z(0.0f), w(0.0f) { /*default constructor*/ }
	inline quaternion(const float src_x, const float src_y, const float src_z, const float src_w) : x(src_x), y(src_y), z(src_z), w(src_w) { /* custom constructor */ }

	inline float self_dot(void)
	{
		return x*x + y*y + z*z + w*w;
	}

	float x, y, z, w;
};







#endif
