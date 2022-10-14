// Source code by Shawn Halayka
// Source code is in the public domain

#ifndef QJS_H
#define QJS_H




#define QJS_VERSION_NUMBER "3.0.5"



#include <cstdlib> // Include this before before glut.h for the sake of MSVC++

#include "GL/glew.h"
#include "GL/glut.h"

// Automatically link in the GLUT and GLEW libraries if compiling on MSVC++
#ifdef _MSC_VER
	#pragma comment(lib, "glew32")
//	#pragma comment(lib, "glut32")
#endif



#include "primitives.h"
#include "mesh.h"
#include "marching_cubes.h"
using marching_cubes::mc_grid_cube;
using marching_cubes::max_triangles_per_mc_cell;
using marching_cubes::mc_edge_table;
using marching_cubes::mc_tri_table;

#include "quaternion_math.h"
#include "eqparse.h"

#include "string_utilities.h"
using string_utilities::lower_string;
using string_utilities::trim_whitespace_string;
using string_utilities::stl_str_tok;
using string_utilities::is_real_number;
using string_utilities::is_unsigned_int;



#include <cstring> // For memcpy()
#include <ctime>

#include <iostream>
using std::cout;
using std::endl;


#include <fstream>
using std::ifstream;
using std::ofstream;

#include <cmath>

#include <vector>
using std::vector;

#include <set>
using std::set;

#include <string>
using std::string;

#include <sstream>
using std::ostringstream;
using std::istringstream;

#include <iomanip>
using std::ios_base;
//using std::setprecision;
//using std::fixed;

#include <utility>
using std::pair;


class addsub_block
{
public:

	addsub_block(void)
	{
		additive = true;
		start_x = end_x = 0;
		start_y = end_y = 0;
		start_z = end_z = 0;
	}

	bool additive;

	float start_x, end_x;
	float start_y, end_y;
	float start_z, end_z;
};


class quaternion_julia_set
{
public:
	quaternion_julia_set(const bool force_cpu);
	~quaternion_julia_set(void);

	bool load_configuration_from_file(const char *file_name);
	bool generate_and_write_isosurface_to_binary_stl_file(const char *file_name);

	inline size_t get_res(void) { return res; };
	inline size_t get_vertex_refinement_steps(void) { return vertex_refinement_steps; };
	inline float get_shell_thickness(void) { return shell_thickness; };
	inline float get_grid_min(void) { return grid_min; };
	inline float get_grid_max(void) { return grid_max; };
	inline size_t get_max_iterations(void) { return max_iterations; };
	inline float get_threshold(void) { return threshold; };
	inline float get_z_w(void) { return z_w; };
	inline float get_C_x(void) { return C.x; };
	inline float get_C_y(void) { return C.y; };
	inline float get_C_z(void) { return C.z; };
	inline float get_C_w(void) { return C.w; };
	inline string get_equation_text(void) { return equation_text; }
	inline string get_status_string(void) { return status_string; }
	string get_blocks_string(void);

protected:
	bool setup_equation_text(const string &src_formula_text, string &error_string);
	bool initialize_fragment_shader(const string &fragment_shader_code, GLint &shader);
	bool generate_fractal_set(vector<bool> &fractal_set);
	void get_surface_set(const vector<bool> &fractal_set, vector<bool> &surface);
	void thicken_shell(const vector<bool> &fractal_set, vector<bool> &shell);
	void add_to_set(vector<bool> &fractal_set, const addsub_block &b);
	void subtract_from_set(vector<bool> &fractal_set, const addsub_block &b);

	void init_grid_cube(mc_grid_cube &cube, const size_t cube_x, const size_t cube_y, const size_t cube_z, const vector<bool> &fractal_set);
	bool tesselate_set(const vector<bool> &fractal_set, indexed_mesh &m);
	void get_vertex_interp_input_from_grid_cube(const mc_grid_cube &cube, vector<float> &input0, vector<float> &input1);
	short unsigned int get_triangles_from_grid_cube(const mc_grid_cube &cube, vector<float> &output, size_t &current_vertex_index, triangle *const triangles);
	vertex_3 vertex_interp_float(vertex_3 v0, vertex_3 v1, float val_v0, float val_v1);

	size_t res;
	size_t vertex_refinement_steps;
	float shell_thickness;
	float grid_max;
	float grid_min;
	short unsigned int max_iterations;
	float threshold;
	float z_w;
	quaternion C;
	string equation_text;
	vector<addsub_block> addsub_blocks;

	float step_size;

	bool parameters_configured;

	bool force_cpu;
	bool opengl_init_ok;
	int glut_window_handle;

	quaternion_julia_set_equation_parser eqparser;

	string status_string;
};


#endif
