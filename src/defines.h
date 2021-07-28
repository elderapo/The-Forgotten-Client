/*
  The Forgotten Client
  Copyright (C) 2020 Saiyans King

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#ifndef __FILE_DEFINES_h_
#define __FILE_DEFINES_h_

#include <SDL2/SDL.h>

#define FPSinterval 1000
typedef struct {
	float rateticks;
	Uint32 framecount;
	Uint32 baseticks;
	Uint32 lastticks;
	Uint32 rate;
} FPSmanager;
extern FPSmanager g_fpsmanager;
void SDL_setFramerate(FPSmanager * manager, Uint32 rate);
void setCursor(Sint32 cursor);

struct KeyRepeat
{
	KeyRepeat() : timestamp(0), delay(200), interval(50) {}

	Uint32 timestamp;
	Uint32 delay;
	Uint32 interval;
	SDL_Keycode key;
	bool firstTime;
};

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define MAKE_RGBA_COLOR(r, g, b, a) ((DWORD) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b)))
#else
#define MAKE_RGBA_COLOR(r, g, b, a) ((DWORD) (((b) << 24) | ((g) << 16) | ((r) << 8) | (a)))
#endif
#define GET_SAFE_PROTOCOLGAME ((g_connection && g_connection->getProtocol()) ? g_connection->getProtocol()->getProtocolGame() : NULL)

#if defined(_M_ARM) || defined(__arm__) || defined(_ARM) || defined(__arm)
#define __USE_NEON__ 1
#endif

#if (defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))) || defined(__amd64__) || defined(__x86_64__) || defined(__i386__) || defined(__i386) || defined(i386)

#ifdef __has_include

#if __has_include(<xmmintrin.h>)
#define __USE_SSE__ 1
#endif
#if __has_include(<immintrin.h>)
#define __USE_FMA3__ 1
#endif
#if __has_include(<ammintrin.h>)
#define __USE_FMA4__ 1
#endif
#if __has_include(<emmintrin.h>)
#define __USE_SSE2__ 1
#endif
#if __has_include(<pmmintrin.h>)
#define __USE_SSE3__ 1
#endif
#if __has_include(<tmmintrin.h>)
#define __USE_SSSE3__ 1
#endif
#if __has_include(<smmintrin.h>)
#define __USE_SSE4_1__ 1
#endif
#if __has_include(<nmmintrin.h>)
#define __USE_SSE4_2__ 1
#endif
#if __has_include(<immintrin.h>)
#define __USE_AVX__ 1
#define __USE_AVX2__ 1
#endif

#else

#define __USE_SSE__ 1
#define __USE_FMA3__ 1
#define __USE_FMA4__ 1
#define __USE_SSE2__ 1
#define __USE_SSE3__ 1
#define __USE_SSSE3__ 1
#define __USE_SSE4_1__ 1
#define __USE_SSE4_2__ 1
#define __USE_AVX__ 1
#define __USE_AVX2__ 1

#endif

#endif

#if defined(__USE_SSE__)
#include <xmmintrin.h>
#endif
#if defined(__USE_FMA3__)
#include <immintrin.h>
#endif
#if defined(__USE_FMA4__)
#include <ammintrin.h>
#endif
#if defined(__USE_SSE2__)
#include <emmintrin.h>
#endif
#if defined(__USE_SSE3__)
#include <pmmintrin.h>
#endif
#if defined(__USE_SSSE3__)
#include <tmmintrin.h>
#endif
#if defined(__USE_SSE4_1__)
#include <smmintrin.h>
#endif
#if defined(__USE_SSE4_2__)
#include <nmmintrin.h>
#endif
#if (defined(__USE_AVX__) || defined(__USE_AVX2__)) && !defined(__linux__)
#include <immintrin.h>
#endif
#if (defined(__USE_AVX__) || defined(__USE_AVX2__)) && defined(__linux__)
#include <x86intrin.h>
#endif
#if defined(__USE_NEON__)
#include <arm_neon.h>
#endif

#if SDL_VIDEO_DRIVER_WINDOWS && SIZEOF_VOIDP == 4
//For some reason 64-bit version of ddraw.dll don't have the direct3d interface
#define SDL_VIDEO_RENDER_DDRAW 1
#endif

#if defined(__WIN32__)
#define PATH_PLATFORM_SLASH '\\'
#else
#define PATH_PLATFORM_SLASH '/'
#endif

#if defined(__WIN32__)
  #include <windows.h>
#else
typedef unsigned int DWORD;
#endif

#include <time.h>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <list>
#include <vector>
#include <memory>
#include <algorithm>
#include <sstream>
#include "stringExtend.h"
#include <bitset>

#include "const.h"
#include "gui_const.h"
#include "structures.h"
#include "circular_buffer.h"
#include "GUI/GUI_UTIL.h"
#include "util.h"
#include "Rect.h"
#include "robin_hood.h"

extern char g_buffer[4096];
extern Uint32 g_clientVersion;
extern std::string g_basePath;
extern std::string g_prefPath;
extern std::string g_mapPath;
extern std::string g_datPath;
extern std::string g_picPath;
extern std::string g_sprPath;

#endif /* __FILE_DEFINES_h_ */
