#ifndef MESH_H
#define MESH_H

#include "primitives.h"

#include <iostream>
using std::cout;
using std::endl;

#include <fstream>
using std::ifstream;
using std::ofstream;

#include <iomanip>
using std::setiosflags;

#include <ios>
using std::ios_base;
using std::ios;

#include <set>
using std::set;

#include <vector>
using std::vector;

#include <limits>
using std::numeric_limits;

#include <cstring> // for memcpy()
#include <cctype>


class indexed_mesh
{
public:
	indexed_mesh(void)
	{
		finalized = false;
	}

	bool operator==(const indexed_mesh &right);
	bool operator!=(const indexed_mesh &right);

	void init_triangle_insertion(void);
	void insert_triangle(const triangle &src_tri);
	void finalize_triangle_insertion(void);
	bool save_to_binary_stereo_lithography_file(const char *const file_name, const size_t buffer_width = 65536);

	float get_x_extent(void);
	float get_y_extent(void);
	float get_z_extent(void);
	float get_triangle_area(const size_t tri_index);
	float get_area(void);
	float get_triangle_volume(const size_t tri_index);
	float get_volume(void);
	size_t get_degenerate_triangle_count(void);
	size_t get_problem_edge_count(void);
	size_t get_triangle_count(void);
	size_t get_vertex_count(void);

protected:
	void clear(void);
	void get_triangles_shared_by_vertex_pair(const size_t v0, const size_t v1, vector<size_t> &triangle_indices);

	vector<vertex_3> vertices;
	vector<indexed_triangle> triangles;
	vector< vector<size_t> > vertex_to_vertex_indices;
	vector< vector<size_t> > vertex_to_triangle_indices;

	bool finalized;
	set<indexed_vertex_3> vertex_set;
};


#endif
