#include "glad.h"
#include "system_state.hpp"
#include "opengl_wrapper.hpp"

#include <GLFW/glfw3.h>
#include <string>

namespace ogl {
	void create_opengl_context(sys::state& state) {
		assert(state.win_ptr && state.win_ptr->window);
		glfwMakeContextCurrent(state.win_ptr->window);

#ifdef __APPLE__
		/* We need to explicitly ask for a 3.2 context on OS X */
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

		glfwSwapInterval(1); // Vsync option
		if(gladLoadGL() == 0) {
			window::emit_error_message("GLAD failed to initialize", true);
		}
	}

	void shutdown_opengl(sys::state& state) {
		state.map_state.map_data.clear_opengl_objects();
		for(auto& f : state.font_collection.bitmap_fonts) {
			f.second.clear_opengl_objects();
		}
	}
} // namespace ogl
