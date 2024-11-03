#include "glad.h"
#include <string>

namespace ogl {

	void create_opengl_context(sys::state& state) {
		assert(state.win_ptr && state.win_ptr->window);
		glfwMakeContextCurrent(state.win_ptr->window);
		glfwSwapInterval(1); // Vsync option
		if(gladLoadGL() == 0) {
			window::emit_error_message("GLAD failed to initialize", true);
		}
	}

void shutdown_opengl(sys::state& state) { }
} // namespace ogl
