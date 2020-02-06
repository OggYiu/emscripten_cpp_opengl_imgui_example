// Copyright 2015 Red Blob Games <redblobgames@gmail.com>
// License: Apache v2.0 <http://www.apache.org/licenses/LICENSE-2.0.html>

#include "glwrappers.h"
#include "common.h"

// Exit the program with an error message
void ShowSDLError(const char* label)
{
	std::cout << label << " failed : " << SDL_GetError() << std::endl;
}

void GLERRORS(const char* label) {
#ifndef __EMSCRIPTEN__
  while (true) {
    GLenum err = glGetError();
    if (err == GL_NO_ERROR) { break; }
    std::cerr << label << " glGetError returned " << err << std::endl;
  }
#endif
}

SDL_Surface* CreateRGBASurface(int width, int height) {
  SDL_Surface* surface = SDL_CreateRGBSurface
    (0, width, height, 32,
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
     0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff
#else
     0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000
#endif
     );
  if (surface == nullptr) { ShowSDLError("SDL_CreateRGBSurface"); }
  return surface;
}


ShaderProgram::ShaderProgram(const char* vertex_shader, const char* fragment_shader) {
  id = glCreateProgram();
  if (id == 0) { ShowSDLError("glCreateProgram"); }

#ifdef GL_ES_VERSION_2_0
  // WebGL requires precision specifiers but OpenGL 2.1 disallows
  // them, so I define the shader without it and then add it here.
  std::string new_fragment_shader = "precision mediump float;\n";
  new_fragment_shader += fragment_shader;
  fragment_shader = new_fragment_shader.c_str();
#endif
  
  AttachShader(GL_VERTEX_SHADER, vertex_shader);
  AttachShader(GL_FRAGMENT_SHADER, fragment_shader);
  glLinkProgram(id);
  
  GLint link_status;
  glGetProgramiv(id, GL_LINK_STATUS, &link_status);
  if (!link_status) {
    GLint log_length;
    glGetProgramiv(id, GL_INFO_LOG_LENGTH, &log_length);
    GLchar log[1024];
    glGetProgramInfoLog(id, 1024, nullptr, log);
    std::cerr << log << std::endl;
	ShowSDLError("link shaders");
  }
}

ShaderProgram::~ShaderProgram() {
  glDeleteProgram(id);
}

void ShaderProgram::AttachShader(GLenum type, const GLchar* source) {
  GLuint shader_id = glCreateShader(type);
  if (shader_id == 0) { ShowSDLError("load shader"); }
           
  glShaderSource(shader_id, 1, &source, nullptr);
  glCompileShader(shader_id);

  GLint compile_status;
  glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compile_status);
  if (!compile_status) {
    GLint log_length;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);
    GLchar log[1024];
    glGetShaderInfoLog(shader_id, 1024, nullptr, log);
    std::cerr << log << std::endl;
	ShowSDLError("compile shader");
  }

  glAttachShader(id, shader_id);
  glDeleteShader(shader_id);
  GLERRORS("AttachShader()");
}


Texture::Texture(SDL_Surface* surface) {
  glGenTextures(1, &id);
  if (surface != nullptr) {
    CopyFromSurface(surface);
  }
}

void Texture::CopyFromPixels(int width, int height,
                             GLenum format, void* pixels)
{
  glBindTexture(GL_TEXTURE_2D, id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // NOTE: these parameters will allow non-power-of-two texture sizes
  // in WebGL, which we want to use for underlay and overlay images.
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
  GLERRORS("Texture creation");
}


void Texture::CopyFromSurface(SDL_Surface* surface) {
  CopyFromPixels(surface->w, surface->h,
                 surface->format->BytesPerPixel == 1? GL_ALPHA
                 : surface->format->BytesPerPixel == 3? GL_RGB
                 : GL_RGBA /* TODO: check for other formats */,
                 surface->pixels);
}

Texture::~Texture() {
  glDeleteTextures(1, &id);
}


VertexBuffer::VertexBuffer() {
  glGenBuffers(1, &id);
}

VertexBuffer::~VertexBuffer() {
  glDeleteBuffers(1, &id);
}


GlContext::GlContext(SDL_Window* window) {

	// Create OpenGLES 2 context on SDL window
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	//SDL_GL_SetSwapInterval(1);
	//SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	//SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

#ifdef __EMSCRIPTEN__
	//const char* glsl_version = "#version 100";
	//const char* glsl_version = "#version 300 es";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

	// Create window with graphics context
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
#endif

  id = SDL_GL_CreateContext(window);
  if (id == nullptr) { ShowSDLError("SDL_GL_CreateContext"); }
}

GlContext::~GlContext() {
  SDL_GL_DeleteContext(id);
}
