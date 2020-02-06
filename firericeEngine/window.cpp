// Copyright 2015 Red Blob Games <redblobgames@gmail.com>
// License: Apache v2.0 <http://www.apache.org/licenses/LICENSE-2.0.html>

#include "window.h"

#include <SDL.h>
#include "glwrappers.h"

#include <vector>


// NOTE: I need this implementation to be in a cpp file and not an h
// file so that the compiler knows where to put the vtable
IRenderLayer::~IRenderLayer() {}


struct WindowImpl {
  SDL_Window* window;
  bool context_initialized;
  GlContext* context;
  std::vector<IRenderLayer*> layers;
  
  WindowImpl();
  ~WindowImpl();
};

int Window::FRAME = 0;

Window::Window(int width_, int height_)
  :visible(true),
	width(width_),
	height(height_),
   self(new WindowImpl())
{
	HandleResize();
}

Window::~Window() {}


void Window::AddLayer(IRenderLayer* layer) {
  self->layers.push_back(layer);
}


void Window::HandleResize() {
  self->context_initialized = false;
  SDL_GL_GetDrawableSize(self->window, &width, &height);
  glViewport(0, 0, width, height);
}


void Window::Render() {
	//if (visible) {

	if (true) {
    glClear(GL_COLOR_BUFFER_BIT);
    for (auto layer : self->layers) {
      layer->Render(self->window, !self->context_initialized);
    }
    self->context_initialized = true;
    SDL_GL_SwapWindow(self->window);
    FRAME++;
  }
}


void Window::ProcessEvent(SDL_Event* event) {
  if (event->type == SDL_WINDOWEVENT) {
    switch (event->window.event) {
    case SDL_WINDOWEVENT_SHOWN: { visible = true; break; }
    case SDL_WINDOWEVENT_HIDDEN: { visible = false; break; }
    case SDL_WINDOWEVENT_SIZE_CHANGED: { HandleResize(); break; }
    }
  }

  for (auto layer : self->layers) {
    layer->ProcessEvent(event);
  }
}


WindowImpl::WindowImpl()
{
	window = SDL_CreateWindow("Hello World",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		800,
		600,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN
	);
	
	context_initialized = false;


	////const char* glsl_version = "#version 100";
	////const char* glsl_version = "#version 300 es";
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

	//// Create window with graphics context
	//SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	//SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	//SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	context = new GlContext(window);
	

#ifndef __EMSCRIPTEN__
	if (gl3wInit()) {
		std::cout << "failed to init GL3W" << std::endl;
		SDL_GL_DeleteContext(window);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return;
	}
#endif

  glClearColor(1.0, 1.0, 1.0, 1.0);
}

WindowImpl::~WindowImpl() {
  SDL_DestroyWindow(window);
}
