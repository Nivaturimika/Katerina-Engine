#include "opengl_wrapper.hpp"
#include "system_state.hpp"
#include "simple_fs.hpp"
#include "fonts.hpp"
#include "bmfont.hpp"
#include "pdqsort.h"

namespace ogl {
	void debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei, GLchar const* message, void const*) {
#if 0
		std::string source_str;
		switch(source) {
		case GL_DEBUG_SOURCE_API:
			source_str = "OpenGL API call";
			break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
			source_str = "Window system API";
			break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER:
			source_str = "Shading language compiler";
			break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:
			source_str = "Application associated with OpenGL";
			break;
		case GL_DEBUG_SOURCE_APPLICATION:
			source_str = "User generated";
			break;
		case GL_DEBUG_SOURCE_OTHER:
			source_str = "Unknown source";
			break;
		}
		std::string error_type;
		switch(type) {
		case GL_DEBUG_TYPE_ERROR:
			error_type = "General error";
			break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			error_type = "Deprecated behavior";
			break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			error_type = "Undefined behavior";
			break;
		case GL_DEBUG_TYPE_PORTABILITY:
			error_type = "Portability issue";
			break;
		case GL_DEBUG_TYPE_PERFORMANCE:
			error_type = "Performance issue";
			break;
		case GL_DEBUG_TYPE_MARKER:
			error_type = "Command stream annotation";
			break;
		case GL_DEBUG_TYPE_PUSH_GROUP:
			error_type = "Error group push";
			break;
		case GL_DEBUG_TYPE_POP_GROUP:
			error_type = "Error group pop";
			break;
		case GL_DEBUG_TYPE_OTHER:
			error_type = "Uknown error type";
			break;
		}
		std::string severity_str;
		switch(severity) {
		case GL_DEBUG_SEVERITY_HIGH:
			severity_str = "High";
			break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			severity_str = "Medium";
			break;
		case GL_DEBUG_SEVERITY_LOW:
			severity_str = "Low";
			break;
		case GL_DEBUG_SEVERITY_NOTIFICATION:
			severity_str = "Notification";
			break;
		}
		std::string full_message("OpenGL error ");
		full_message += std::to_string(id);
		full_message += " ";
		full_message += "; Source: ";
		full_message += source_str;
		full_message += " ; Type: ";
		full_message += error_type;
		full_message += "; Severity: ";
		full_message += severity_str;
		full_message += "; ";
		full_message += message;
		full_message += "\n";
		reports::write_debug(full_message.c_str());
#endif
	}

	std::string_view opengl_get_error_name(GLenum t) {
		switch(t) {
		case GL_INVALID_ENUM:
			return "GL_INVALID_ENUM";
		case GL_INVALID_VALUE:
			return "GL_INVALID_VALUE";
		case GL_INVALID_OPERATION:
			return "GL_INVALID_OPERATION";
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			return "GL_INVALID_FRAMEBUFFER_OPERATION";
		case GL_OUT_OF_MEMORY:
			return "GL_OUT_OF_MEMORY";
		case GL_STACK_UNDERFLOW:
			return "GL_STACK_UNDERFLOW";
		case GL_STACK_OVERFLOW:
			return "GL_STACK_OVERFLOW";
		case GL_NO_ERROR:
			return "GL_NO_ERROR";
		default:
			return "Unknown";
		}
	}

	void notify_user_of_fatal_opengl_error(std::string message) {
		std::string full_message = message;
		full_message += "\n";
		full_message += opengl_get_error_name(glGetError());
		window::emit_error_message("OpenGL error:" + full_message, false);
	}

	std::unique_ptr<char[]> get_shader_log(GLuint shader) {
		GLint length = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		auto log = std::unique_ptr<char[]>(new char[static_cast<size_t>(length)]);
		GLsizei written = 0;
		glGetShaderInfoLog(shader, length, &written, log.get());
		return log;
	}

	GLint compile_shader(std::string_view source, GLenum type) {
		GLuint shader = glCreateShader(type);
		if(!shader) {
			notify_user_of_fatal_opengl_error("shader creation failed");
			return 0;
		}

		std::string s_source(source);
		GLchar const* texts[] = {
			"#version 140\n"
			"#extension GL_ARB_explicit_uniform_location : enable\n"
			"#extension GL_ARB_explicit_attrib_location : enable\n"
			"#extension GL_ARB_shader_subroutine : enable\n"
			"#extension GL_ARB_vertex_array_object : enable\n"
			"#define M_PI 3.1415926535897932384626433832795f\n"
			"#define PI 3.1415926535897932384626433832795f\n",
			s_source.c_str()
		};
		glShaderSource(shader, std::extent_v<decltype(texts)>, texts, nullptr);
		glCompileShader(shader);

		GLint result;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
		if(result == GL_FALSE) {
			auto log = get_shader_log(shader);
			notify_user_of_fatal_opengl_error(std::string("Shader failed to compile:\n") + (log.get() ? log.get() : "No log"));
			//dispose of resource
			glDeleteShader(shader);
			return 0;
		}
		auto log = get_shader_log(shader);
		reports::write_debug(log.get() ? log.get() : "No log");
		return shader;
	}

	GLuint create_program(std::string_view vertex_shader, std::string_view fragment_shader, uint32_t flags) {
		GLint result = 0;
		GLint lib_v_shader = 0;
		GLint lib_f_shader = 0;
		GLint v_shader = 0;
		GLint f_shader = 0;

		GLuint program = glCreateProgram();
		if(program == 0) {
			notify_user_of_fatal_opengl_error("Program creation failed");
			goto error_exit;
		}

		v_shader = compile_shader(vertex_shader, GL_VERTEX_SHADER);
		if(!v_shader)
			goto error_exit;

		f_shader = compile_shader(fragment_shader, GL_FRAGMENT_SHADER);
		if(!f_shader)
			goto error_exit;

		lib_f_shader = compile_shader(
		"uniform float gamma;\n"
		"vec4 gamma_correct(vec4 colour) {\n"
			"\treturn vec4(pow(colour.rgb, vec3(1.f / gamma)), colour.a);\n"
		"}\n"
		"\n", GL_FRAGMENT_SHADER);
		if(!lib_f_shader)
			goto error_exit;

		lib_v_shader = 0;
		if(flags == 0) {
			// Globe coords
			lib_v_shader = compile_shader(
			"uniform vec2 offset;\n"
			"uniform mat4 model_proj_view;\n"
			"vec4 globe_coords(vec3 world_pos) {\n"
				"\t\tvec4 new_world_pos = vec4(1.f);\n"
				"\tnew_world_pos.x = cos(world_pos.x * 2.f * PI) * sin(world_pos.y * PI);\n"
				"\tnew_world_pos.y = 2.f * sin(world_pos.x * PI) * cos(world_pos.x * PI) * sin(world_pos.y * PI);\n"
				"\tnew_world_pos.z = cos(world_pos.y * PI);\n"
				"\t\treturn model_proj_view * new_world_pos * (1.f + world_pos.z);\n"
			"}\n"
			"vec4 calc_gl_position(in vec3 world_pos) {\n"
				"\treturn globe_coords(vec3(world_pos.x, world_pos.z, world_pos.y));\n"
			"}\n", GL_VERTEX_SHADER);
		} else if(flags == 1) {
			//Flat coords
			lib_v_shader = compile_shader(
			"uniform vec2 offset;\n"
			"uniform mat4 model_proj_view;\n"
			"//Flat coords\n"
			"vec4 flat_coords(vec4 world_pos) {\n"
				"\tworld_pos.x = mod(world_pos.x - offset.x, 1.f);\n"
				"\tvec4 v = model_proj_view * world_pos;\n"
				"\tv.w = v.z + 1.f;\n"
				"\treturn v;\n"
			"}\n"
			"vec4 calc_gl_position(in vec3 world_pos) {\n"
				"\treturn flat_coords(vec4(world_pos.x, world_pos.z, world_pos.y, 1.f));\n"
			"}\n", GL_VERTEX_SHADER);
		} else if(flags == 2) {
			//Perspective coords
			lib_v_shader = compile_shader(
			"uniform vec2 offset;\n"
			"uniform mat4 model_proj_view;\n"
			"vec4 perspective_coords(vec2 world_pos) {\n"
				"\t\tvec4 new_world_pos = vec4(1.f);\n"
				"\tnew_world_pos.x = cos(world_pos.x * 2.f * PI) * sin(world_pos.y * PI);\n"
				"\tnew_world_pos.y = 2.f * sin(world_pos.x * PI) * cos(world_pos.x * PI) * sin(world_pos.y * PI);\n"
				"\tnew_world_pos.z = cos(world_pos.y * PI);\n"
				"\treturn model_proj_view * new_world_pos;\n"
			"}\n"
			"vec4 calc_gl_position(in vec3 world_pos) {\n"
				"\treturn perspective_coords(vec2(world_pos.x, world_pos.z));\n"
			"}\n", GL_VERTEX_SHADER);
		}
		if(!lib_v_shader) {
			goto error_exit;
		}

		glAttachShader(program, v_shader);
		glAttachShader(program, f_shader);
		glAttachShader(program, lib_v_shader);
		glAttachShader(program, lib_f_shader);
		glLinkProgram(program);
		glGetProgramiv(program, GL_LINK_STATUS, &result);
		if(result == GL_FALSE) {
			GLint length = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
			auto log = std::unique_ptr<char[]>(new char[static_cast<size_t>(length)]);
			GLsizei written = 0;
			glGetProgramInfoLog(program, length, &written, log.get());
			notify_user_of_fatal_opengl_error(std::string("Program failed to link:\n") + log.get());
			goto error_exit;
		}

		glDeleteShader(v_shader);
		glDeleteShader(f_shader);
		glDeleteShader(lib_v_shader);
		glDeleteShader(lib_f_shader);
		return program;
	error_exit:
		if(v_shader)
			glDeleteShader(v_shader);
		if(f_shader)
			glDeleteShader(f_shader);
		if(lib_v_shader)
			glDeleteShader(lib_v_shader);
		if(lib_f_shader)
			glDeleteShader(lib_f_shader);
		if(program)
			glDeleteProgram(program);
		return 0;
	}

	void load_special_icons(sys::state& state) {
		reports::write_debug("Loading special GFX icons\n");

		auto root = get_root(state.common_fs);
		auto gfx_dir = simple_fs::open_directory(root, NATIVE("gfx"));

		auto interface_dir = simple_fs::open_directory(gfx_dir, NATIVE("interface"));
		auto money_dds = simple_fs::open_file(interface_dir, NATIVE("icon_money_big.dds"));
		if(money_dds) {
			auto content = simple_fs::view_contents(*money_dds);
			uint32_t size_x, size_y;
			state.open_gl.money_icon_tex = GLuint(ogl::SOIL_direct_load_DDS_from_memory(reinterpret_cast<uint8_t const*>(content.data),
				content.file_size, size_x, size_y, ogl::SOIL_FLAG_TEXTURE_REPEATS));
		}

		auto assets_dir = simple_fs::open_directory(root, NATIVE("assets"));
		auto cross_dds = simple_fs::open_file(assets_dir, NATIVE("trigger_not.dds"));
		if(cross_dds) {
			auto content = simple_fs::view_contents(*cross_dds);
			uint32_t size_x, size_y;
			state.open_gl.cross_icon_tex = GLuint(ogl::SOIL_direct_load_DDS_from_memory(reinterpret_cast<uint8_t const*>(content.data),
				content.file_size, size_x, size_y, ogl::SOIL_FLAG_TEXTURE_REPEATS));
		}
		auto cb_cross_dds = simple_fs::open_file(assets_dir, NATIVE("trigger_not_cb.dds"));
		if(cb_cross_dds) {
			auto content = simple_fs::view_contents(*cb_cross_dds);
			uint32_t size_x, size_y;
			state.open_gl.color_blind_cross_icon_tex = GLuint(ogl::SOIL_direct_load_DDS_from_memory(reinterpret_cast<uint8_t const*>(content.data),
			content.file_size, size_x, size_y, ogl::SOIL_FLAG_TEXTURE_REPEATS));
		}
		auto checkmark_dds = simple_fs::open_file(assets_dir, NATIVE("trigger_yes.dds"));
		if(checkmark_dds) {
			auto content = simple_fs::view_contents(*checkmark_dds);
			uint32_t size_x, size_y;
			state.open_gl.checkmark_icon_tex = GLuint(ogl::SOIL_direct_load_DDS_from_memory(
				reinterpret_cast<uint8_t const*>(content.data), content.file_size, size_x, size_y, ogl::SOIL_FLAG_TEXTURE_REPEATS));
		}

		auto n_dds = simple_fs::open_file(interface_dir, NATIVE("politics_foreign_naval_units.dds"));
		if(n_dds) {
			auto content = simple_fs::view_contents(*n_dds);
			uint32_t size_x, size_y;
			state.open_gl.navy_icon_tex = GLuint(ogl::SOIL_direct_load_DDS_from_memory(
			reinterpret_cast<uint8_t const*>(content.data), content.file_size, size_x, size_y, ogl::SOIL_FLAG_TEXTURE_REPEATS));
		}
		auto a_dds = simple_fs::open_file(interface_dir, NATIVE("topbar_army.dds"));
		if(a_dds) {
			auto content = simple_fs::view_contents(*a_dds);
			uint32_t size_x, size_y;
			state.open_gl.army_icon_tex = GLuint(ogl::SOIL_direct_load_DDS_from_memory(
			reinterpret_cast<uint8_t const*>(content.data), content.file_size, size_x, size_y, ogl::SOIL_FLAG_TEXTURE_REPEATS));
		}
	}

	std::string_view framebuffer_error(GLenum e) {
		switch(e) {
		case GL_FRAMEBUFFER_UNDEFINED:
			return "GL_FRAMEBUFFER_UNDEFINED";
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT ";
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			return "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			return "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
		case GL_FRAMEBUFFER_UNSUPPORTED:
			return "GL_FRAMEBUFFER_UNSUPPORTED";
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			return "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
			return "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
		default:
			break;
		}
		return "???";
	}

	void initialize_msaa(sys::state& state, int32_t size_x, int32_t size_y) {
		if(state.user_settings.antialias_level == 0)
			return;
		if(!size_x || !size_y)
			return;

		reports::write_debug("Initializing MSAA\n");
		glEnable(GL_MULTISAMPLE);
		// setup screen VAO
		static const float sq_vertices[] = {
			// positions   // texCoords
			-1.0f,  1.0f,  0.0f, 1.0f,
			-1.0f, -1.0f,  0.0f, 0.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,
			-1.0f,  1.0f,  0.0f, 1.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,
			 1.0f,  1.0f,  1.0f, 1.0f
		};
		glGenVertexArrays(1, &state.open_gl.msaa_vao);
		glGenBuffers(1, &state.open_gl.msaa_vbo);
		glBindVertexArray(state.open_gl.msaa_vao);
		glBindBuffer(GL_ARRAY_BUFFER, state.open_gl.msaa_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(sq_vertices), &sq_vertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
		// framebuffer
		glGenFramebuffers(1, &state.open_gl.msaa_framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, state.open_gl.msaa_framebuffer);
		// create a multisampled color attachment texture
		glGenTextures(1, &state.open_gl.msaa_texcolorbuffer);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, state.open_gl.msaa_texcolorbuffer);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, GLsizei(state.user_settings.antialias_level), GL_RGBA, size_x, size_y, GL_TRUE);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, state.open_gl.msaa_texcolorbuffer, 0);
		// create a (also multisampled) renderbuffer object for depth and stencil attachments
		glGenRenderbuffers(1, &state.open_gl.msaa_rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, state.open_gl.msaa_rbo);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, GLsizei(state.user_settings.antialias_level), GL_DEPTH24_STENCIL8, size_x, size_y);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, state.open_gl.msaa_rbo);
		if(auto r = glCheckFramebufferStatus(GL_FRAMEBUFFER); r != GL_FRAMEBUFFER_COMPLETE) {
			//notify_user_of_fatal_opengl_error("MSAA framebuffer wasn't completed: " + std::string(framebuffer_error(r)));
			state.user_settings.antialias_level--;
			deinitialize_msaa(state);
			if(state.user_settings.antialias_level != 0) {
				initialize_msaa(state, size_x, size_y);
			} else {
				state.open_gl.msaa_enabled = false;
			}
			return;
		}
		// configure second post-processing framebuffer
		glGenFramebuffers(1, &state.open_gl.msaa_interbuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, state.open_gl.msaa_interbuffer);
		// create a color attachment texture
		glGenTextures(1, &state.open_gl.msaa_texture);
		glBindTexture(GL_TEXTURE_2D, state.open_gl.msaa_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size_x, size_y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, state.open_gl.msaa_texture, 0);	// we only need a color buffer
		if(auto r = glCheckFramebufferStatus(GL_FRAMEBUFFER); r != GL_FRAMEBUFFER_COMPLETE) {
			notify_user_of_fatal_opengl_error("MSAA post processing framebuffer wasn't completed: " + std::string(framebuffer_error(r)));
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		auto root = get_root(state.common_fs);

		auto assets_dir = open_directory(root, NATIVE("assets"));
		auto shaders_dir = open_directory(assets_dir, NATIVE("shaders"));
		auto msaa_fshader = open_file(shaders_dir, NATIVE("msaa_f_shader.glsl"));
		auto msaa_vshader = open_file(shaders_dir, NATIVE("msaa_v_shader.glsl"));
		if(bool(msaa_fshader) && bool(msaa_vshader)) {
			auto vertex_content = view_contents(*msaa_vshader);
			auto fragment_content = view_contents(*msaa_fshader);
			state.open_gl.msaa_shader_program = create_program(std::string_view(vertex_content.data, vertex_content.file_size), std::string_view(fragment_content.data, fragment_content.file_size), 0);
			if(state.open_gl.msaa_shader_program) {
				state.open_gl.msaa_uniform_screen_size = glGetUniformLocation(state.open_gl.msaa_shader_program, "screen_size");
				state.open_gl.msaa_uniform_gaussian_blur = glGetUniformLocation(state.open_gl.msaa_shader_program, "gaussian_radius");
			}
		} else {
			notify_user_of_fatal_opengl_error("Unable to open a MSAA shaders files");
		}
		state.open_gl.msaa_enabled = true;
	}


	void deinitialize_msaa(sys::state& state) {
		if(!state.open_gl.msaa_enabled)
			return;

		reports::write_debug("Deinitializing MSAA\n");
		state.open_gl.msaa_enabled = false;
		if(state.open_gl.msaa_texture)
			glDeleteTextures(1, &state.open_gl.msaa_texture);
		if(state.open_gl.msaa_interbuffer)
			glDeleteFramebuffers(1, &state.open_gl.msaa_framebuffer);
		if(state.open_gl.msaa_rbo)
			glDeleteRenderbuffers(1, &state.open_gl.msaa_rbo);
		if(state.open_gl.msaa_texcolorbuffer)
			glDeleteTextures(1, &state.open_gl.msaa_texcolorbuffer);
		if(state.open_gl.msaa_framebuffer)
			glDeleteFramebuffers(1, &state.open_gl.msaa_framebuffer);
		if(state.open_gl.msaa_vbo)
			glDeleteBuffers(1, &state.open_gl.msaa_vbo);
		if(state.open_gl.msaa_vao)
			glDeleteVertexArrays(1, &state.open_gl.msaa_vao);
		if(state.open_gl.msaa_shader_program)
			glDeleteProgram(state.open_gl.msaa_shader_program);
		glDisable(GL_MULTISAMPLE);
	}

	void initialize_opengl(sys::state& state) {
		reports::write_debug("Initializing OpenGL context\n");

		create_opengl_context(state);

		glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);
		glEnable(GL_LINE_SMOOTH);

		load_shaders(state); // create shaders
		load_global_squares(state); // create various squares to drive the shaders with

		state.flag_type_map.resize(size_t(culture::flag_type::count), 0);
		// Create the remapping for flags
		state.world.for_each_national_identity([&](dcon::national_identity_id ident_id) {
			auto fat_id = dcon::fatten(state.world, ident_id);
			auto nat_id = fat_id.get_nation_from_identity_holder().id;
			for(auto gov_id : state.world.in_government_type) {
				state.flag_types.push_back(culture::flag_type(gov_id.get_flag()));
			}
		});
		// Eliminate duplicates
		pdqsort(state.flag_types.begin(), state.flag_types.end());
		state.flag_types.erase(std::unique(state.flag_types.begin(), state.flag_types.end()), state.flag_types.end());

		// Automatically assign texture offsets to the flag_types
		auto id = 0;
		for(auto type : state.flag_types)
			state.flag_type_map[uint32_t(type)] = uint8_t(id++);
		assert(state.flag_type_map[0] == 0); // default_flag

		// Allocate textures for the flags
		state.open_gl.asset_textures.resize(state.ui_defs.textures.size() + (state.world.national_identity_size() + 1) * state.flag_types.size());

		state.map_state.load_map(state);

		load_special_icons(state);

		initialize_msaa(state, window::creation_parameters().size_x, window::creation_parameters().size_y);
	}

	void load_shaders(sys::state& state) {
		auto root = get_root(state.common_fs);

		auto assets_dir = open_directory(root, NATIVE("assets"));
		auto shaders_dir = open_directory(assets_dir, NATIVE("shaders"));
		auto ui_fshader = open_file(shaders_dir, NATIVE("ui_f_shader.glsl"));
		auto ui_vshader = open_file(shaders_dir, NATIVE("ui_v_shader.glsl"));
		if(bool(ui_fshader) && bool(ui_vshader)) {
			auto vertex_content = view_contents(*ui_vshader);
			auto fragment_content = view_contents(*ui_fshader);
			state.open_gl.ui_shader_program = create_program(std::string_view(vertex_content.data, vertex_content.file_size), std::string_view(fragment_content.data, fragment_content.file_size) ,0);
			if(state.open_gl.ui_shader_program) {
				state.open_gl.ui_shader_texture_sampler_uniform = glGetUniformLocation(state.open_gl.ui_shader_program, "texture_sampler");
				state.open_gl.ui_shader_secondary_texture_sampler_uniform = glGetUniformLocation(state.open_gl.ui_shader_program, "secondary_texture_sampler");
				state.open_gl.ui_shader_screen_width_uniform = glGetUniformLocation(state.open_gl.ui_shader_program, "screen_width");
				state.open_gl.ui_shader_screen_height_uniform = glGetUniformLocation(state.open_gl.ui_shader_program, "screen_height");
				state.open_gl.ui_shader_gamma_uniform = glGetUniformLocation(state.open_gl.ui_shader_program, "gamma");

				state.open_gl.ui_shader_d_rect_uniform = glGetUniformLocation(state.open_gl.ui_shader_program, "d_rect");
				state.open_gl.ui_shader_subroutines_index_uniform = glGetUniformLocation(state.open_gl.ui_shader_program, "subroutines_index");
				state.open_gl.ui_shader_inner_color_uniform = glGetUniformLocation(state.open_gl.ui_shader_program, "inner_color");
				state.open_gl.ui_shader_subrect_uniform = glGetUniformLocation(state.open_gl.ui_shader_program, "subrect");
				state.open_gl.ui_shader_border_size_uniform = glGetUniformLocation(state.open_gl.ui_shader_program, "border_size");
			}
		} else {
			notify_user_of_fatal_opengl_error("Unable to open the UI shaders files");
		}
	}

	void load_global_square(GLuint vao, GLuint vbo, GLushort const* data) {
		assert(vao && vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLushort) * 16, data, GL_STATIC_DRAW);
		glBindVertexArray(vao);
		glVertexAttribPointer(0, 2, GL_UNSIGNED_SHORT, GL_TRUE, sizeof(GLushort) * 4, 0); // position
		glVertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_TRUE, sizeof(GLushort) * 4, (const void*)(sizeof(GLushort) * 2)); // texture coordinates
		glEnableVertexAttribArray(0); // position
		glEnableVertexAttribArray(1); // texture coordinates
	}

	void load_global_squares(sys::state& state) {
		static const GLushort global_square_data[] = {
			0, 0, 0, 0,
			0, 65535, 0, 65535,
			65535, 65535, 65535, 65535,
			65535, 0, 65535, 0
		};
		static const GLushort global_square_right_data[] = {
			0, 0, 0, 65535,
			0, 65535, 65535, 65535,
			65535, 65535, 65535, 0,
			65535, 0, 0, 0
		};
		static const GLushort global_square_left_data[] = {
			0, 0, 65535, 0,
			0, 65535, 0, 0,
			65535, 65535, 0, 65535,
			65535, 0, 65535, 65535
		};
		static const GLushort global_square_flipped_data[] = {
			0, 0, 0, 65535,
			0, 65535, 0, 0,
			65535, 65535, 65535, 0,
			65535, 0, 65535, 65535
		};
		static const GLushort global_square_right_flipped_data[] = {
			0, 0, 0, 0,
			0, 65535, 65535, 0,
			65535, 65535, 65535, 65535,
			65535, 0, 0, 65535
		};
		static const GLushort global_square_left_flipped_data[] = {
			0, 0, 65535, 65535,
			0, 65535, 0, 65535,
			65535, 65535, 0, 0,
			65535, 0, 65535, 0
		};
		//RTL squares
		static const GLushort global_rtl_square_data[] = {
			0, 0, 65535, 0,
			0, 65535, 65535, 65535,
			65535, 65535, 0, 65535,
			65535, 0, 0, 0
		};
		static const GLushort global_rtl_square_right_data[] = {
			0, 65535, 65535, 0,
			0, 0, 0, 0,
			65535, 0, 0, 65535,
			65535, 65535, 65535, 65535
		};
		static const GLushort global_rtl_square_left_data[] = {
			0, 0, 0, 0,
			0, 65535, 65535, 0,
			65535, 65535, 65535, 65535,
			65535, 0, 0, 65535
		};
		static const GLushort global_rtl_square_flipped_data[] = {
			0, 0, 65535, 65535,
			0, 65535, 65535, 0,
			65535, 65535, 0, 0,
			65535, 0, 0, 65535
		};
		static const GLushort global_rtl_square_right_flipped_data[] = {
			0, 0, 65535, 0,
			0, 65535, 0, 0,
			65535, 65535, 0, 65535,
			65535, 0, 65535, 65535
		};
		static const GLushort global_rtl_square_left_flipped_data[] = {
			0, 0, 0, 65535,
			0, 65535, 65535, 65535,
			65535, 65535, 65535, 0,
			65535, 0, 0, 0
		};

		glGenVertexArrays(64 + 12, state.open_gl.global_square_vao);

		// Populate the position buffer
		glGenBuffers(1, &state.open_gl.global_square_buffer);
		load_global_square(state.open_gl.global_square_vao[0], state.open_gl.global_square_buffer, global_square_data);

		glGenBuffers(1, &state.open_gl.global_square_left_buffer);
		load_global_square(state.open_gl.global_square_vao[1], state.open_gl.global_square_left_buffer, global_square_left_data);

		glGenBuffers(1, &state.open_gl.global_square_right_buffer);
		load_global_square(state.open_gl.global_square_vao[2], state.open_gl.global_square_right_buffer, global_square_right_data);

		glGenBuffers(1, &state.open_gl.global_square_right_flipped_buffer);
		load_global_square(state.open_gl.global_square_vao[3], state.open_gl.global_square_right_flipped_buffer, global_square_right_flipped_data);

		glGenBuffers(1, &state.open_gl.global_square_left_flipped_buffer);
		load_global_square(state.open_gl.global_square_vao[4], state.open_gl.global_square_left_flipped_buffer, global_square_left_flipped_data);

		glGenBuffers(1, &state.open_gl.global_square_flipped_buffer);
		load_global_square(state.open_gl.global_square_vao[5], state.open_gl.global_square_flipped_buffer, global_square_flipped_data);

		//RTL mode squares
		glGenBuffers(1, &state.open_gl.global_rtl_square_buffer);
		load_global_square(state.open_gl.global_square_vao[6], state.open_gl.global_rtl_square_buffer, global_rtl_square_data);

		glGenBuffers(1, &state.open_gl.global_rtl_square_left_buffer);
		load_global_square(state.open_gl.global_square_vao[7], state.open_gl.global_rtl_square_left_buffer, global_rtl_square_left_data);

		glGenBuffers(1, &state.open_gl.global_rtl_square_right_buffer);
		load_global_square(state.open_gl.global_square_vao[8], state.open_gl.global_rtl_square_right_buffer, global_rtl_square_right_data);

		glGenBuffers(1, &state.open_gl.global_rtl_square_right_flipped_buffer);
		load_global_square(state.open_gl.global_square_vao[9], state.open_gl.global_rtl_square_right_flipped_buffer, global_rtl_square_right_flipped_data);

		glGenBuffers(1, &state.open_gl.global_rtl_square_left_flipped_buffer);
		load_global_square(state.open_gl.global_square_vao[10], state.open_gl.global_rtl_square_left_flipped_buffer, global_rtl_square_left_flipped_data);

		glGenBuffers(1, &state.open_gl.global_rtl_square_flipped_buffer);
		load_global_square(state.open_gl.global_square_vao[11], state.open_gl.global_rtl_square_flipped_buffer, global_rtl_square_flipped_data);

		glGenBuffers(64, state.open_gl.sub_square_buffers);
		for(uint32_t i = 0; i < 64; ++i) {
			float const cell_x = static_cast<float>(i & 7) / 8.0f;
			float const cell_y = static_cast<float>((i >> 3) & 7) / 8.0f;
			GLushort global_sub_square_data[] = {
				0, 0, uint16_t((cell_x) * 65535.f), uint16_t((cell_y) * 65535.f),
				0, 65535, uint16_t((cell_x) * 65535.f), uint16_t((cell_y + 1.0f / 8.0f) * 65535.f),
				65535, 65535, uint16_t((cell_x + 1.0f / 8.0f) * 65535.f), uint16_t((cell_y + 1.0f / 8.0f) * 65535.f),
				65535, 0, uint16_t((cell_x + 1.0f / 8.0f) * 65535.f), uint16_t((cell_y) * 65535.f)
			};
			load_global_square(state.open_gl.global_square_vao[12 + i], state.open_gl.sub_square_buffers[i], global_sub_square_data);
		}
	}

	inline auto map_color_modification_to_index(color_modification e) {
		switch(e) {
		case color_modification::disabled:
			return parameters::disabled;
		case color_modification::interactable:
			return parameters::interactable;
		case color_modification::interactable_disabled:
			return parameters::interactable_disabled;
		case color_modification::none:
		default:
			return parameters::enabled;
		}
	}

	void bind_vertices_by_rotation(sys::state const& state, ui::rotation r, bool flipped, bool rtl) {
		switch(r) {
		case ui::rotation::upright:
			if(!flipped) {
				glBindVertexArray(state.open_gl.global_square_vao[rtl ? 6 : 0]);
				glBindBuffer(GL_ARRAY_BUFFER, rtl ? state.open_gl.global_rtl_square_buffer : state.open_gl.global_square_buffer);
			} else {
				glBindVertexArray(state.open_gl.global_square_vao[rtl ? 11 : 5]);
				glBindBuffer(GL_ARRAY_BUFFER, rtl ? state.open_gl.global_rtl_square_flipped_buffer : state.open_gl.global_square_flipped_buffer);
			}
			break;
		case ui::rotation::r90_left:
			if(!flipped) {
				glBindVertexArray(state.open_gl.global_square_vao[rtl ? 7 : 1]);
				glBindBuffer(GL_ARRAY_BUFFER, rtl ? state.open_gl.global_rtl_square_left_buffer : state.open_gl.global_square_left_buffer);
			} else {
				glBindVertexArray(state.open_gl.global_square_vao[rtl ? 10 : 4]);
				glBindBuffer(GL_ARRAY_BUFFER, rtl ? state.open_gl.global_rtl_square_left_flipped_buffer : state.open_gl.global_square_left_flipped_buffer);
			}
			break;
		case ui::rotation::r90_right:
			if(!flipped) {
				glBindVertexArray(state.open_gl.global_square_vao[rtl ? 8 : 2]);
				glBindBuffer(GL_ARRAY_BUFFER, rtl ? state.open_gl.global_rtl_square_right_buffer : state.open_gl.global_square_right_buffer);
			} else {
				glBindVertexArray(state.open_gl.global_square_vao[rtl ? 9 : 3]);
				glBindBuffer(GL_ARRAY_BUFFER, rtl ? state.open_gl.global_rtl_square_right_flipped_buffer : state.open_gl.global_square_right_flipped_buffer);
			}
			break;
		}
	}

	void render_simple_rect(sys::state const& state, float x, float y, float width, float height, ui::rotation r, bool flipped, bool rtl) {
		bind_vertices_by_rotation(state, r, flipped, rtl);
		glUniform4f(state.open_gl.ui_shader_d_rect_uniform, x, y, width, height);
		GLuint subroutines[2] = { map_color_modification_to_index(color_modification::none), parameters::linegraph_color };
		glUniform2ui(state.open_gl.ui_shader_subroutines_index_uniform, subroutines[0], subroutines[1]);
		//glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 2, subroutines); // must set all subroutines in one call
		glUniform3f(state.open_gl.ui_shader_inner_color_uniform, 1.f, 0.f, 0.f);
		glLineWidth(2.0f);
		glDrawArrays(GL_LINE_STRIP, 0, 4);
	}

	void render_textured_rect(sys::state const& state, color_modification enabled, float x, float y, float width, float height, GLuint texture_handle, ui::rotation r, bool flipped, bool rtl) {
		bind_vertices_by_rotation(state, r, flipped, rtl);

		glUniform4f(state.open_gl.ui_shader_d_rect_uniform, x, y, width, height);
		glBindTexture(GL_TEXTURE_2D, texture_handle);

		GLuint subroutines[2] = { map_color_modification_to_index(enabled), parameters::no_filter };
		glUniform2ui(state.open_gl.ui_shader_subroutines_index_uniform, subroutines[0], subroutines[1]);
		//glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 2, subroutines); // must set all subroutines in one call

		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	}

	void render_textured_rect_direct(sys::state const& state, float x, float y, float width, float height, uint32_t handle) {
		glBindVertexArray(state.open_gl.global_square_vao[0]);
		glBindBuffer(GL_ARRAY_BUFFER, state.open_gl.global_square_buffer);
		glUniform4f(state.open_gl.ui_shader_d_rect_uniform, x, y, width, height);
		glBindTexture(GL_TEXTURE_2D, handle);

		GLuint subroutines[2] = {parameters::enabled, parameters::no_filter};
		glUniform2ui(state.open_gl.ui_shader_subroutines_index_uniform, subroutines[0], subroutines[1]);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	}

	void render_linegraph(sys::state const& state, color_modification enabled, float x, float y, float width, float height, lines& l) {
		//glBindVertexArray(state.open_gl.global_square_vao);
		l.bind_buffer();

		glUniform4f(state.open_gl.ui_shader_d_rect_uniform, x, y, width, height);
		GLuint subroutines[2] = { map_color_modification_to_index(enabled), parameters::linegraph };
		glUniform2ui(state.open_gl.ui_shader_subroutines_index_uniform, subroutines[0], subroutines[1]);
		//glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 2, subroutines); // must set all subroutines in one call

		glLineWidth(4.0f);
		glUniform3f(state.open_gl.ui_shader_inner_color_uniform, 0.f, 0.f, 0.f);
		glDrawArrays(GL_LINE_STRIP, 0, static_cast<GLsizei>(l.count));
		glLineWidth(2.0f);
		if(state.user_settings.color_blind_mode == sys::color_blind_mode::achroma) {
			glUniform3f(state.open_gl.ui_shader_inner_color_uniform, 0.f, 0.f, 0.f);
		} else if(state.user_settings.color_blind_mode == sys::color_blind_mode::tritan) {
			glUniform3f(state.open_gl.ui_shader_inner_color_uniform, 1.f, 1.f, 0.f);
		} else if(state.user_settings.color_blind_mode == sys::color_blind_mode::deutan || state.user_settings.color_blind_mode == sys::color_blind_mode::protan) {
			glUniform3f(state.open_gl.ui_shader_inner_color_uniform, 1.f, 1.f, 1.f);
		} else {
			glUniform3f(state.open_gl.ui_shader_inner_color_uniform, 1.f, 1.f, 0.f);
		}
		glDrawArrays(GL_LINE_STRIP, 0, static_cast<GLsizei>(l.count));
	}

	void render_linegraph(sys::state const& state, color_modification enabled, float x, float y, float width, float height, float r, float g, float b, lines& l) {
		//glBindVertexArray(state.open_gl.global_square_vao);
		l.bind_buffer();

		glUniform4f(state.open_gl.ui_shader_d_rect_uniform, x, y, width, height);
		GLuint subroutines[2] = { map_color_modification_to_index(enabled), parameters::linegraph_color };
		glUniform2ui(state.open_gl.ui_shader_subroutines_index_uniform, subroutines[0], subroutines[1]);
		//glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 2, subroutines); // must set all subroutines in one call

		glLineWidth(4.0f);
		glUniform3f(state.open_gl.ui_shader_inner_color_uniform, 0.f, 0.f, 0.f);
		glDrawArrays(GL_LINE_STRIP, 0, static_cast<GLsizei>(l.count));
		glLineWidth(2.0f);
		glUniform3f(state.open_gl.ui_shader_inner_color_uniform, r, g, b);
		glDrawArrays(GL_LINE_STRIP, 0, static_cast<GLsizei>(l.count));
	}

	void render_barchart(sys::state const& state, color_modification enabled, float x, float y, float width, float height,
		data_texture& t, ui::rotation r, bool flipped, bool rtl) {
		//glBindVertexArray(state.open_gl.global_square_vao);
		bind_vertices_by_rotation(state, r, flipped, rtl);
		glUniform4f(state.open_gl.ui_shader_d_rect_uniform, x, y, width, height);
		glBindTexture(GL_TEXTURE_2D, t.handle());

		GLuint subroutines[2] = { map_color_modification_to_index(enabled), parameters::barchart };
		glUniform2ui(state.open_gl.ui_shader_subroutines_index_uniform, subroutines[0], subroutines[1]);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	}

	void render_piechart(sys::state const& state, color_modification enabled, float x, float y, float size, data_texture& t) {
		//glBindVertexArray(state.open_gl.global_square_vao);
		glBindBuffer(GL_ARRAY_BUFFER, state.open_gl.global_square_buffer);
		glUniform4f(state.open_gl.ui_shader_d_rect_uniform, x, y, size, size);
		glBindTexture(GL_TEXTURE_2D, t.handle());

		GLuint subroutines[2] = { map_color_modification_to_index(enabled), parameters::piechart };
		glUniform2ui(state.open_gl.ui_shader_subroutines_index_uniform, subroutines[0], subroutines[1]);
		//glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 2, subroutines); // must set all subroutines in one call

		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	}

	void render_bordered_rect(sys::state const& state, color_modification enabled, float border_size, float x, float y, float width,
		float height, GLuint texture_handle, ui::rotation r, bool flipped, bool rtl) {
		//glBindVertexArray(state.open_gl.global_square_vao);
		bind_vertices_by_rotation(state, r, flipped, rtl);

		glUniform4f(state.open_gl.ui_shader_d_rect_uniform, x, y, width, height);
		glUniform1f(state.open_gl.ui_shader_border_size_uniform, border_size);
		glBindTexture(GL_TEXTURE_2D, texture_handle);

		GLuint subroutines[2] = { map_color_modification_to_index(enabled), parameters::frame_stretch };
		glUniform2ui(state.open_gl.ui_shader_subroutines_index_uniform, subroutines[0], subroutines[1]);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	}

	void render_masked_rect(sys::state const& state, color_modification enabled, float x, float y, float width, float height, GLuint texture_handle, GLuint mask_texture_handle, ui::rotation r, bool flipped, bool rtl) {
		glUniform4f(state.open_gl.ui_shader_d_rect_uniform, x, y, width, height);

		glBindTexture(GL_TEXTURE_2D, texture_handle);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mask_texture_handle);
		glActiveTexture(GL_TEXTURE0);

		GLuint subroutines[2] = { map_color_modification_to_index(enabled), parameters::use_mask };
		glUniform2ui(state.open_gl.ui_shader_subroutines_index_uniform, subroutines[0], subroutines[1]);
		//glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 2, subroutines); // must set all subroutines in one call

		//glBindVertexArray(state.open_gl.global_square_vao);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	}

	void render_progress_bar(sys::state const& state, color_modification enabled, float progress, float x, float y, float width, float height, GLuint left_texture_handle, GLuint right_texture_handle, ui::rotation r, bool flipped, bool rtl) {
		//glBindVertexArray(state.open_gl.global_square_vao);
		bind_vertices_by_rotation(state, r, flipped, rtl);

		glUniform4f(state.open_gl.ui_shader_d_rect_uniform, x, y, width, height);
		glUniform1f(state.open_gl.ui_shader_border_size_uniform, progress);

		glBindTexture(GL_TEXTURE_2D, left_texture_handle);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, right_texture_handle);
		glActiveTexture(GL_TEXTURE0);

		GLuint subroutines[2] = { map_color_modification_to_index(enabled), parameters::progress_bar };
		glUniform2ui(state.open_gl.ui_shader_subroutines_index_uniform, subroutines[0], subroutines[1]);
		//glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 2, subroutines); // must set all subroutines in one call
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	}

	void render_tinted_textured_rect(sys::state const& state, float x, float y, float width, float height, float r, float g, float b, GLuint texture_handle, ui::rotation rot, bool flipped, bool rtl) {
		//glBindVertexArray(state.open_gl.global_square_vao);

		bind_vertices_by_rotation(state, rot, flipped, rtl);

		glUniform3f(state.open_gl.ui_shader_inner_color_uniform, r, g, b);
		glUniform4f(state.open_gl.ui_shader_d_rect_uniform, x, y, width, height);
		glBindTexture(GL_TEXTURE_2D, texture_handle);

		GLuint subroutines[2] = { parameters::tint, parameters::no_filter };
		glUniform2ui(state.open_gl.ui_shader_subroutines_index_uniform, subroutines[0], subroutines[1]);
		//glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 2, subroutines); // must set all subroutines in one call

		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	}

	void render_tinted_subsprite(sys::state const& state, int frame, int total_frames, float x, float y, float width, float height, float r, float g, float b, GLuint texture_handle, ui::rotation rot, bool flipped, bool rtl) {
		//glBindVertexArray(state.open_gl.global_square_vao);

		bind_vertices_by_rotation(state, rot, flipped, rtl);

		auto const scale = 1.0f / static_cast<float>(total_frames);
		glUniform3f(state.open_gl.ui_shader_inner_color_uniform, static_cast<float>(frame) * scale, scale, 0.0f);
		glUniform4f(state.open_gl.ui_shader_subrect_uniform, r, g, b, 0);
		glUniform4f(state.open_gl.ui_shader_d_rect_uniform, x, y, width, height);
		glBindTexture(GL_TEXTURE_2D, texture_handle);

		GLuint subroutines[2] = { parameters::alternate_tint, parameters::sub_sprite };
		glUniform2ui(state.open_gl.ui_shader_subroutines_index_uniform, subroutines[0], subroutines[1]);
		//glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 2, subroutines); // must set all subroutines in one call

		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	}

	void render_subsprite(sys::state const& state, color_modification enabled, int frame, int total_frames, float x, float y, float width, float height, GLuint texture_handle, ui::rotation r, bool flipped, bool rtl) {
		//glBindVertexArray(state.open_gl.global_square_vao);

		bind_vertices_by_rotation(state, r, flipped, rtl);

		auto const scale = 1.0f / float(total_frames);
		glUniform3f(state.open_gl.ui_shader_inner_color_uniform, float(frame) * scale, scale, 0.0f);
		glUniform4f(state.open_gl.ui_shader_d_rect_uniform, x, y, width, height);
		glBindTexture(GL_TEXTURE_2D, texture_handle);

		GLuint subroutines[2] = { map_color_modification_to_index(enabled), parameters::sub_sprite };
		glUniform2ui(state.open_gl.ui_shader_subroutines_index_uniform, subroutines[0], subroutines[1]);
		//glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 2, subroutines); // must set all subroutines in one call

		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	}

	GLuint get_flag_texture_handle_from_tag(sys::state& state, const char tag[3]) {
		char ltag[3];
		ltag[0] = char(toupper(tag[0]));
		ltag[1] = char(toupper(tag[1]));
		ltag[2] = char(toupper(tag[2]));
		dcon::national_identity_id ident{};
		state.world.for_each_national_identity([&](dcon::national_identity_id id) {
			auto curr = nations::int_to_tag(state.world.national_identity_get_identifying_int(id));
			if(curr[0] == ltag[0] && curr[1] == ltag[1] && curr[2] == ltag[2]) {
				ident = id;
			}
		});
		if(!bool(ident)) {
			// QOL: We will print the text instead of displaying the flag, for ease of viewing invalid tags
			return 0;
		}
		auto fat_id = dcon::fatten(state.world, ident);
		auto nation = fat_id.get_nation_from_identity_holder();
		culture::flag_type flag_type = culture::flag_type{};
		if(bool(nation.id) && nation.get_owned_province_count() != 0) {
			flag_type = culture::get_current_flag_type(state, nation.id);
		} else {
			flag_type = culture::get_current_flag_type(state, ident);
		}
		return ogl::get_flag_handle(state, ident, flag_type);
	}

	bool display_tag_is_valid(sys::state& state, char tag[3]) {
		tag[0] = char(toupper(tag[0]));
		tag[1] = char(toupper(tag[1]));
		tag[2] = char(toupper(tag[2]));
		dcon::national_identity_id ident{};
		state.world.for_each_national_identity([&](dcon::national_identity_id id) {
			auto curr = nations::int_to_tag(state.world.national_identity_get_identifying_int(id));
			if(curr[0] == tag[0] && curr[1] == tag[1] && curr[2] == tag[2])
				ident = id;
		});
		return bool(ident);
	}

	void render_text_icon(sys::state& state, text::embedded_icon ico, float x, float baseline_y, float font_size, text::font& f, ogl::color_modification cmod) {
		float scale = 1.f;
		float icon_baseline = baseline_y + (f.internal_ascender / 64.f * font_size) - font_size;

		//glBindVertexArray(state.open_gl.global_square_vao);
		bind_vertices_by_rotation(state, ui::rotation::upright, false, false);
		switch(ico) {
		case text::embedded_icon::army:
			scale = 1.3f;
			glBindTexture(GL_TEXTURE_2D, state.open_gl.army_icon_tex);
			break;
		case text::embedded_icon::navy:
			glBindTexture(GL_TEXTURE_2D, state.open_gl.navy_icon_tex);
			scale = 1.3f;
			break;
		case text::embedded_icon::check:
			glBindTexture(GL_TEXTURE_2D, state.open_gl.checkmark_icon_tex);
			icon_baseline += font_size * 0.1f;
			break;
		case text::embedded_icon::xmark:
		{
			GLuint false_icon = (state.user_settings.color_blind_mode == sys::color_blind_mode::deutan || state.user_settings.color_blind_mode == sys::color_blind_mode::protan)
				? state.open_gl.color_blind_cross_icon_tex
				: state.open_gl.cross_icon_tex;
			glBindTexture(GL_TEXTURE_2D, false_icon);
			icon_baseline += font_size * 0.1f;
			break;
		}
		}

		GLuint icon_subroutines[2] = { map_color_modification_to_index(cmod), parameters::no_filter };
		glUniform2ui(state.open_gl.ui_shader_subroutines_index_uniform, icon_subroutines[0], icon_subroutines[1]);
		//glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 2, icon_subroutines); // must set all subroutines in one call
		glUniform4f(state.open_gl.ui_shader_d_rect_uniform, x, icon_baseline, scale * font_size, scale * font_size);
		glUniform4f(state.open_gl.ui_shader_subrect_uniform, 0.f, 1.f, 0.f, 1.f);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	}

	void render_text_flag(sys::state& state, text::embedded_flag ico, float x, float baseline_y, float font_size, text::font& f, ogl::color_modification cmod) {
		float icon_baseline = baseline_y + (f.internal_ascender / 64.f * font_size) - font_size;

		auto fat_id = dcon::fatten(state.world, ico.tag);
		auto nation = fat_id.get_nation_from_identity_holder();
		culture::flag_type flag_type = culture::flag_type{};
		if(bool(nation.id) && nation.get_owned_province_count() != 0) {
			flag_type = culture::get_current_flag_type(state, nation.id);
		} else {
			flag_type = culture::get_current_flag_type(state, ico.tag);
		}
		GLuint flag_texture_handle = ogl::get_flag_handle(state, ico.tag, flag_type);

		GLuint icon_subroutines[2] = { map_color_modification_to_index(cmod), parameters::no_filter };
		glUniform2ui(state.open_gl.ui_shader_subroutines_index_uniform, icon_subroutines[0], icon_subroutines[1]);
		//glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 2, icon_subroutines); // must set all subroutines in one call
		bind_vertices_by_rotation(state, ui::rotation::upright, false, false);
		glBindTexture(GL_TEXTURE_2D, flag_texture_handle);
		glUniform4f(state.open_gl.ui_shader_d_rect_uniform, x, icon_baseline + font_size * 0.15f, 1.5f * font_size * 0.9f, font_size * 0.9f);
		glUniform4f(state.open_gl.ui_shader_subrect_uniform, 0.f, 1.f, 0.f, 1.f);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	}


	void internal_text_render(sys::state& state, text::stored_glyphs const& txt, float x, float baseline_y, float size, text::font& f) {
		if(f.textures.empty())
			return; //edge case

		//glBindVertexArray(state.open_gl.global_square_vao);
		GLuint subroutines[2] = { map_color_modification_to_index(ogl::color_modification::none), parameters::filter };
		glUniform2ui(state.open_gl.ui_shader_subroutines_index_uniform, subroutines[0], subroutines[1]);
		unsigned int glyph_count = static_cast<unsigned int>(txt.glyph_info.size());
		for(unsigned int i = 0; i < glyph_count; i++) {
			hb_codepoint_t glyphid = txt.glyph_info[i].codepoint;
			auto gso = f.glyph_positions[glyphid];
			float x_advance = float(txt.glyph_info[i].x_advance) / (float((1 << 6) * text::magnification_factor));
			float x_offset = float(txt.glyph_info[i].x_offset) / (float((1 << 6) * text::magnification_factor)) + float(gso.x);
			float y_offset = float(gso.y) - float(txt.glyph_info[i].y_offset) / (float((1 << 6) * text::magnification_factor));
			glBindBuffer(GL_ARRAY_BUFFER, state.open_gl.sub_square_buffers[gso.texture_slot & 63]);
			assert(uint32_t(gso.texture_slot >> 6) < f.textures.size());
			assert(f.textures[gso.texture_slot >> 6]);
			glBindTexture(GL_TEXTURE_2D, f.textures[gso.texture_slot >> 6]);
			glUniform4f(state.open_gl.ui_shader_d_rect_uniform, x + x_offset * size / 64.f, baseline_y + y_offset * size / 64.f, size, size);
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			x += x_advance * size / 64.f;
			baseline_y -= (float(txt.glyph_info[i].y_advance) / (float((1 << 6) * text::magnification_factor))) * size / 64.f;
		}
	}

	void render_classic_text(sys::state& state, text::stored_glyphs const& txt, float x, float y, float size, color_modification enabled, color3f const& c, text::bm_font const& font, text::font& base_font) {
		std::string codepoints = "";
		for(uint32_t i = 0; i < uint32_t(txt.glyph_info.size()); i++) {
			codepoints.push_back(char(txt.glyph_info[i].codepoint));
		}
		uint32_t count = uint32_t(codepoints.length());

		float adv = 1.0f / font.width; // Font texture atlas spacing.
		//glBindVertexArray(state.open_gl.global_square_vao);
		bind_vertices_by_rotation(state, ui::rotation::upright, false, false);
		GLuint subroutines[2] = { map_color_modification_to_index(enabled), parameters::subsprite_b };
		glUniform2ui(state.open_gl.ui_shader_subroutines_index_uniform, subroutines[0], subroutines[1]);
		//glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 2, subroutines); // must set all subroutines in one call

		// Set Text Color, all one color for now.
		//------ FOR SCHOMBERT ------//
		// Every iteration of this loop draws one character of the string 'fmt'.
		//'texlst' contains information for each vertex of each rectangle for each character.
		// Every 4 elements in 'texlst' is one complete rectangle, and one character.
		//'texlst[i].texx' and 'texlst[i].texy' are the intended texture coordinates of a vertex on the texture.
		//'texlst[i].x' and 'texlst[i].y' are the coordinates of a vertex of the rendered rectangle in the window.
		// The color variables are unused currently.
		//
		// Spacing, kearning, etc. are already applied.
		// Scaling (unintentionally) is also applied (by whatever part of KatEngine scales the normal fonts).

		glBindTexture(GL_TEXTURE_2D, font.ftexid);
		glUniform3f(state.open_gl.ui_shader_inner_color_uniform, c.r, c.g, c.b);
		for(uint32_t i = 0; i < count; ++i) {
			uint8_t ch = uint8_t(codepoints[i]);
			if(i != 0 && i < count - 1 && ch == 0xC2 && uint8_t(codepoints[i + 1]) == 0xA3) {
				ch = 0xA3;
				i++;
			} else if(ch == 0xA4) {
				ch = 0xA3;
			}
			auto const& f = font.chars[ch];
			float CurX = x + f.x_offset;
			float CurY = y + f.y_offset;
			glUniform4f(state.open_gl.ui_shader_d_rect_uniform, CurX, CurY, float(f.width), float(f.height));
			glUniform4f(state.open_gl.ui_shader_subrect_uniform,
				float(f.x) / float(font.width) /* x offset */,
				float(f.width) / float(font.width) /* x width */,
				float(f.y) / float(font.width) /* y offset */,
				float(f.height) / float(font.width) /* y height */
			);
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			x += f.x_advance;
		}
	}

	void render_new_text(sys::state& state, text::stored_glyphs const& txt, color_modification enabled, float x, float y, float size, color3f const& c, text::font& f) {
		glUniform3f(state.open_gl.ui_shader_inner_color_uniform, c.r, c.g, c.b);
		glUniform1f(state.open_gl.ui_shader_border_size_uniform, 0.08f * 16.0f / size);
		internal_text_render(state, txt, x, y + size, size, f);
	}

	void render_text(sys::state& state, text::stored_glyphs const& txt, color_modification enabled, float x, float y, color3f const& c, uint16_t font_id) {
		auto& font = state.font_collection.get_font(state, text::font_index_from_font_id(state, font_id));
		if(state.user_settings.use_classic_fonts) {
			render_classic_text(state, txt, x, y, float(text::size_from_font_id(font_id)), enabled, c, text::get_bm_font(state, font_id), font);
			return;
		}
		render_new_text(state, txt, enabled, x, y, float(text::size_from_font_id(font_id)), c, font);
	}

	void lines::set_y(float* v) {
		for(uint32_t i = 0; i < count; ++i) {
			buffer[i * 4 + 0] = float(i) / float(count - 1); //pos x
			buffer[i * 4 + 1] = 1.0f - v[i]; //pos y
			buffer[i * 4 + 2] = 0.5f; //tex x
			buffer[i * 4 + 3] = v[i]; //tex y
		}
		pending_data_update = true;
	}

	void lines::set_default_y() {
		for(uint32_t i = 0; i < count; ++i) {
			buffer[i * 4 + 0] = float(i) / float(count - 1); //pos x
			buffer[i * 4 + 1] = 0.5f; //pos y
			buffer[i * 4 + 2] = 0.5f; //tex x
			buffer[i * 4 + 3] = 0.5f; //tex y
		}
		pending_data_update = true;
	}

	void lines::bind_buffer() {
		if(buffer && pending_data_update) {
			if(buffer_handle == 0) {
				glGenBuffers(1, &buffer_handle);
				glGenVertexArrays(1, &array_handle);
			}
			std::vector<GLushort> tmp_buffer(count * 4);
			for(uint32_t i = 0; i < count; i++) {
				tmp_buffer[i * 4 + 0] = GLushort(buffer[i * 4 + 0] * 65535.f);
				tmp_buffer[i * 4 + 1] = GLushort(buffer[i * 4 + 1] * 65535.f);
				tmp_buffer[i * 4 + 2] = GLushort(buffer[i * 4 + 2] * 65535.f);
				tmp_buffer[i * 4 + 3] = GLushort(buffer[i * 4 + 3] * 65535.f);
			}
			glBindBuffer(GL_ARRAY_BUFFER, buffer_handle);
			glBufferData(GL_ARRAY_BUFFER, sizeof(GLushort) * 4 * count, tmp_buffer.data(), GL_DYNAMIC_DRAW);
			pending_data_update = false;
			//
			glBindVertexArray(array_handle);
			glVertexAttribPointer(0, 2, GL_UNSIGNED_SHORT, GL_TRUE, sizeof(GLushort) * 4, 0); // position
			glVertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_TRUE, sizeof(GLushort) * 4, (const void*)(sizeof(GLushort) * 2)); // texture coordinates
			glEnableVertexAttribArray(0); // position
			glEnableVertexAttribArray(1); // texture coordinates
		}
		glBindVertexArray(array_handle);
		glBindBuffer(GL_ARRAY_BUFFER, buffer_handle);
	}

	bool msaa_enabled(sys::state const& state) {
		return state.open_gl.msaa_enabled;
	}

	image load_stb_image(simple_fs::file& file) {
		int32_t file_channels = 4;
		int32_t size_x = 0;
		int32_t size_y = 0;
		auto content = simple_fs::view_contents(file);
		auto data = stbi_load_from_memory(reinterpret_cast<uint8_t const*>(content.data), int32_t(content.file_size), &size_x, &size_y, &file_channels, 4);
		return image(data, size_x, size_y, 4);
	}

	GLuint make_gl_texture(uint8_t* data, uint32_t size_x, uint32_t size_y, uint32_t channels) {
		GLuint texture_handle;
		glGenTextures(1, &texture_handle);
		const GLuint internalformats[] = { GL_R8, GL_RG8, GL_RGB8, GL_RGBA8 };
		const GLuint formats[] = { GL_RED, GL_RG, GL_RGB, GL_RGBA };
		if(texture_handle) {
			glBindTexture(GL_TEXTURE_2D, texture_handle);
			glTexImage2D(GL_TEXTURE_2D, 0, internalformats[channels - 1], size_x, size_y, 0, formats[channels - 1], GL_UNSIGNED_BYTE, data);
		}
		return texture_handle;
	}

	GLuint make_gl_texture(simple_fs::directory const& dir, native_string_view file_name) {
		if(auto f = open_file(dir, file_name); f) {
			auto image = load_stb_image(*f);
			return make_gl_texture(image.data, image.size_x, image.size_y, image.channels);
		}
		return 0;
	}

	void set_gltex_parameters(GLuint texture_handle, GLuint texture_type, GLuint filter, GLuint wrap) {
		glBindTexture(texture_type, texture_handle);
		if(filter == GL_LINEAR_MIPMAP_NEAREST || filter == GL_LINEAR_MIPMAP_LINEAR) {
			glTexParameteri(texture_type, GL_TEXTURE_MIN_FILTER, filter);
			glTexParameteri(texture_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glGenerateMipmap(texture_type);
		} else {
			glTexParameteri(texture_type, GL_TEXTURE_MAG_FILTER, filter);
			glTexParameteri(texture_type, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}
		glTexParameteri(texture_type, GL_TEXTURE_WRAP_S, wrap);
		glTexParameteri(texture_type, GL_TEXTURE_WRAP_T, wrap);
		glBindTexture(texture_type, 0);
	}

	GLuint load_texture_array_from_file(simple_fs::file& file, int32_t tiles_x, int32_t tiles_y) {
		auto image = load_stb_image(file);

		GLuint texture_handle = 0;
		glGenTextures(1, &texture_handle);
		if(texture_handle) {
			glBindTexture(GL_TEXTURE_2D_ARRAY, texture_handle);
			//
			size_t p_dx = image.size_x / tiles_x; // Pixels of each tile in x
			size_t p_dy = image.size_y / tiles_y; // Pixels of each tile in y
			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, GLsizei(p_dx), GLsizei(p_dy), GLsizei(tiles_x * tiles_y), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, image.size_x);
			glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, image.size_y);
			for(int32_t x = 0; x < tiles_x; x++) {
				for(int32_t y = 0; y < tiles_y; y++) {
					glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, GLint(x * tiles_x + y), GLsizei(p_dx), GLsizei(p_dy), 1, GL_RGBA, GL_UNSIGNED_BYTE, ((uint32_t const*)image.data) + (x * p_dy * image.size_x + y * p_dx));
				}
			}
			set_gltex_parameters(texture_handle, GL_TEXTURE_2D_ARRAY, GL_LINEAR_MIPMAP_NEAREST, GL_REPEAT);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
			glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
		}
		return texture_handle;
	}

} // namespace ogl
