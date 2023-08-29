#pragma once

#ifdef _WIN32

#include <windows.h>
#include <GL/glew.h>
#include <GL/wglew.h>
#include <GL/gl.h>
#include <GL/glu.h>

#elif defined(__ANDROID__)

#include <EGL/egl.h>
#include <GLES3/gl3.h>

#elif defined(__APPLE__)

#include <TargetConditionals.h>
#define GL_SILENCE_DEPRECATION

#if TARGET_OS_IPHONE

//iOS and Simulator
#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>

#else

//OSX
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>
#include <OpenGL/glu.h>
#include <OpenGL/OpenGL.h>

#endif

#elif defined(__linux__) || defined(__FreeBSD__)

#if defined(USE_GLEW)
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#elif defined(GLES_COMPATIBILITY)
#include <EGL/egl.h>
#include <GLES3/gl3.h>
// egl can reference Xlib.h->X.h which defines None, which causes issues with qt (qgroupaction) down the line for using None in enum
#undef None
#endif

#elif defined(__EMSCRIPTEN__)

#include <GLES3/gl3.h>

#endif

#include "maybe_unused.h"

#ifdef _DEBUG
#define CHECKGLERROR() { FRAMEWORK_MAYBE_UNUSED auto errorCode = glGetError(); assert(errorCode == GL_NO_ERROR); }
#else
#define CHECKGLERROR()
#endif
