// Source code by Shawn Halayka
// Source code is in the public domain


#include "quaternion_julia_set.h"


quaternion_julia_set::quaternion_julia_set(const bool src_force_cpu)
{
	force_cpu = src_force_cpu;

	res = 100;
	vertex_refinement_steps = 0;
	shell_thickness = 0;
	grid_max = 1.5;
	grid_min = -grid_min;
	z_w = 0;
	C.x = 0.3f;
	C.y = 0.5f;
	C.z = 0.4f;
	C.w = 0.2f;
	max_iterations = 8;
	threshold = 0;
	equation_text = "";

	step_size = 0;

	// This can only be set to true once the equation has been successfully set up.
	parameters_configured = false;

	if(true == force_cpu)
	{
		status_string = "Forcing CPU-only mode.";
		opengl_init_ok = false;
	}
	else
	{
		// Initialize the OpenGL context using the GLUT helper library.
		// Fake the command-line argument(s) that GLUT expects.
		int argc = 1; char **argv = 0; argv = new char*; argv[0] = new char[6];
		argv[0][0] = 'w'; argv[0][1] = 'h'; argv[0][2] = 'y'; argv[0][3] = '?'; argv[0][4] = '!'; argv[0][5] = '\0';

		glutInit(&argc, argv);

		delete [] argv[0]; delete argv;

		glutInitDisplayMode(GLUT_RGBA);
		glut_window_handle = glutCreateWindow("");

		// Make sure that the graphics drivers are capable of at least some basic OpenGL functionality,
		// so that we can use a fragment shader to perform render-to-texture with rectangular non-power-of-two textures.
		if(!(GLEW_OK == glewInit() && 
			 GLEW_VERSION_2_0 && 
			 GLEW_ARB_framebuffer_object &&
			 GLEW_ARB_texture_rectangle && 
			 GLEW_ARB_texture_non_power_of_two))
		{
			status_string += "OpenGL 2.0 initialization failure -- forcing CPU-only mode. Note: your graphics card / drivers do not support OpenGL 2.0, or some basic extensions related to render-to-texture with rectangular non-power-of-two textures.";
			opengl_init_ok = false;
		}
		else
		{
			status_string = "OpenGL 2.0 initialization successful.";
			opengl_init_ok = true;
		}
	}
}

quaternion_julia_set::~quaternion_julia_set(void)
{
	if(false == force_cpu)
		glutDestroyWindow(glut_window_handle);
}

bool quaternion_julia_set::load_configuration_from_file(const char *file_name)
{
	addsub_blocks.clear();

	vector<string> tokens;
	parameters_configured = false;

	ifstream config_file(file_name);

	if(config_file.fail())
		return false;

	string line;
	istringstream iss;

	// Get grid resolution.
	getline(config_file, line);
	if("" == line) return false;
	tokens = stl_str_tok("//", line);
	line = trim_whitespace_string(tokens[0]);
	if("" == line) return false;

	iss.str(line);
	iss	>> res;

	if(res < 1 || res > 100000)
		res = 100;

	// Get vertex refinement steps.
	getline(config_file, line);
	if("" == line) return false;
	tokens = stl_str_tok("//", line);
	line = trim_whitespace_string(tokens[0]);
	if("" == line) return false;

	iss.clear();
	iss.str(line);
	iss	>> vertex_refinement_steps;

	if(vertex_refinement_steps < 0 || vertex_refinement_steps > 1000)
		vertex_refinement_steps = 0;

	// Get shell thickness.
	getline(config_file, line);
	if("" == line) return false;
	tokens = stl_str_tok("//", line);
	line = trim_whitespace_string(tokens[0]);
	if("" == line) return false;

	iss.clear();
	iss.str(line);
	iss	>> shell_thickness;

	if(shell_thickness < 0)
		shell_thickness = 0;
	else if(shell_thickness > 1)
		shell_thickness = 1;

	// Get grid extent.
	getline(config_file, line);
	if("" == line) return false;
	tokens = stl_str_tok("//", line);
	line = trim_whitespace_string(tokens[0]);
	if("" == line) return false;

	iss.clear();
	iss.str(line);
	iss	>> grid_min;

	getline(config_file, line);
	if("" == line) return false;
	tokens = stl_str_tok("//", line);
	line = trim_whitespace_string(tokens[0]);
	if("" == line) return false;

	iss.clear();
	iss.str(line);
	iss	>> grid_max;

	if(grid_min == grid_max)
	{
		grid_min = -1.5;
		grid_max = 1.5;
	}
	else if(grid_min > grid_max)
	{
		float min = grid_max;
		float max = grid_min;

		grid_min = min;
		grid_max = max;
	}

	// Set step size now that we have res and grid extent parameters
	step_size = (grid_max - grid_min) / (res - 1);

	// Get max iterations.
	getline(config_file, line);
	if("" == line) return false;
	tokens = stl_str_tok("//", line);
	line = trim_whitespace_string(tokens[0]);
	if("" == line) return false;

	iss.clear();
	iss.str(line);
	iss	>> max_iterations;

	// Get threshold.
	getline(config_file, line);
	if("" == line) return false;
	tokens = stl_str_tok("//", line);
	line = trim_whitespace_string(tokens[0]);
	if("" == line) return false;

	iss.clear();
	iss.str(line);
	iss	>> threshold;

	// Get Z.w.
	getline(config_file, line);
	if("" == line) return false;
	tokens = stl_str_tok("//", line);
	line = trim_whitespace_string(tokens[0]);
	if("" == line) return false;

	iss.clear();
	iss.str(line);
	iss	>> z_w;

	// Get C.
	getline(config_file, line);
	if("" == line) return false;
	tokens = stl_str_tok("//", line);
	line = trim_whitespace_string(tokens[0]);
	if("" == line) return false;

	iss.clear();
	iss.str(line);
	iss	>> C.x;

	getline(config_file, line);
	if("" == line) return false;
	tokens = stl_str_tok("//", line);
	line = trim_whitespace_string(tokens[0]);
	if("" == line) return false;

	iss.clear();
	iss.str(line);
	iss	>> C.y;

	getline(config_file, line);
	if("" == line) return false;
	tokens = stl_str_tok("//", line);
	line = trim_whitespace_string(tokens[0]);
	if("" == line) return false;

	iss.clear();
	iss.str(line);
	iss	>> C.z;

	getline(config_file, line);
	if("" == line) return false;
	tokens = stl_str_tok("//", line);
	line = trim_whitespace_string(tokens[0]);
	if("" == line) return false;

	iss.clear();
	iss.str(line);
	iss	>> C.w;

	// Get equation text.
	getline(config_file, line);
	if("" == line) return false;
	tokens = stl_str_tok("//", line);
	equation_text = tokens[0];

	string error_string;

	if(false == setup_equation_text(equation_text, error_string))
	{
		status_string = "Error parsing formula -- " + error_string;
		return false;
	}


	// To do: change stlstrtok to keep adding to current string until last components of current string are the token
	// then chop the token off of the end of the string and put it into the vector -- this way when a line begins with a 
	// comment, then the first string in the vector will be blank (add/sub bug exists right now when the line begins with
	// a comment. This way I can check to see if the first in the vector is blank on an addsub line, and know to ignore it --
	// right now it gives the remainder of the line after the comment as the first in the vector, so addsub lines cannot be
	// commented out.


	// The remaining lines should contain additive or subtractive block statements.
	while(getline(config_file, line))
	{
		line = trim_whitespace_string(line);

		if("" == line)
			continue;

		tokens = stl_str_tok("//", line);
		line = tokens[0];

		tokens = stl_str_tok(",", lower_string(line));

		if(7 != tokens.size())
		{
			status_string = "addblock/subblock format error";
			return false;
		}

		addsub_block b;

		if("addblock" == tokens[0])
		{
			b.additive = true;
		}
		else if("subblock" == tokens[0])
		{
			b.additive = false;
		}
		else
		{
			status_string = "Unrecognized block token: " + tokens[0];
			return false;
		}

		iss.clear();
		iss.str(tokens[1]);
		iss	>> b.start_x;

		iss.clear();
		iss.str(tokens[2]);
		iss	>> b.end_x;

		iss.clear();
		iss.str(tokens[3]);
		iss	>> b.start_y;

		iss.clear();
		iss.str(tokens[4]);
		iss	>> b.end_y;

		iss.clear();
		iss.str(tokens[5]);
		iss	>> b.start_z;

		iss.clear();
		iss.str(tokens[6]);
		iss	>> b.end_z;

		if(0 > b.start_x)
			b.start_x = 0;
		else if(1 < b.start_x)
			b.start_x = 1;

		if(0 > b.end_x)
			b.end_x = 0;
		else if(1 < b.end_x)
			b.end_x = 1;

		if(0 > b.start_y)
			b.start_y = 0;
		else if(1 < b.start_y)
			b.start_y = 1;

		if(0 > b.end_y)
			b.end_y = 0;
		else if(1 < b.end_y)
			b.end_y = 1;

		if(0 > b.start_z)
			b.start_z = 0;
		else if(1 < b.start_z)
			b.start_z = 1;

		if(0 > b.end_z)
			b.end_z = 0;
		else if(1 < b.end_z)
			b.end_z = 1;

		if(b.start_x > b.end_x)
		{
			float temp = b.start_x;
			b.start_x = b.end_x;
			b.end_x = temp;
		}

		if(b.start_y > b.end_y)
		{
			float temp = b.start_y;
			b.start_y = b.end_y;
			b.end_y = temp;
		}

		if(b.start_z > b.end_z)
		{
			float temp = b.start_z;
			b.start_z = b.end_z;
			b.end_z = temp;
		}

		addsub_blocks.push_back(b);
	}

	parameters_configured = true;
	status_string = "OK";

	return true;
}

bool quaternion_julia_set::generate_and_write_isosurface_to_binary_stl_file(const char *file_name)
{
	if(false == parameters_configured)
	{
		status_string = "The quaternion Julia set parameters have not yet been configured.";
		return false;
	}

	time_t start_time;
	time(&start_time);

	vector<bool> fractal_set;

	if(false == generate_fractal_set(fractal_set))
		return false;

	cout << "Elapsed time so far: " << time(0) - start_time << " seconds.\n" << endl;

	// Hollow out the set if desired.
	if(0 < shell_thickness)
	{
		cout << "Finding surface" << endl;

		vector<bool> surface;
		get_surface_set(fractal_set, surface);

		cout << "Elapsed time so far: " << time(0) - start_time << " seconds.\n" << endl;

		// Get shell thickness in terms of integer units with respect to res -- use rounding.
		size_t shell_thickness_int = static_cast<size_t>(floorf(0.5f + static_cast<float>(res) * shell_thickness));

		// Make shell thickness at least 2 -- if thickness is 1, then there is trouble keeping
		// track of what's part of the initial surface and what's part of the interior surface.
		if(2 > shell_thickness_int)
			shell_thickness_int = 2;

		// Shell is already 1 unit thick; thicken further if needed.
		for(size_t i = 1; i < shell_thickness_int; i++)
		{
			cout << "Thickening shell (pass " << i << " of " << shell_thickness_int - 1 << ')' << endl;
			thicken_shell(fractal_set, surface);
		}

		// Assign the shell to the set.
		surface.swap(fractal_set);

		cout << "Elapsed time so far: " << time(0) - start_time << " seconds.\n" << endl;
	}

	// Add / subtract blocks from the set.
	if(0 < addsub_blocks.size())
	{
		for(size_t i = 0; i < addsub_blocks.size(); i++)
		{
			if(true == addsub_blocks[i].additive)
			{
				cout << "Adding block " << i + 1 << " of " << addsub_blocks.size() << endl;
				add_to_set(fractal_set, addsub_blocks[i]);
			}
			else
			{
				cout << "Subtracting block " << i + 1 << " of " << addsub_blocks.size() << endl;
				subtract_from_set(fractal_set, addsub_blocks[i]);
			}
		}

		cout << "Elapsed time so far: " << time(0) - start_time << " seconds.\n" << endl;
	}

	cout << "Converting set to isosurface" << endl;
	indexed_mesh m;

	if(false == tesselate_set(fractal_set, m))
		return false;

	cout << "Elapsed time so far: " << time(0) - start_time << " seconds.\n" << endl;

	if(0 == m.get_triangle_count())
	{
		cout << "No triangles generated -- aborting early." << endl;
		status_string = "OK";
		return true;
	}

	cout << "Analyzing mesh for problem edges (cracks, holes) and degenerate triangles" << endl;

	size_t problem_edge_count = m.get_problem_edge_count();
	size_t degenerate_triangle_count = m.get_degenerate_triangle_count();

	if(0 == problem_edge_count && 0 == degenerate_triangle_count)
	{
		cout << "No problems detected." << endl;
	}
	else
	{
		cout << problem_edge_count << " problem edges found" << endl;
		cout << degenerate_triangle_count << " degenerate triangles found" << endl;
		cout << "Did you go a little too hardcore on the vertex refinement steps / grid resolution options?" << endl;
		cout << "If not, try using netfabb or MeshLab to fix the mesh." << endl;
	}

	cout << "Elapsed time so far: " << time(0) - start_time << " seconds.\n" << endl;

	cout << "Mesh information:" << endl;
	cout << "Mesh x extent:     " << m.get_x_extent() << " units" << endl;
	cout << "Mesh y extent:     " << m.get_y_extent() << " units" << endl;
	cout << "Mesh z extent:     " << m.get_z_extent() << " units" << endl;
	cout << "Mesh surface area: " << m.get_area()     << " units^2" << endl;
	cout << "Mesh volume:       " << m.get_volume()   << " units^3" << endl;
	cout << "File name:         " << file_name << endl;
	cout << "Triangles:         " << m.get_triangle_count() << endl;
	cout << "Vertices:          " << m.get_triangle_count()*3 << " (of which " << m.get_vertex_count() << " are unique)" << endl;

	if(false == m.save_to_binary_stereo_lithography_file(file_name))
	{
		status_string = "Could not save to binary Stereo Lithography file: ";
		status_string += file_name;
		return false;
	}

	cout << "Total elapsed time: " << time(0) - start_time << " seconds." << endl;

	status_string = "OK";
	return true;
}

string quaternion_julia_set::get_blocks_string(void)
{
	if(0 == addsub_blocks.size())
		return "None";

	ostringstream oss;

	for(size_t i = 0; i < addsub_blocks.size(); i++)
	{
		if(true == addsub_blocks[i].additive)
			oss << "addblock, ";
		else
			oss << "subblock, ";

		oss << addsub_blocks[i].start_x << ", ";
		oss << addsub_blocks[i].end_x << ", ";
		oss << addsub_blocks[i].start_y << ", ";
		oss << addsub_blocks[i].end_y << ", ";
		oss << addsub_blocks[i].start_z << ", ";
		oss << addsub_blocks[i].end_z;
		
		if(i < addsub_blocks.size() - 1)
			oss << endl;
	}

	return oss.str();
}

bool quaternion_julia_set::setup_equation_text(const string &src_equation_text, string &error_string)
{
	if(eqparser.setup(src_equation_text, error_string, C))
		return true;
	else
		return false;
}

bool quaternion_julia_set::initialize_fragment_shader(const string &fragment_shader_code, GLint &shader)
{
	shader = 0;

	if(true == opengl_init_ok)
	{
		// Compile shader.
		const char *cch = 0;
		GLint status = GL_FALSE;
		GLint frag = glCreateShader(GL_FRAGMENT_SHADER);

		glShaderSource(frag, 1, &(cch = fragment_shader_code.c_str()), 0);
		glCompileShader(frag);
		glGetShaderiv(frag, GL_COMPILE_STATUS, &status);

		if(GL_FALSE == status)
		{
			status_string = "Fragment shader compile error.\n";
			vector<GLchar> buf(4096, '\0');
			glGetShaderInfoLog(frag, 4095, 0, &buf[0]);

			for(size_t i = 0; i < buf.size(); i++)
				if('\0' != buf[i])
					status_string += buf[i];

			status_string += '\n';

			glDeleteShader(frag);
			return false;
		}

		// Link to get final shader.
		shader = glCreateProgram();
		glAttachShader(shader, frag);
		glLinkProgram(shader);
		glGetProgramiv(shader, GL_LINK_STATUS, &status);

		if(GL_FALSE == status)
		{
			status_string = "Program link error.\n";
			vector<GLchar> buf(4096, '\0');
			glGetShaderInfoLog(shader, 4095, 0, &buf[0]);

			for(size_t i = 0; i < buf.size(); i++)
				if('\0' != buf[i])
					status_string += buf[i];

			status_string += '\n';

			glDetachShader(shader, frag);
			glDeleteShader(frag);
			return false;
		}

		// Cleanup.
		glDetachShader(shader, frag);
		glDeleteShader(frag);

		return true;
	}

	return false;
}

bool quaternion_julia_set::generate_fractal_set(vector<bool> &fractal_set)
{
	fractal_set.resize(res*res*res);

	GLint shader_handle = 0;
	GLuint fbo_handle = 0;
	GLuint tex_fbo_handle = 0;
	GLuint tex_in_handle = 0;
	GLuint tex_out_handle = 0;
	const GLint tex_in_internal_format = GL_RGB32F_ARB;
	const GLint tex_in_format = GL_RGB;
	const GLint tex_out_internal_format = GL_RGB32F_ARB; // Note: We only need one channel, but alpha and luminance aren't cross-platform compatible ...
	const GLint tex_out_format = GL_RGB;
	const GLint var_type = GL_FLOAT;
	const GLint tex_size_x = res;
	const GLint tex_size_y = res;
	GLint max_tex_size = 0;

	if(true == opengl_init_ok)
	{
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_tex_size);

		if(max_tex_size < tex_size_x || max_tex_size < tex_size_y)
		{
			ostringstream oss;
			oss << "GPU max texture size (" << max_tex_size << ") is not large enough.";
			status_string = oss.str();

			return false;
		}

		ofstream of("main_shader.txt");
		of << eqparser.emit_fragment_shader_code() << "\n// Equation text: " << equation_text << endl;
		of.close();

		// Load and compile shader.
		if(false == initialize_fragment_shader(eqparser.emit_fragment_shader_code(), shader_handle))
			return false;

		// Allocate OpenGL objects.
		glGenTextures(1, &tex_in_handle);
		glBindTexture(GL_TEXTURE_2D, tex_in_handle);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glGenTextures(1, &tex_out_handle);
		glBindTexture(GL_TEXTURE_2D, tex_out_handle);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		// Initialize FBO and FBO output texture.
		// These are used to "draw" to a texture instead of to the screen (render-to-texture).
		glGenFramebuffersEXT(1, &fbo_handle);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_handle);
		glGenTextures(1, &tex_fbo_handle);
		glBindTexture(GL_TEXTURE_2D, tex_fbo_handle);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, tex_out_internal_format, tex_size_x, tex_size_y, 0, tex_out_format, var_type, 0);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, tex_fbo_handle, 0);

		// Initial setup for "drawing".
		glUseProgram(shader_handle);
		glUniform1i(glGetUniformLocation(shader_handle, "z_xyz"), 0); // Use texture 0.
		glUniform1f(glGetUniformLocation(shader_handle, "z_w"), z_w);
		glUniform4f(glGetUniformLocation(shader_handle, "c"), C.x, C.y, C.z, C.w);
		glUniform1i(glGetUniformLocation(shader_handle, "max_iterations"), max_iterations);
		glUniform1f(glGetUniformLocation(shader_handle, "threshold"), threshold);
	}

	vector<float> input(res*res*3, 0); // one float per channel, three channels (RGB)
	vector<float> output(res*res*3, 0); // one float per channel, three channels (RGB). Note: We only need one channel, but alpha and luminance aren't cross-platform compatible ...

	for(size_t z = 0; z < res; z++)
	{
		cout << "Calculating xy-plane " << z + 1 << " of " << res << endl;

		// Set up input.
		for(size_t x = 0; x < res; x++)
		{
			for(size_t y = 0; y < res; y++)
			{
				size_t input_index = 3*(x*res + y);
				input[input_index + 0] = grid_min + x*step_size;
				input[input_index + 1] = grid_min + y*step_size;
				input[input_index + 2] = grid_min + z*step_size;
			}
		}

		if(true == opengl_init_ok)
		{
			// Write to GPU memory.
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex_in_handle);
			glTexImage2D(GL_TEXTURE_2D, 0, tex_in_internal_format, tex_size_x, tex_size_y, 0, tex_in_format, var_type, &input[0]);

			// Calculate by "drawing".
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(0, 1, 0, 1, 0, 1);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glViewport(0, 0, tex_size_x, tex_size_y);

			glBegin(GL_QUADS);
				glTexCoord2f(0, 1);	glVertex2f(0, 1);
				glTexCoord2f(0, 0);	glVertex2f(0, 0);
				glTexCoord2f(1, 0);	glVertex2f(1, 0);
				glTexCoord2f(1, 1);	glVertex2f(1, 1);
			glEnd();

			// Read from GPU memory.
			glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
			glReadPixels(0, 0, tex_size_x, tex_size_y, tex_out_format, var_type, &output[0]);
		}
		else
		{
			for(size_t x = 0; x < res; x++)
			{
				for(size_t y = 0; y < res; y++)
				{
					size_t input_index = 3*(x*res + y);
					size_t output_index = 3*(x*res + y);

					output[output_index] = eqparser.iterate(quaternion(input[input_index + 0], input[input_index + 1], input[input_index + 2], z_w), max_iterations, threshold);
				}
			}
		}

		// Convert to bools.
		for(size_t x = 0; x < res; x++)
		{
			for(size_t y = 0; y < res; y++)
			{
				size_t set_index = x*res*res + y*res + z;
				size_t output_index = 3*(x*res + y);

				// If in set.
				if(threshold > output[output_index])
					fractal_set[set_index] = true;
				else
					fractal_set[set_index] = false;
			}
		}
	} // End: for(size_t z = 0, ...

	// Make border.
	for(size_t x = 0; x < res; x++)
	{
		for(size_t y = 0; y < res; y++)
		{
			for(size_t z = 0; z < res; z++)
			{
				size_t set_index = x*res*res + y*res + z;

				if( x == 0 || x == res - 1 || 
					y == 0 || y == res - 1 ||
					z == 0 || z == res - 1 )
				{
					fractal_set[set_index] = false;
				}
			}
		}
	}

	if(true == opengl_init_ok)
	{
		// Cleanup OpenGL objects.
		glDeleteTextures(1, &tex_in_handle);
		glDeleteTextures(1, &tex_out_handle);
		glDeleteTextures(1, &tex_fbo_handle);
		glDeleteFramebuffersEXT(1, &fbo_handle);
		glUseProgram(0);
		glDeleteProgram(shader_handle);
	}

	return true;
}

void quaternion_julia_set::get_surface_set(const vector<bool> &fractal_set, vector<bool> &surface)
{
	if(0 == fractal_set.size())
		return;

	surface.resize(fractal_set.size(), false);

	// Skip the first and last of each dimension, since we know those are not in the set by default
	// (they make up the border).
	for(size_t x = 1; x < res - 1; x++)
	{
		for(size_t y = 1; y < res - 1; y++)
		{
			for(size_t z = 1; z < res - 1; z++)
			{
				size_t set_index = x*res*res + y*res + z;

				// If not in set, it definitely won't be part of the surface set.
				if(false == fractal_set[set_index])
					continue;

				// Search for any neighbours not in set -- if one is found, then this is part of the surface set.
				for(signed char i = -1; i <= 1; i++)
				{
					for(signed char j = -1; j <= 1; j++)
					{
						for(signed char k = -1; k <= 1; k++)
						{
							size_t neighbour_index = (x + i)*res*res + (y + j)*res + (z + k);

							if(false == fractal_set[neighbour_index])
							{
								surface[set_index] = true;
								i = j = k = 2; // No need to look at any remaining neighbours.
							}
						}
					}
				} // End: for(signed char i = -1; ...
			}
		}
	} // End: for(size_t x = 1; ...
}

void quaternion_julia_set::thicken_shell(const vector<bool> &fractal_set, vector<bool> &shell)
{
	vector<bool> initial_shell = shell;

	// Skip the first and last of each dimension, since we know those are not in the set by default
	// (they make up the border).
	for(size_t x = 1; x < res - 1; x++)
	{
		for(size_t y = 1; y < res - 1; y++)
		{
			for(size_t z = 1; z < res - 1; z++)
			{
				size_t set_index = x*res*res + y*res + z;

				// If already in shell or not in the set, skip it.
				if(true == initial_shell[set_index] || false == fractal_set[set_index])
					continue;

				// Search for any neighbours in shell -- if one is found, then this is part of the shell.
				for(signed char i = -1; i <= 1; i++)
				{
					for(signed char j = -1; j <= 1; j++)
					{
						for(signed char k = -1; k <= 1; k++)
						{
							size_t neighbour_index = (x + i)*res*res + (y + j)*res + (z + k);

							if(true == initial_shell[neighbour_index])
							{
								shell[set_index] = true;
								i = j = k = 2; // No need to look at any remaining neighbours.
							}
						}
					}
				} // End: for(signed char i = -1; ...
			}
		}
	}
}

void quaternion_julia_set::add_to_set(vector<bool> &fractal_set, const addsub_block &b)
{
	size_t x0 = static_cast<size_t>(floorf(0.5f + static_cast<float>(res - 1) * b.start_x));
	size_t x1 = static_cast<size_t>(floorf(0.5f + static_cast<float>(res - 1) * b.end_x));
	size_t y0 = static_cast<size_t>(floorf(0.5f + static_cast<float>(res - 1) * b.start_y));
	size_t y1 = static_cast<size_t>(floorf(0.5f + static_cast<float>(res - 1) * b.end_y));
	size_t z0 = static_cast<size_t>(floorf(0.5f + static_cast<float>(res - 1) * b.start_z));
	size_t z1 = static_cast<size_t>(floorf(0.5f + static_cast<float>(res - 1) * b.end_z));

	// To do: ensure that add blocks are at least 2 integer units in size on each extent (take blank border into account, except where res == 1, 2, 3),
	// so that they do not collapse into sheets for small resolutions after vertex refinement

	for(size_t x = x0; x <= x1; x++)
	{
		for(size_t y = y0; y <= y1; y++)
		{
			for(size_t z = z0; z <= z1; z++)
			{
				// Make sure not to fill the border.
				if( x == 0 || x == res - 1 ||
					y == 0 || y == res - 1 ||
					y == 0 || z == res - 1 )
					continue;

				size_t set_index = x*res*res + y*res + z;
				fractal_set[set_index] = true;
			}
		}
	}
}

void quaternion_julia_set::subtract_from_set(vector<bool> &fractal_set, const addsub_block &b)
{
	size_t x0 = static_cast<size_t>(floorf(0.5f + static_cast<float>(res - 1) * b.start_x));
	size_t x1 = static_cast<size_t>(floorf(0.5f + static_cast<float>(res - 1) * b.end_x));
	size_t y0 = static_cast<size_t>(floorf(0.5f + static_cast<float>(res - 1) * b.start_y));
	size_t y1 = static_cast<size_t>(floorf(0.5f + static_cast<float>(res - 1) * b.end_y));
	size_t z0 = static_cast<size_t>(floorf(0.5f + static_cast<float>(res - 1) * b.start_z));
	size_t z1 = static_cast<size_t>(floorf(0.5f + static_cast<float>(res - 1) * b.end_z));

	for(size_t x = x0; x <= x1; x++)
	{
		for(size_t y = y0; y <= y1; y++)
		{
			for(size_t z = z0; z <= z1; z++)
			{
				size_t set_index = x*res*res + y*res + z;
				fractal_set[set_index] = false;
			}
		}
	}
}

void quaternion_julia_set::init_grid_cube(mc_grid_cube &cube, const size_t cube_x, const size_t cube_y, const size_t cube_z, const vector<bool> &fractal_set)
{
	// Note: default notation for MC -- small values (ie. false) are inside of the surface, large values (ie. true) are outside of the surface.
	// This is why we must negate before assigning to cube.value[...].

	size_t x_offset = 0;
	size_t y_offset = 0;
	size_t z_offset = 0;

	// Setup vertex 0
	x_offset = 0;
	y_offset = 0;
	z_offset = 0;
	cube.vertex[0].x = grid_min + ((cube_x + x_offset) * step_size);
	cube.vertex[0].y = grid_min + ((cube_y + y_offset) * step_size);
	cube.vertex[0].z = grid_min + ((cube_z + z_offset) * step_size);
	cube.value[0] = !fractal_set[(cube_x + x_offset)*res*res + (cube_y + y_offset)*res + (cube_z + z_offset)];

	// Setup vertex 1
	x_offset = 1;
	y_offset = 0;
	z_offset = 0;
	cube.vertex[1].x = grid_min + ((cube_x + x_offset) * step_size);
	cube.vertex[1].y = grid_min + ((cube_y + y_offset) * step_size);
	cube.vertex[1].z = grid_min + ((cube_z + z_offset) * step_size);
	cube.value[1] = !fractal_set[(cube_x + x_offset)*res*res + (cube_y + y_offset)*res + (cube_z + z_offset)];

	// Setup vertex 2
	x_offset = 1;
	y_offset = 0;
	z_offset = 1;
	cube.vertex[2].x = grid_min + ((cube_x + x_offset) * step_size);
	cube.vertex[2].y = grid_min + ((cube_y + y_offset) * step_size);
	cube.vertex[2].z = grid_min + ((cube_z + z_offset) * step_size);
	cube.value[2] = !fractal_set[(cube_x + x_offset)*res*res + (cube_y + y_offset)*res + (cube_z + z_offset)];

	// Setup vertex 3
	x_offset = 0; 
	y_offset = 0;
	z_offset = 1;
	cube.vertex[3].x = grid_min + ((cube_x + x_offset) * step_size);
	cube.vertex[3].y = grid_min + ((cube_y + y_offset) * step_size);
	cube.vertex[3].z = grid_min + ((cube_z + z_offset) * step_size);
	cube.value[3] = !fractal_set[(cube_x + x_offset)*res*res + (cube_y + y_offset)*res + (cube_z + z_offset)];

	// Setup vertex 4
	x_offset = 0;
	y_offset = 1;
	z_offset = 0;
	cube.vertex[4].x = grid_min + ((cube_x + x_offset) * step_size);
	cube.vertex[4].y = grid_min + ((cube_y + y_offset) * step_size);
	cube.vertex[4].z = grid_min + ((cube_z + z_offset) * step_size);
	cube.value[4] = !fractal_set[(cube_x + x_offset)*res*res + (cube_y + y_offset)*res + (cube_z + z_offset)];

	// Setup vertex 5
	x_offset = 1;
	y_offset = 1;
	z_offset = 0;
	cube.vertex[5].x = grid_min + ((cube_x + x_offset) * step_size);
	cube.vertex[5].y = grid_min + ((cube_y + y_offset) * step_size);
	cube.vertex[5].z = grid_min + ((cube_z + z_offset) * step_size);
	cube.value[5] = !fractal_set[(cube_x + x_offset)*res*res + (cube_y + y_offset)*res + (cube_z + z_offset)];

	// Setup vertex 6
	x_offset = 1;
	y_offset = 1;
	z_offset = 1;
	cube.vertex[6].x = grid_min + ((cube_x + x_offset) * step_size);
	cube.vertex[6].y = grid_min + ((cube_y + y_offset) * step_size);
	cube.vertex[6].z = grid_min + ((cube_z + z_offset) * step_size);
	cube.value[6] = !fractal_set[(cube_x + x_offset)*res*res + (cube_y + y_offset)*res + (cube_z + z_offset)];

	// Setup vertex 7
	x_offset = 0;
	y_offset = 1;
	z_offset = 1;
	cube.vertex[7].x = grid_min + ((cube_x + x_offset) * step_size);
	cube.vertex[7].y = grid_min + ((cube_y + y_offset) * step_size);
	cube.vertex[7].z = grid_min + ((cube_z + z_offset) * step_size);
	cube.value[7] = !fractal_set[(cube_x + x_offset)*res*res + (cube_y + y_offset)*res + (cube_z + z_offset)];
}

bool quaternion_julia_set::tesselate_set(const vector<bool> &fractal_set, indexed_mesh &m)
{
	GLint shader_handle = 0;
	GLuint fbo_handle = 0;
	GLuint tex_fbo_handle = 0;
	GLuint tex_in0_handle = 0;
	GLuint tex_in1_handle = 0;
	GLuint tex_out_handle = 0;
	const GLint tex_in_internal_format = GL_RGBA32F_ARB;
	const GLint tex_in_format = GL_RGBA;
	const GLint tex_out_internal_format = GL_RGBA32F_ARB; // Note: We only need three channels, but using RGB on AMD 6310 causes some small errors related to bit corruption ...
	const GLint tex_out_format = GL_RGBA;
	const GLint var_type = GL_FLOAT;

	if(true == opengl_init_ok)
	{
		ofstream of("vertex_interp_shader.txt");
		of << eqparser.emit_vertex_interp_fragment_shader_code() << "\n// Equation text: " << equation_text << endl;
		of.close();

		// Load and compile shader.
		if(false == initialize_fragment_shader(eqparser.emit_vertex_interp_fragment_shader_code(), shader_handle))
			return false;

		// Allocate OpenGL objects.
		glGenTextures(1, &tex_in0_handle);
		glBindTexture(GL_TEXTURE_1D, tex_in0_handle);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

		glGenTextures(1, &tex_in1_handle);
		glBindTexture(GL_TEXTURE_1D, tex_in1_handle);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

		glGenTextures(1, &tex_out_handle);
		glBindTexture(GL_TEXTURE_1D, tex_out_handle);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

		// Initialize FBO and FBO output texture.
		// These are used to "draw" to a texture instead of to the screen (render-to-texture).
		glGenFramebuffersEXT(1, &fbo_handle);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_handle);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

		glGenTextures(1, &tex_fbo_handle);
		glBindTexture(GL_TEXTURE_1D, tex_fbo_handle);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

		// Initial setup for "drawing".
		glUseProgram(shader_handle);
		glUniform1i(glGetUniformLocation(shader_handle, "input0"), 0); // Use texture 0.
		glUniform1i(glGetUniformLocation(shader_handle, "input1"), 1); // Use texture 1.
		glUniform1f(glGetUniformLocation(shader_handle, "z_w"), z_w);
		glUniform4f(glGetUniformLocation(shader_handle, "c"), C.x, C.y, C.z, C.w);
		glUniform1i(glGetUniformLocation(shader_handle, "max_iterations"), max_iterations);
		glUniform1f(glGetUniformLocation(shader_handle, "threshold"), threshold);
		glUniform1i(glGetUniformLocation(shader_handle, "vertex_refinement_steps"), vertex_refinement_steps);
	}

	m.init_triangle_insertion();

	for(size_t cube_z = 0; cube_z < res - 1; cube_z++)
	{
		cout << "Tesselating grid cube array " << cube_z + 1 << " of " << res - 1 << endl;

		// Get input for shader.
		// Contains four floats per vertex interpolation (3 for vertex position, 1 for value).
		// input0 contains the first vertex in each pair, input1 contains the second vertex in each pair.
		vector<float> input0, input1;

		for(size_t cube_x = 0; cube_x < res - 1; cube_x++)
		{
			for(size_t cube_y = 0; cube_y < res - 1; cube_y++)
			{
				mc_grid_cube cube;

				init_grid_cube(cube, cube_x, cube_y, cube_z, fractal_set);
				get_vertex_interp_input_from_grid_cube(cube, input0, input1);
			}
		}

		// If there were absolutely no vertex interps generated, then there will be absolutely no
		// triangles in this grid cube array, so just continue to the next grid cube array.
		if(0 == input0.size())
			continue;


		// Calculate output via shader.
		const size_t num_vertex_interps = input0.size()/4;
		
		// Contains three floats per vertex interpolation (3 for vertex position).
		vector<float> output(num_vertex_interps*4, 0);


		if(true == opengl_init_ok)
		{
			size_t num_vertex_interps_remaining = num_vertex_interps;

			GLint max_tex_size = 0;

			glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_tex_size);
			
			while(0 < num_vertex_interps_remaining)
			{
				size_t tex_size_x;

				if(num_vertex_interps_remaining > static_cast<size_t>(max_tex_size))
					tex_size_x = max_tex_size;
				else
					tex_size_x = num_vertex_interps_remaining;

				const size_t index = num_vertex_interps - num_vertex_interps_remaining;
				const size_t input_index = index*4;
				const size_t output_index = index*4;

				// Set the FBO size.
				glBindTexture(GL_TEXTURE_1D, tex_fbo_handle);
				glTexImage1D(GL_TEXTURE_1D, 0, tex_out_internal_format, tex_size_x, 0, tex_out_format, var_type, 0);
				glFramebufferTexture1DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_1D, tex_fbo_handle, 0);

				// Write to GPU memory.
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_1D, tex_in0_handle);
				glTexImage1D(GL_TEXTURE_1D, 0, tex_in_internal_format, tex_size_x, 0, tex_in_format, var_type, &input0[input_index]);

				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_1D, tex_in1_handle);
				glTexImage1D(GL_TEXTURE_1D, 0, tex_in_internal_format, tex_size_x, 0, tex_in_format, var_type, &input1[input_index]);


				// Calculate by "drawing".
				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				glOrtho(0, 1, 0, 1, 0, 1);
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();
				glViewport(0, 0, tex_size_x, 1);

				glBegin(GL_QUADS);
					glTexCoord2f(0, 1);	glVertex2f(0, 1);
					glTexCoord2f(0, 0);	glVertex2f(0, 0);
					glTexCoord2f(1, 0);	glVertex2f(1, 0);
					glTexCoord2f(1, 1);	glVertex2f(1, 1);
				glEnd();

				// Read from GPU memory.
				glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
				glReadPixels(0, 0, tex_size_x, 1, tex_out_format, var_type, &output[output_index]);

				num_vertex_interps_remaining -= tex_size_x;
			}
		}
		else
		{
			for(size_t i = 0; i < num_vertex_interps; i++)
			{
				size_t input_index = i*4;

				vertex_3 in0_vert;
				in0_vert.x = input0[input_index + 0];
				in0_vert.y = input0[input_index + 1];
				in0_vert.z = input0[input_index + 2];

				vertex_3 in1_vert;
				in1_vert.x = input1[input_index + 0];
				in1_vert.y = input1[input_index + 1];
				in1_vert.z = input1[input_index + 2];

				float in0_val = input0[input_index + 3];
				float in1_val = input1[input_index + 3];

				vertex_3 out_vert;
				out_vert = vertex_interp_float(in0_vert, in1_vert, in0_val, in1_val);

				size_t output_index = i*4;
				output[output_index + 0] = out_vert.x;
				output[output_index + 1] = out_vert.y;
				output[output_index + 2] = out_vert.z;
			}
		}


		// Tesselate output.
		input0.clear();
		input1.clear();

		size_t current_vertex_index = 0;

		for(size_t cube_x = 0; cube_x < res - 1; cube_x++)
		{
			for(size_t cube_y = 0; cube_y < res - 1; cube_y++)
			{
				mc_grid_cube cube;
				triangle temp_triangle_array[max_triangles_per_mc_cell];

				init_grid_cube(cube, cube_x, cube_y, cube_z, fractal_set);
				short unsigned int number_of_triangles_generated = get_triangles_from_grid_cube(cube, output, current_vertex_index, temp_triangle_array);

				for(short unsigned int i = 0; i < number_of_triangles_generated; i++)
					m.insert_triangle(temp_triangle_array[i]);
			}
		}
	}

	cout << "Generating mesh adjacency data" << endl;

	m.finalize_triangle_insertion();

	if(true == opengl_init_ok)
	{
		// Cleanup OpenGL objects.
		glDeleteTextures(1, &tex_in0_handle);
		glDeleteTextures(1, &tex_in1_handle);
		glDeleteTextures(1, &tex_out_handle);
		glDeleteTextures(1, &tex_fbo_handle);
		glDeleteFramebuffersEXT(1, &fbo_handle);
		glUseProgram(0);
		glDeleteProgram(shader_handle);
	}

	return true;
}

void quaternion_julia_set::get_vertex_interp_input_from_grid_cube(const mc_grid_cube &cube, vector<float> &input0, vector<float> &input1)
{
	short unsigned int case_index = 0;

	// Note: default notation for MC -- small values are inside of the surface, large values are outside of the surface.
	if(false == cube.value[0]) case_index |= 1;
	if(false == cube.value[1]) case_index |= 2;
	if(false == cube.value[2]) case_index |= 4;
	if(false == cube.value[3]) case_index |= 8;
	if(false == cube.value[4]) case_index |= 16;
	if(false == cube.value[5]) case_index |= 32;
	if(false == cube.value[6]) case_index |= 64;
	if(false == cube.value[7]) case_index |= 128;

	if(0 == mc_edge_table[case_index])
		return;

	if(mc_edge_table[case_index] & 1)
	{
		input0.push_back(cube.vertex[0].x);
		input0.push_back(cube.vertex[0].y);
		input0.push_back(cube.vertex[0].z);
		input0.push_back(static_cast<float>(cube.value[0]));

		input1.push_back(cube.vertex[1].x);
		input1.push_back(cube.vertex[1].y);
		input1.push_back(cube.vertex[1].z);
		input1.push_back(static_cast<float>(cube.value[1]));
	}

	if(mc_edge_table[case_index] & 2)
	{
		input0.push_back(cube.vertex[1].x);
		input0.push_back(cube.vertex[1].y);
		input0.push_back(cube.vertex[1].z);
		input0.push_back(static_cast<float>(cube.value[1]));

		input1.push_back(cube.vertex[2].x);
		input1.push_back(cube.vertex[2].y);
		input1.push_back(cube.vertex[2].z);
		input1.push_back(static_cast<float>(cube.value[2]));
	}

	if(mc_edge_table[case_index] & 4)
	{
		input0.push_back(cube.vertex[2].x);
		input0.push_back(cube.vertex[2].y);
		input0.push_back(cube.vertex[2].z);
		input0.push_back(static_cast<float>(cube.value[2]));

		input1.push_back(cube.vertex[3].x);
		input1.push_back(cube.vertex[3].y);
		input1.push_back(cube.vertex[3].z);
		input1.push_back(static_cast<float>(cube.value[3]));
	}

	if(mc_edge_table[case_index] & 8)
	{
		input0.push_back(cube.vertex[3].x);
		input0.push_back(cube.vertex[3].y);
		input0.push_back(cube.vertex[3].z);
		input0.push_back(static_cast<float>(cube.value[3]));

		input1.push_back(cube.vertex[0].x);
		input1.push_back(cube.vertex[0].y);
		input1.push_back(cube.vertex[0].z);
		input1.push_back(static_cast<float>(cube.value[0]));
	}

	if(mc_edge_table[case_index] & 16)
	{
		input0.push_back(cube.vertex[4].x);
		input0.push_back(cube.vertex[4].y);
		input0.push_back(cube.vertex[4].z);
		input0.push_back(static_cast<float>(cube.value[4]));

		input1.push_back(cube.vertex[5].x);
		input1.push_back(cube.vertex[5].y);
		input1.push_back(cube.vertex[5].z);
		input1.push_back(static_cast<float>(cube.value[5]));
	}

	if(mc_edge_table[case_index] & 32)
	{
		input0.push_back(cube.vertex[5].x);
		input0.push_back(cube.vertex[5].y);
		input0.push_back(cube.vertex[5].z);
		input0.push_back(static_cast<float>(cube.value[5]));

		input1.push_back(cube.vertex[6].x);
		input1.push_back(cube.vertex[6].y);
		input1.push_back(cube.vertex[6].z);
		input1.push_back(static_cast<float>(cube.value[6]));
	}

	if(mc_edge_table[case_index] & 64)
	{
		input0.push_back(cube.vertex[6].x);
		input0.push_back(cube.vertex[6].y);
		input0.push_back(cube.vertex[6].z);
		input0.push_back(static_cast<float>(cube.value[6]));

		input1.push_back(cube.vertex[7].x);
		input1.push_back(cube.vertex[7].y);
		input1.push_back(cube.vertex[7].z);
		input1.push_back(static_cast<float>(cube.value[7]));
	}

	if(mc_edge_table[case_index] & 128)
	{
		input0.push_back(cube.vertex[7].x);
		input0.push_back(cube.vertex[7].y);
		input0.push_back(cube.vertex[7].z);
		input0.push_back(static_cast<float>(cube.value[7]));

		input1.push_back(cube.vertex[4].x);
		input1.push_back(cube.vertex[4].y);
		input1.push_back(cube.vertex[4].z);
		input1.push_back(static_cast<float>(cube.value[4]));
	}

	if(mc_edge_table[case_index] & 256)
	{
		input0.push_back(cube.vertex[0].x);
		input0.push_back(cube.vertex[0].y);
		input0.push_back(cube.vertex[0].z);
		input0.push_back(static_cast<float>(cube.value[0]));

		input1.push_back(cube.vertex[4].x);
		input1.push_back(cube.vertex[4].y);
		input1.push_back(cube.vertex[4].z);
		input1.push_back(static_cast<float>(cube.value[4]));
	}

	if(mc_edge_table[case_index] & 512)
	{
		input0.push_back(cube.vertex[1].x);
		input0.push_back(cube.vertex[1].y);
		input0.push_back(cube.vertex[1].z);
		input0.push_back(static_cast<float>(cube.value[1]));

		input1.push_back(cube.vertex[5].x);
		input1.push_back(cube.vertex[5].y);
		input1.push_back(cube.vertex[5].z);
		input1.push_back(static_cast<float>(cube.value[5]));
	}

	if(mc_edge_table[case_index] & 1024)
	{
		input0.push_back(cube.vertex[2].x);
		input0.push_back(cube.vertex[2].y);
		input0.push_back(cube.vertex[2].z);
		input0.push_back(static_cast<float>(cube.value[2]));

		input1.push_back(cube.vertex[6].x);
		input1.push_back(cube.vertex[6].y);
		input1.push_back(cube.vertex[6].z);
		input1.push_back(static_cast<float>(cube.value[6]));
	}

	if(mc_edge_table[case_index] & 2048)
	{
		input0.push_back(cube.vertex[3].x);
		input0.push_back(cube.vertex[3].y);
		input0.push_back(cube.vertex[3].z);
		input0.push_back(static_cast<float>(cube.value[3]));

		input1.push_back(cube.vertex[7].x);
		input1.push_back(cube.vertex[7].y);
		input1.push_back(cube.vertex[7].z);
		input1.push_back(static_cast<float>(cube.value[7]));
	}
}


short unsigned int quaternion_julia_set::get_triangles_from_grid_cube(const mc_grid_cube &cube, vector<float> &output, size_t &current_vertex_index, triangle *const triangles)
{
	short unsigned int case_index = 0;

	// Note: default notation for MC -- small values (ie. false) are inside of the surface, large values (ie. true) are outside of the surface.
	if(false == cube.value[0]) case_index |= 1;
	if(false == cube.value[1]) case_index |= 2;
	if(false == cube.value[2]) case_index |= 4;
	if(false == cube.value[3]) case_index |= 8;
	if(false == cube.value[4]) case_index |= 16;
	if(false == cube.value[5]) case_index |= 32;
	if(false == cube.value[6]) case_index |= 64;
	if(false == cube.value[7]) case_index |= 128;

	if(0 == mc_edge_table[case_index])
		return 0;

	vertex_3 vertlist[12];

	if(mc_edge_table[case_index] & 1)
	{
		size_t output_index = current_vertex_index*4;

		vertlist[0].x = output[output_index + 0];
		vertlist[0].y = output[output_index + 1];
		vertlist[0].z = output[output_index + 2];

		current_vertex_index++;
	}

	if(mc_edge_table[case_index] & 2)
	{
		size_t output_index = current_vertex_index*4;

		vertlist[1].x = output[output_index + 0];
		vertlist[1].y = output[output_index + 1];
		vertlist[1].z = output[output_index + 2];

		current_vertex_index++;
	}

	if(mc_edge_table[case_index] & 4)
	{
		size_t output_index = current_vertex_index*4;

		vertlist[2].x = output[output_index + 0];
		vertlist[2].y = output[output_index + 1];
		vertlist[2].z = output[output_index + 2];

		current_vertex_index++;
	}

	if(mc_edge_table[case_index] & 8)
	{
		size_t output_index = current_vertex_index*4;

		vertlist[3].x = output[output_index + 0];
		vertlist[3].y = output[output_index + 1];
		vertlist[3].z = output[output_index + 2];

		current_vertex_index++;
	}

	if(mc_edge_table[case_index] & 16)
	{
		size_t output_index = current_vertex_index*4;

		vertlist[4].x = output[output_index + 0];
		vertlist[4].y = output[output_index + 1];
		vertlist[4].z = output[output_index + 2];

		current_vertex_index++;
	}

	if(mc_edge_table[case_index] & 32)
	{
		size_t output_index = current_vertex_index*4;

		vertlist[5].x = output[output_index + 0];
		vertlist[5].y = output[output_index + 1];
		vertlist[5].z = output[output_index + 2];

		current_vertex_index++;
	}

	if(mc_edge_table[case_index] & 64)
	{
		size_t output_index = current_vertex_index*4;

		vertlist[6].x = output[output_index + 0];
		vertlist[6].y = output[output_index + 1];
		vertlist[6].z = output[output_index + 2];

		current_vertex_index++;
	}

	if(mc_edge_table[case_index] & 128)
	{
		size_t output_index = current_vertex_index*4;

		vertlist[7].x = output[output_index + 0];
		vertlist[7].y = output[output_index + 1];
		vertlist[7].z = output[output_index + 2];

		current_vertex_index++;
	}

	if(mc_edge_table[case_index] & 256)
	{
		size_t output_index = current_vertex_index*4;

		vertlist[8].x = output[output_index + 0];
		vertlist[8].y = output[output_index + 1];
		vertlist[8].z = output[output_index + 2];

		current_vertex_index++;
	}

	if(mc_edge_table[case_index] & 512)
	{
		size_t output_index = current_vertex_index*4;

		vertlist[9].x = output[output_index + 0];
		vertlist[9].y = output[output_index + 1];
		vertlist[9].z = output[output_index + 2];

		current_vertex_index++;
	}

	if(mc_edge_table[case_index] & 1024)
	{
		size_t output_index = current_vertex_index*4;

		vertlist[10].x = output[output_index + 0];
		vertlist[10].y = output[output_index + 1];
		vertlist[10].z = output[output_index + 2];

		current_vertex_index++;
	}

	if(mc_edge_table[case_index] & 2048)
	{
		size_t output_index = current_vertex_index*4;

		vertlist[11].x = output[output_index + 0];
		vertlist[11].y = output[output_index + 1];
		vertlist[11].z = output[output_index + 2];

		current_vertex_index++;
	}

	short unsigned int num_tris = 0;

	for(short unsigned int i = 0; mc_tri_table[case_index][i] != -1; i += 3)
	{
		triangles[num_tris].vertex[0] = vertlist[mc_tri_table[case_index][i    ]];
		triangles[num_tris].vertex[1] = vertlist[mc_tri_table[case_index][i + 1]];
		triangles[num_tris].vertex[2] = vertlist[mc_tri_table[case_index][i + 2]];

		num_tris++;
	}

	return num_tris;
}

vertex_3 quaternion_julia_set::vertex_interp_float(vertex_3 v0, vertex_3 v1, float val_v0, float val_v1)
{
	// Sort the vertices so that way the same two vertices will always produce the same result
	// regardless of the order in which they were passed into the function.
	//
	// This may seem unnecessary, but adding two floats can produce different results depending
	// on their order, if the very rightmost decimal places are in use.
	if(v0 > v1)
	{
		vertex_3 temp(v0);
		float temp_val = val_v0;

		v0 = v1;
		val_v0 = val_v1;

		v1 = temp;
		val_v1 = temp_val;
	}

	// Start half-way between the vertices.
	vertex_3 result = (v0 + v1)*0.5f;

	// Refine the result, if need be.
	if(0 < vertex_refinement_steps)
	{
		vertex_3 forward, backward;

		// If p1 is outside of the surface and p2 is inside of the surface ...
		if(val_v0 > val_v1)
		{
			forward = v0;
			backward = v1;
		}
		else
		{
			forward = v1;
			backward = v0;
		}

		for(size_t i = 0; i < vertex_refinement_steps; i++)
		{
			// If point is in the quaternion Julia set, then move forward by 1/2 of a step, else move backward by 1/2 of a step ...
			if(threshold > eqparser.iterate(quaternion(result.x, result.y, result.z, z_w), max_iterations, threshold))
			{
				backward = result;
				result += (forward - result)*0.5f;
			}
			else
			{
				forward = result;
				result += (backward - result)*0.5f;
			}
		}
	}

	return result;
}

