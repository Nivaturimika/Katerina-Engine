#include "glad.h"

#include <cassert>

#ifndef UNICODE
#define UNICODE
#endif
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include "Windows.h"
//#include "wglew.h"
#include "reports.hpp"

namespace ogl {
	void create_opengl_context(sys::state& state) {
		assert(state.win_ptr && state.win_ptr->hwnd && !state.open_gl.context);

		HDC window_dc = state.win_ptr->opengl_window_dc;

		bool has_pfd_set = false;
		auto const pfd_count = DescribePixelFormat(window_dc, 1, sizeof(PIXELFORMATDESCRIPTOR), NULL);
		for(uint32_t i = 0; i < uint32_t(pfd_count); i++) {
			PIXELFORMATDESCRIPTOR pfd;
			if(!DescribePixelFormat(window_dc, i + 1, sizeof(PIXELFORMATDESCRIPTOR), &pfd)) {
				reports::write_debug("Unable to describe PixelFormat " + std::to_string(i) + "\n");
				continue;
			}
			if((pfd.dwFlags & PFD_DRAW_TO_WINDOW) != 0
			&& (pfd.dwFlags & PFD_SUPPORT_OPENGL) != 0
			&& (pfd.dwFlags & PFD_DOUBLEBUFFER) != 0
			&& pfd.iPixelType == PFD_TYPE_RGBA
			&& pfd.cDepthBits >= 8
			&& pfd.cStencilBits >= 8
			&& pfd.cColorBits >= 24) {
				reports::write_debug("Found usable pixel format #" + std::to_string(i) + "\n");
				reports::write_debug("Stencil=" + std::to_string(pfd.cStencilBits) + ",ColorDepth=" + std::to_string(pfd.cColorBits) + ",AccumBits=" + std::to_string(pfd.cAccumBits) + "\n");
				if(SetPixelFormat(window_dc, i + 1, &pfd)) {
					has_pfd_set = true;
					break;
				} else {
					reports::write_debug("Unable to set a pixel format: " + std::to_string(GetLastError()) + "\n");
				}
			}
		}
		if(!has_pfd_set) {
			reports::write_debug("Using default PFD as fallback\n");
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
			if(!SetPixelFormat(window_dc, pixel_format, &pfd)) {
				reports::write_debug("Unable to set a pixel format: " + std::to_string(GetLastError()) + "\n");
			}
		}

		auto gl_lib = LoadLibraryW(L"opengl32.dll");
		if(gl_lib) {
			state.open_gl.context = ((decltype(&wglCreateContext))GetProcAddress(gl_lib, "wglCreateContext"))(window_dc);
			if(state.open_gl.context == NULL) {
				window::emit_error_message("Unable to create WGL context" + std::to_string(GetLastError()), true);
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
			reports::write_debug("GL Version: " + std::string(reinterpret_cast<char const*>(glGetString(GL_VERSION))) + "\n");
			reports::write_debug("GL Shading version: " + std::string(reinterpret_cast<char const*>(glGetString(GL_SHADING_LANGUAGE_VERSION))) + "\n");

			// Enable vsync
			BOOL (*_wglSwapIntervalEXT)(int) = (BOOL(*)(int))(((decltype(&wglGetProcAddress))GetProcAddress(gl_lib, "wglGetProcAddress"))("wglSwapIntervalEXT"));
			if(_wglSwapIntervalEXT) {
				reports::write_debug("WGL_EXT_swap_control is on\n");
				_wglSwapIntervalEXT(1);
			}

			FreeLibrary(gl_lib);
		} else {
			window::emit_error_message("Opengl32.dll is missing", true);
		}
	}

	void shutdown_opengl(sys::state& state) {
		assert(state.win_ptr && state.win_ptr->hwnd && state.open_gl.context);

		state.map_state.map_data.clear_opengl_objects();
		for(auto& f : state.font_collection.bitmap_fonts) {
			f.second.clear_opengl_objects();
		}

		auto gl_lib = LoadLibraryW(L"opengl32.dll");
		if(gl_lib) {
			((decltype(&wglMakeCurrent))GetProcAddress(gl_lib, "wglMakeCurrent"))(state.win_ptr->opengl_window_dc, NULL);
			((decltype(&wglDeleteContext))GetProcAddress(gl_lib, "wglDeleteContext"))(HGLRC(state.open_gl.context));
			FreeLibrary(gl_lib);
		}
		state.open_gl.context = nullptr;
	}
} // namespace ogl
