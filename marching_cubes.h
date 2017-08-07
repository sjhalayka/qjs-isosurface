// Modified from Paul Bourke, Polygonising a Scalar Field


#ifndef MARCHING_CUBES_H
#define MARCHING_CUBES_H


#include "primitives.h"


namespace marching_cubes
{
	class mc_grid_cube
	{
	public:
		vertex_3 vertex[8];
		bool value[8];
	};

	// Marching Cubes will make a maximum of 5 triangles per cell.
	const size_t max_triangles_per_mc_cell = 5;

	extern int mc_edge_table[256];
	extern int mc_tri_table[256][16];
};


#endif
