#include "mesh.h"


bool indexed_mesh::operator==(const indexed_mesh &right)
{
	if(triangles == right.triangles &&	vertices == right.vertices && vertex_to_triangle_indices == right.vertex_to_triangle_indices &&	vertex_to_vertex_indices == right.vertex_to_vertex_indices)
		return true;

	return false;
}

bool indexed_mesh::operator!=(const indexed_mesh &right)
{
	return !(*this == right);
}

void indexed_mesh::init_triangle_insertion(void)
{
	clear();
	finalized = false;
}

void indexed_mesh::insert_triangle(const triangle &src_tri)
{
	indexed_triangle t;
	triangles.push_back(t);
	size_t tri_index = triangles.size() - 1;

	// For each of the three vertices in the triangle.
	for(short unsigned int j = 0; j < 3; j++)
	{
		indexed_vertex_3 v;
		v.x = src_tri.vertex[j].x;
		v.y = src_tri.vertex[j].y;
		v.z = src_tri.vertex[j].z;

		// Look for vertex in set.
		set<indexed_vertex_3>::const_iterator find_iter = vertex_set.find(v);

		// If vertex not found in set...
		if(vertex_set.end() == find_iter)
		{
			// Assign new vertices index
			v.index = vertices.size();

			// Add vertex to set
			vertex_set.insert(v);

			// Add vertex to vector
			vertex_3 indexless_vertex;
			indexless_vertex.x = v.x;
			indexless_vertex.y = v.y;
			indexless_vertex.z = v.z;

			vertices.push_back(indexless_vertex);

			// Assign vertex index to triangle
			triangles[tri_index].vertex_indices[j] = v.index;

			// Add triangle index to vertex
			vector<size_t> tri_indices;
			tri_indices.push_back(tri_index);
			vertex_to_triangle_indices.push_back(tri_indices);
		}
		else
		{
			// Assign existing vertex index to triangle
			triangles[tri_index].vertex_indices[j] = find_iter->index;

			// Add triangle index to vertex
			vertex_to_triangle_indices[find_iter->index].push_back(tri_index);
		}
	} // End of: for(short unsigned int j = 0; j < 3; j++) ...
}

void indexed_mesh::finalize_triangle_insertion(void)
{
	if(0 == triangles.size())
	{
		finalized = true;
		return;
	}

	vertex_to_vertex_indices.resize(vertices.size());

	for(size_t i = 0; i < vertex_to_triangle_indices.size(); i++)
	{
		// Use a temporary set to avoid duplicates.
		set<size_t> vertex_to_vertex_indices_set;

		for(size_t j = 0; j < vertex_to_triangle_indices[i].size(); j++)
		{
			size_t tri_index = vertex_to_triangle_indices[i][j];

			for(size_t k = 0; k < 3; k++)
				if(i != triangles[tri_index].vertex_indices[k]) // Don't add current vertex index to its own adjacency list.
					vertex_to_vertex_indices_set.insert(triangles[tri_index].vertex_indices[k]);
		}

		// Copy to final vector.
		for(set<size_t>::const_iterator ci = vertex_to_vertex_indices_set.begin(); ci != vertex_to_vertex_indices_set.end(); ci++)
			vertex_to_vertex_indices[i].push_back(*ci);
	}

	finalized = true;
	vertex_set.clear();
}

bool indexed_mesh::save_to_binary_stereo_lithography_file(const char *const file_name, const size_t buffer_width)
{
	if(false == finalized)
		return false;

	if(0 == triangles.size())
		return false;

	// Write to file.
	ofstream out(file_name, ios_base::binary);

	if(out.fail())
		return false;

	const size_t header_size = 80;
	vector<char> buffer(header_size, 0);
	const unsigned int num_triangles = triangles.size(); // Must be 4-byte unsigned int.
	vertex_3 normal;

	// Write blank header.
	out.write(reinterpret_cast<const char *>(&(buffer[0])), header_size);

	// Write number of triangles.
	out.write(reinterpret_cast<const char *>(&num_triangles), sizeof(unsigned int));

	// Enough bytes for twelve 4-byte floats plus one 2-byte integer, per triangle.
	const size_t per_triangle_data_size = (12*sizeof(float) + sizeof(short unsigned int));
	const size_t buffer_size = per_triangle_data_size * buffer_width;
	buffer.resize(buffer_size, 0);

	// Use a pointer to assist with the copying.
	// Should probably use std::copy() instead, but memcpy() does the trick, so whatever...
	char *cp = &buffer[0];
	size_t buffer_count = 0;

	cout << "Writing " << per_triangle_data_size*triangles.size() / 1048576 << " MB of data to disk" << endl;

	for(size_t i = 0; i < triangles.size(); i++)
	{
		vertex_3 v0 = vertices[triangles[i].vertex_indices[1]] - vertices[triangles[i].vertex_indices[0]];
		vertex_3 v1 = vertices[triangles[i].vertex_indices[2]] - vertices[triangles[i].vertex_indices[0]];
		normal = v0.cross(v1);
		normal.normalize();

		memcpy(cp, &normal.x, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &normal.y, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &normal.z, sizeof(float)); cp += sizeof(float);

		memcpy(cp, &vertices[triangles[i].vertex_indices[0]].x, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &vertices[triangles[i].vertex_indices[0]].y, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &vertices[triangles[i].vertex_indices[0]].z, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &vertices[triangles[i].vertex_indices[1]].x, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &vertices[triangles[i].vertex_indices[1]].y, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &vertices[triangles[i].vertex_indices[1]].z, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &vertices[triangles[i].vertex_indices[2]].x, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &vertices[triangles[i].vertex_indices[2]].y, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &vertices[triangles[i].vertex_indices[2]].z, sizeof(float)); cp += sizeof(float);

		cp += sizeof(short unsigned int);

		buffer_count++;

		// If buffer is full, write triangles in buffer to disk.
		if(buffer_count == buffer_width)
		{
			out.write(reinterpret_cast<const char *>(&buffer[0]), buffer_size);

			if(out.fail())
				return false;

			buffer_count = 0;
			cp = &buffer[0];
		}
	}

	// Write any remaining triangles in buffer to disk.
	// This will occur whenever triangles.size() % buffer_width != 0
	// (ie. when triangle count is not a multiple of buffer_width, which should happen almost all of the time).
	if(buffer_count > 0)
	{
		out.write(reinterpret_cast<const char *>(&buffer[0]), per_triangle_data_size*buffer_count);

		if(out.fail())
			return false;
	}

	out.close();

	return true;
}

float indexed_mesh::get_x_extent(void)
{
	float min = numeric_limits<float>::max();
	float max = numeric_limits<float>::min();

	for(size_t i = 0; i < vertices.size(); i++)
	{
		if(vertices[i].x < min)
			min = vertices[i].x;

		if(vertices[i].x > max)
			max = vertices[i].x;
	}

	return fabsf(min - max);
}

float indexed_mesh::get_y_extent(void)
{
	float min = numeric_limits<float>::max();
	float max = numeric_limits<float>::min();

	for(size_t i = 0; i < vertices.size(); i++)
	{
		if(vertices[i].y < min)
			min = vertices[i].y;

		if(vertices[i].y > max)
			max = vertices[i].y;
	}

	return fabsf(min - max);
}

float indexed_mesh::get_z_extent(void)
{
	float min = numeric_limits<float>::max();
	float max = numeric_limits<float>::min();

	for(size_t i = 0; i < vertices.size(); i++)
	{
		if(vertices[i].z < min)
			min = vertices[i].z;

		if(vertices[i].z > max)
			max = vertices[i].z;
	}

	return fabsf(min - max);
}

float indexed_mesh::get_triangle_area(const size_t tri_index)
{
	if(tri_index >= triangles.size())
		return 0;

	vertex_3 a = vertices[triangles[tri_index].vertex_indices[1]] - vertices[triangles[tri_index].vertex_indices[0]];
	vertex_3 b = vertices[triangles[tri_index].vertex_indices[2]] - vertices[triangles[tri_index].vertex_indices[0]];
	vertex_3 cross = a.cross(b);

	return 0.5f*cross.length();
}

float indexed_mesh::get_area(void)
{
	float total_area = 0;

	for(size_t i = 0; i < triangles.size(); i++)
		total_area += get_triangle_area(i);

	return total_area;
}

float indexed_mesh::get_triangle_volume(const size_t tri_index)
{
	if(tri_index >= triangles.size())
		return 0;

	vertex_3 a = vertices[triangles[tri_index].vertex_indices[0]];
	vertex_3 b = vertices[triangles[tri_index].vertex_indices[1]];
	vertex_3 c = vertices[triangles[tri_index].vertex_indices[2]];

	return a.dot(b.cross(c)) / 6.0f;
}

float indexed_mesh::get_volume(void)
{
	float total_volume = 0;

	for(size_t i = 0; i < triangles.size(); i++)
		total_volume += get_triangle_volume(i);

	return total_volume;
}

size_t indexed_mesh::get_degenerate_triangle_count(void)
{
	size_t degenerate_count = 0;

	for(size_t i = 0; i < triangles.size(); i++)
	{
		size_t v0 = triangles[i].vertex_indices[0];
		size_t v1 = triangles[i].vertex_indices[1];
		size_t v2 = triangles[i].vertex_indices[2];

		if( vertices[v0] == vertices[v1] ||
			vertices[v0] == vertices[v2] ||
			vertices[v1] == vertices[v2] )
		{
			degenerate_count++;
		}
	}

	return degenerate_count;
}

size_t indexed_mesh::get_problem_edge_count(void)
{
	// Find edges that belong to only one triangle.
	set<ordered_indexed_edge> problem_edges_set;
	size_t problem_edge_id = 0;

	// For each vertex.
	for(size_t i = 0; i < vertices.size(); i++)
	{
		// For each edge.
		for(size_t j = 0; j < vertex_to_vertex_indices[i].size(); j++)
		{
			size_t neighbour_j = vertex_to_vertex_indices[i][j];

			vector<size_t> tri_indices;
			get_triangles_shared_by_vertex_pair(i, neighbour_j, tri_indices);

			// Found a problem edge.
			if(2 != tri_indices.size())
			{
				indexed_vertex_3 v0;
				v0.index = i;
				v0.x = vertices[i].x;
				v0.y = vertices[i].y;
				v0.z = vertices[i].z;

				indexed_vertex_3 v1;
				v1.index = neighbour_j;
				v1.x = vertices[neighbour_j].x;
				v1.y = vertices[neighbour_j].y;
				v1.z = vertices[neighbour_j].z;

				ordered_indexed_edge problem_edge(v0, v1);

				if(problem_edges_set.end() == problem_edges_set.find(problem_edge))
				{
					problem_edge.id = problem_edge_id++;
					problem_edges_set.insert(problem_edge);
				}
			} // End of: Found a problem edge.
		} // End of: For each edge.
	} // End of: For each vertex.

	return problem_edges_set.size();
}

size_t indexed_mesh::get_triangle_count(void)
{
	return triangles.size();
}

size_t indexed_mesh::get_vertex_count(void)
{
	return vertices.size();
}

void indexed_mesh::clear(void)
{
	triangles.clear();
	vertices.clear();
	vertex_to_triangle_indices.clear();
	vertex_to_vertex_indices.clear();
	vertex_set.clear();
}

void indexed_mesh::get_triangles_shared_by_vertex_pair(const size_t v0, const size_t v1, vector<size_t> &triangle_indices)
{
	triangle_indices.clear();

	for(size_t i = 0; i < vertex_to_triangle_indices[v0].size(); i++)
	{
		for(size_t j = 0; j < vertex_to_triangle_indices[v1].size(); j++)
		{
			size_t tri0_index = vertex_to_triangle_indices[v0][i];
			size_t tri1_index = vertex_to_triangle_indices[v1][j];

			if(tri0_index == tri1_index)
			{
				triangle_indices.push_back(tri0_index);
				break;
			}
		}
	}
}
