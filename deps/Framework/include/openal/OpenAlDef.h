#ifndef _OPENALDEF_H_
#define _OPENALDEF_H_

#ifdef _WIN32
#include <al.h>
#include <alc.h>
#elif defined(__linux__) || defined(__FreeBSD__) || defined(__EMSCRIPTEN__)
#include <AL/al.h>
#include <AL/alc.h>
#else
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#endif

#endif
