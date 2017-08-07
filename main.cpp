// Source code by Shawn Halayka
// Source code is in the public domain


#include "quaternion_julia_set.h"


#include <iostream>
using std::cout;
using std::endl;

#include <exception>
using std::exception;

#include <new>
using std::bad_alloc;

#include <typeinfo>



bool parse_args(int argc, char **argv, bool &force_cpu);

// To do: consider using double-precision, and outputting to OBJ or Collada with large setprecision().
int main(int argc, char **argv)
{
	cout << "GPU-accelerated quaternion Julia set isosurface extractor v" << QJS_VERSION_NUMBER << endl;


	// Get command-line arguments.
	bool force_cpu = false;

	if(false == parse_args(argc, argv, force_cpu))
	{
		cout << "Example usage: " << argv[0] << " config.txt fractal.stl [-cpu]" << endl;
		return 0;
	}


	// Create quaternion Julia set object / initialize OpenGL.
	quaternion_julia_set qjs(force_cpu);
	cout << qjs.get_status_string() << '\n' << endl;


	// Initialize quaterion Julia set parameters.
	if(false == qjs.load_configuration_from_file(argv[1]))
	{
		cout << "Error reading " << argv[1] << " -- try using the following sample configuration file:" << endl;
		cout << "100     // Grid resolution (an unsigned integer)" << endl;
		cout << "8       // Vertex refinement steps (an unsigned integer)" << endl;
		cout << "0.001   // Shell thickness (a real number [0, 1]) -- Use 0 to make solid object" << endl;
		cout << "-1.5    // Grid minimum extent (a real number)" << endl;
		cout << "1.5     // Grid maximum extent (a real number)" << endl;
		cout << "8       // Maximum iterations (an unsigned integer)" << endl;
		cout << "4.0     // Threshold (a real number)" << endl;
		cout << "0.0     // Z.w (a real number)" << endl;
		cout << "0.3     // C.x (a real number)" << endl;
		cout << "0.5     // C.y (a real number)" << endl;
		cout << "0.4     // C.z (a real number)" << endl;
		cout << "0.2     // C.w (a real number)" << endl;
		cout << "Z = sin(Z) + C * sin(Z)  // Iterative equation" << endl;
		cout << "addblock, 0.93, 0.97, 0.01, 0.15, 0.2,  0.8 // Add a block, x start, x end, ..." << endl; 
		cout << "addblock, 0.03, 0.07, 0.01, 0.15, 0.2,  0.8" << endl;
		cout << "subblock, 0.95, 1,    0.3,  0.33, 0.3,  0.33 // Subtract a block: ..." << endl;
		cout << "subblock, 0,    0.05, 0.67, 0.7,  0.67, 0.7" << endl;

		return 1;
	}
	else
	{
		cout << "Configuration:" << endl;
		cout << "==========================================================" << endl;
		cout << "Grid resolution: " << qjs.get_res() << endl;
		cout << "Vertex refinement steps: " << qjs.get_vertex_refinement_steps() << endl;
		cout << "Shell thickness: " << qjs.get_shell_thickness() << endl;
		cout << "Grid minimum extent: " << qjs.get_grid_min() << endl;
		cout << "Grid maximum extent: " << qjs.get_grid_max() << endl;
		cout << "Maximum iterations: " << qjs.get_max_iterations() << endl;
		cout << "Threshold: " << qjs.get_threshold() << endl;
		cout << "Z.w: " << qjs.get_z_w() << endl;
		cout << "C.x: " << qjs.get_C_x() << endl;
		cout << "C.y: " << qjs.get_C_y() << endl;
		cout << "C.z: " << qjs.get_C_z() << endl;
		cout << "C.w: " << qjs.get_C_w() << endl;
		cout << "Equation: " << qjs.get_equation_text() << endl;
		cout << "Add / subtract blocks:\n" << qjs.get_blocks_string() << endl;
		cout << "==========================================================\n" << endl;
	}


	// Produce the isosurface.
	try
	{
		if(false == qjs.generate_and_write_isosurface_to_binary_stl_file(argv[2]))
		{
			cout << "Error: " << qjs.get_status_string() << endl;
			return 2;
		}
	}
	catch(exception &e)
	{
		if(typeid(e) == typeid(bad_alloc))
			cout << "Error: Not enough RAM available. Aborting." << endl;
		else
			cout << "Unexpected error: " << e.what() << ". Aborting." << endl;
	}

	return 0;
}

bool parse_args(int argc, char **argv, bool &force_cpu)
{
	// Assume that the arguments are bad until we know otherwise.
	bool successful_parse = false;

	// Use GPU mode by default.
	force_cpu = false;

	if(3 == argc)
	{
		// OK -- we've got an input file name and an output file name.
		successful_parse = true;
	}
	else if(4 == argc)
	{
		string last_arg = lower_string(argv[3]);
		
		// OK -- we've got an input file name, an output file name, and a valid 'force cpu' argument.
		if(last_arg == "-cpu" || last_arg == "/cpu" || last_arg == "cpu")
		{
			force_cpu = true;
			successful_parse = true;
		}
	}

	return successful_parse;
}
