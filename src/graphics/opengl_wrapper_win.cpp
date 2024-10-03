#include "wglew.h"

#include <cassert>

#ifndef UNICODE
#define UNICODE
#endif
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include "Windows.h"

namespace ogl {

	void create_opengl_context(sys::state& state) {
		assert(state.win_ptr && state.win_ptr->hwnd && !state.open_gl.context);

		HDC window_dc = state.win_ptr->opengl_window_dc;

		PIXELFORMATDESCRIPTOR pfd;
		ZeroMemory(&pfd, sizeof(pfd));
		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;
		pfd.cDepthBits = 24;
		pfd.cStencilBits = 8;
		pfd.iLayerType = PFD_MAIN_PLANE;
		int const pixel_format = ChoosePixelFormat(window_dc, &pfd);
		SetPixelFormat(window_dc, pixel_format, &pfd);

		auto handle_to_ogl_dc = wglCreateContext(window_dc);
		wglMakeCurrent(window_dc, handle_to_ogl_dc);
		if(glewInit() != GLEW_OK) {
			window::emit_error_message("GLEW failed to initialize", true);
		}

		bool v = false;
		if(v && wglewIsSupported("WGL_ARB_create_context")) {
			// Explicitly request for OpenGL 3.0
			static const int attribs_3_0[] = {
				WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
				WGL_CONTEXT_MINOR_VERSION_ARB, 0,
				WGL_CONTEXT_FLAGS_ARB,
#ifndef NDEBUG
				WGL_CONTEXT_DEBUG_BIT_ARB |
#endif
				0,
				WGL_CONTEXT_PROFILE_MASK_ARB,
				WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
				0
			};
			state.open_gl.context = wglCreateContextAttribsARB(window_dc, nullptr, attribs_3_0);
		} else {
			state.open_gl.context = wglCreateContext(window_dc);
		}

		if(state.open_gl.context == nullptr) {
			window::emit_error_message("Unable to create WGL context", true);
		}

		wglMakeCurrent(window_dc, HGLRC(state.open_gl.context));
		wglDeleteContext(handle_to_ogl_dc);

		glDebugMessageCallback(debug_callback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_OTHER, GL_DEBUG_SEVERITY_LOW, 0, nullptr, GL_FALSE);
		//
		std::string msg;
		msg += "GL Version: " + std::string(reinterpret_cast<char const*>(glGetString(GL_VERSION))) + "\n";
		msg += "GL Shading version: " + std::string(reinterpret_cast<char const*>(glGetString(GL_SHADING_LANGUAGE_VERSION))) + "\n";
		OutputDebugStringA(msg.c_str());

		if(wglewIsSupported("WGL_EXT_swap_control_tear") == 1) {
			OutputDebugStringA("WGL_EXT_swap_control_tear is on");
			wglSwapIntervalEXT(-1);
		} else if(wglewIsSupported("WGL_EXT_swap_control") == 1) {
			OutputDebugStringA("WGL_EXT_swap_control is on");
			wglSwapIntervalEXT(1);
		}
	}

	void shutdown_opengl(sys::state& state) {
		assert(state.win_ptr && state.win_ptr->hwnd && state.open_gl.context);
		wglMakeCurrent(state.win_ptr->opengl_window_dc, nullptr);
		wglDeleteContext(HGLRC(state.open_gl.context));
		state.open_gl.context = nullptr;
	}
} // namespace ogl
