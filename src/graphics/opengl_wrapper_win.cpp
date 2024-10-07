#include "glad.h"

#include <cassert>

#ifndef UNICODE
#define UNICODE
#endif
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include "Windows.h"
//#include "wglew.h"

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

		auto gl_lib = LoadLibraryW(L"opengl32.dll");
		if(gl_lib) {
			state.open_gl.context = ((decltype(&wglCreateContext))GetProcAddress(gl_lib, "wglCreateContext"))(window_dc);
			if(state.open_gl.context == NULL) {
				window::emit_error_message("Unable to create WGL context", true);
			}
			((decltype(&wglMakeCurrent))GetProcAddress(gl_lib, "wglMakeCurrent"))(window_dc, HGLRC(state.open_gl.context));
			if(gladLoadGL() == 0) {
				window::emit_error_message("GLAD failed to initialize", true);
			}
#if 0
			glDebugMessageCallback(debug_callback, nullptr);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
			glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_OTHER, GL_DEBUG_SEVERITY_LOW, 0, nullptr, GL_FALSE);
#endif
			//
			std::string msg;
			msg += "GL Version: " + std::string(reinterpret_cast<char const*>(glGetString(GL_VERSION))) + "\n";
			msg += "GL Shading version: " + std::string(reinterpret_cast<char const*>(glGetString(GL_SHADING_LANGUAGE_VERSION))) + "\n";
			reports::write_debug(msg.c_str());
			/*
			if(wglewIsSupported("WGL_EXT_swap_control_tear") == 1) {
				reports::write_debug("WGL_EXT_swap_control_tear is on\n");
				wglSwapIntervalEXT(-1);
			} else if(wglewIsSupported("WGL_EXT_swap_control") == 1) {
				reports::write_debug("WGL_EXT_swap_control is on\n");
				wglSwapIntervalEXT(1);
			}
			*/
			FreeLibrary(gl_lib);
		} else {
			window::emit_error_message("Opengl32.dll is missing", true);
		}
	}

	void shutdown_opengl(sys::state& state) {
		assert(state.win_ptr && state.win_ptr->hwnd && state.open_gl.context);
		auto gl_lib = LoadLibraryW(L"opengl32.dll");
		if(gl_lib) {
			((decltype(&wglMakeCurrent))GetProcAddress(gl_lib, "wglMakeCurrent"))(state.win_ptr->opengl_window_dc, NULL);
			((decltype(&wglDeleteContext))GetProcAddress(gl_lib, "wglDeleteContext"))(HGLRC(state.open_gl.context));
			FreeLibrary(gl_lib);
		}
		state.open_gl.context = nullptr;
	}
} // namespace ogl
