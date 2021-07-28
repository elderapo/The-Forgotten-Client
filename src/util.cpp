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

#include "util.h"
#include "engine.h"

#include <random>
#ifdef __WIN32__
#include <Shellapi.h>
#endif

LPUTIL_Faststrstr UTIL_Faststrstr;
LPUTIL_FastCopy UTIL_FastCopy;
LPXTEA_DECRYPT XTEA_decrypt;
LPXTEA_ENCRYPT XTEA_encrypt;
LPADLER32CHECKSUM adler32Checksum;

extern Engine g_engine;

static const Uint8 outfitColorTable[][3] =
{
	{0xFF, 0xFF, 0xFF}, {0xFF, 0xD5, 0xBF}, {0xFF, 0xEA, 0xBF}, {0xFF, 0xFF, 0xBF}, {0xEA, 0xFF, 0xBF}, {0xD5, 0xFF, 0xBF},
	{0xBF, 0xFF, 0xBF}, {0xBF, 0xFF, 0xD5}, {0xBF, 0xFF, 0xEA}, {0xBF, 0xFF, 0xFF}, {0xBF, 0xEA, 0xFF}, {0xBF, 0xD5, 0xFF},
	{0xBF, 0xBF, 0xFF}, {0xD5, 0xBF, 0xFF}, {0xEA, 0xBF, 0xFF}, {0xFF, 0xBF, 0xFF}, {0xFF, 0xBF, 0xEA}, {0xFF, 0xBF, 0xD5},
	{0xFF, 0xBF, 0xBF}, {0xDB, 0xDB, 0xDB}, {0xBF, 0x9F, 0x8F}, {0xBF, 0xAF, 0x8F}, {0xBF, 0xBF, 0x8F}, {0xAF, 0xBF, 0x8F},
	{0x9F, 0xBF, 0x8F}, {0x8F, 0xBF, 0x8F}, {0x8F, 0xBF, 0x9F}, {0x8F, 0xBF, 0xAF}, {0x8F, 0xBF, 0xBF}, {0x8F, 0xAF, 0xBF},
	{0x8F, 0x9F, 0xBF}, {0x8F, 0x8F, 0xBF}, {0x9F, 0x8F, 0xBF}, {0xAF, 0x8F, 0xBF}, {0xBF, 0x8F, 0xBF}, {0xBF, 0x8F, 0xAF},
	{0xBF, 0x8F, 0x9F}, {0xBF, 0x8F, 0x8F}, {0xB6, 0xB6, 0xB6}, {0xBF, 0x7F, 0x5F}, {0xBF, 0x9F, 0x5F}, {0xBF, 0xBF, 0x5F},
	{0x9F, 0xBF, 0x5F}, {0x7F, 0xBF, 0x5F}, {0x5F, 0xBF, 0x5F}, {0x5F, 0xBF, 0x7F}, {0x5F, 0xBF, 0x9F}, {0x5F, 0xBF, 0xBF},
	{0x5F, 0x9F, 0xBF}, {0x5F, 0x7F, 0xBF}, {0x5F, 0x5F, 0xBF}, {0x7F, 0x5F, 0xBF}, {0x9F, 0x5F, 0xBF}, {0xBF, 0x5F, 0xBF},
	{0xBF, 0x5F, 0x9F}, {0xBF, 0x5F, 0x7F}, {0xBF, 0x5F, 0x5F}, {0x92, 0x92, 0x92}, {0xBF, 0x6A, 0x3F}, {0xBF, 0x95, 0x3F},
	{0xBF, 0xBF, 0x3F}, {0x95, 0xBF, 0x3F}, {0x6A, 0xBF, 0x3F}, {0x3F, 0xBF, 0x3F}, {0x3F, 0xBF, 0x6A}, {0x3F, 0xBF, 0x95},
	{0x3F, 0xBF, 0xBF}, {0x3f, 0x95, 0xBF}, {0x3F, 0x6A, 0xBF}, {0x3F, 0x3F, 0xBF}, {0x6A, 0x3F, 0xBF}, {0x95, 0x3F, 0xBF},
	{0xBF, 0x3F, 0xBF}, {0xBF, 0x3F, 0x95}, {0xBF, 0x3F, 0x6A}, {0xBF, 0x3F, 0x3F}, {0x6D, 0x6D, 0x6D}, {0xFF, 0x55, 0x00},
	{0xFF, 0xAA, 0x00}, {0xFF, 0xFF, 0x00}, {0xAA, 0xFF, 0x00}, {0x55, 0xFF, 0x00}, {0x00, 0xFF, 0x00}, {0x00, 0xFF, 0x55},
	{0x00, 0xFF, 0xAA}, {0x00, 0xFF, 0xFF}, {0x00, 0xAA, 0xFF}, {0x00, 0x55, 0xFF}, {0x00, 0x00, 0xFF}, {0x55, 0x00, 0xFF},
	{0xAA, 0x00, 0xFF}, {0xFF, 0x00, 0xFF}, {0xFF, 0x00, 0xAA}, {0xFF, 0x00, 0x55}, {0xFF, 0x00, 0x00}, {0x49, 0x49, 0x49},
	{0xBF, 0x3F, 0x00}, {0xBF, 0x7F, 0x00}, {0xBF, 0xBF, 0x00}, {0x7F, 0xBF, 0x00}, {0x3F, 0xBF, 0x00}, {0x00, 0xBF, 0x00},
	{0x00, 0xBF, 0x3F}, {0x00, 0xBF, 0x7F}, {0x00, 0xBF, 0xBF}, {0x00, 0x7F, 0xBF}, {0x00, 0x3F, 0xBF}, {0x00, 0x00, 0xBF},
	{0x3F, 0x00, 0xBF}, {0x7F, 0x00, 0xBF}, {0xBf, 0x00, 0xBF}, {0xBF, 0x00, 0x7F}, {0xBF, 0x00, 0x3F}, {0xBF, 0x00, 0x00},
	{0x24, 0x24, 0x24}, {0x7F, 0x2A, 0x00}, {0x7F, 0x55, 0x00}, {0x7F, 0x7F, 0x00}, {0x55, 0x7F, 0x00}, {0x2A, 0x7F, 0x00},
	{0x00, 0x7F, 0x00}, {0x00, 0x7F, 0x2A}, {0x00, 0x7F, 0x55}, {0x00, 0x7F, 0x7F}, {0x00, 0x55, 0x7F}, {0x00, 0x2A, 0x7F},
	{0x00, 0x00, 0x7F}, {0x2A, 0x00, 0x7F}, {0x55, 0x00, 0x7F}, {0x7F, 0x00, 0x7F}, {0x7F, 0x00, 0x55}, {0x7F, 0x00, 0x2A},
	{0x7F, 0x00, 0x00},
};

Uint8 colorTo8bit(Uint8 r, Uint8 g, Uint8 b)
{
	Uint32 c = 0;
	c += (r / 51) * 36;
	c += (g / 51) * 6;
	c += (b / 51);
	return SDL_static_cast(Uint8, c);
}

void colorFrom8bitFloat(Uint8 color, float& r, float& g, float& b)
{
	if(color == 0)
	{
		r = 0.0f;
		g = 0.0f;
		b = 0.0f;
		return;
	}

	r = ((color / 36) % 6 * 51) / 255.f;
	g = ((color / 6) % 6 * 51) / 255.f;
	b = (color % 6 * 51) / 255.f;
}

void colorFrom8bit(Uint8 color, Uint8& r, Uint8& g, Uint8& b)
{
	if(color == 0)
	{
		r = 0;
		g = 0;
		b = 0;
		return;
	}

	r = (color / 36) % 6 * 51;
	g = (color / 6) % 6 * 51;
	b = color % 6 * 51;
}

void getOutfitColorFloat(Uint8 color, float& r, float& g, float& b)
{
	if(color >= 133)
		color = 0;

	const Uint8* RGBc = outfitColorTable[color];
	r = RGBc[0] / 255.f;
	g = RGBc[1] / 255.f;
	b = RGBc[2] / 255.f;
}

void getOutfitColorRGB(Uint8 color, Uint8& r, Uint8& g, Uint8& b)
{
	if(color >= 133)
		color = 0;

	const Uint8* RGBc = outfitColorTable[color];
	r = RGBc[0];
	g = RGBc[1];
	b = RGBc[2];
}

SDL_FORCE_INLINE float edgeFunction(const float a[2], const float b[2], const float c[2])
{
	return (c[0] - a[0]) * (b[1] - a[1]) - (c[1] - a[1]) * (b[0] - a[0]);
}

void getTrianglePointFloat(const float v0[2], const float v1[2], const float v2[2], const float c0[3], const float c1[3], const float c2[3], const float p[2], float result[3])
{
	float area = edgeFunction(v0, v1, v2);
	float w0 = edgeFunction(v1, v2, p); w0 /= area;
	float w1 = edgeFunction(v2, v0, p); w1 /= area;
	float w2 = edgeFunction(v0, v1, p); w2 /= area;
	result[0] = w0 * c0[0] + w1 * c1[0] + w2 * c2[0];
	result[1] = w0 * c0[1] + w1 * c1[1] + w2 * c2[1];
	result[2] = w0 * c0[2] + w1 * c1[2] + w2 * c2[2];
}

#ifdef __GNUC__
void __cpuid(int* cpuinfo, int info)
{
	__asm__ __volatile__(
		"xchg %%ebx, %%edi;"
		"cpuid;"
		"xchg %%ebx, %%edi;"
		:"=a" (cpuinfo[0]), "=D" (cpuinfo[1]), "=c" (cpuinfo[2]), "=d" (cpuinfo[3])
		: "0" (info)
	);
}
#endif

bool SDL_HasSSSE3()
{
	//SDL don't have checks for SSSE3 so write one ourself
	//Currently only gcc + msvc
	#if defined(__GNUC__) || defined(_MSC_VER)
	int cpuinfo[4];
	__cpuid(cpuinfo, 1);
	return (cpuinfo[2] & (1 << 9));
	#else
	return false;
	#endif
}

bool SDL_HasFMA3()
{
	//SDL don't have checks for FMA3 so write one ourself
	//Currently only gcc + msvc
	#if defined(__GNUC__) || defined(_MSC_VER)
	if(!SDL_HasAVX())
		return false;

	int cpuinfo[4];
	__cpuid(cpuinfo, 1);
	return (cpuinfo[2] & (1 << 12));
	#else
	return false;
	#endif
}

bool SDL_HasFMA4()
{
	//SDL don't have checks for FMA4 so write one ourself
	//Currently only gcc + msvc
	#if defined(__GNUC__) || defined(_MSC_VER)
	if(!SDL_HasAVX())
		return false;

	int cpuinfo[4];
	__cpuid(cpuinfo, 0x80000001);
	return (cpuinfo[2] & (1 << 16));
	#else
	return false;
	#endif
}

bool SDL_HasXOP()
{
	//SDL don't have checks for XOP so write one ourself
	//Currently only gcc + msvc
	#if defined(__GNUC__) || defined(_MSC_VER)
	if(!SDL_HasAVX())
		return false;

	int cpuinfo[4];
	__cpuid(cpuinfo, 0x80000001);
	return (cpuinfo[2] & (1 << 11));
	#else
	return false;
	#endif
}

char* SDL_GetCPUName()
{
	#if defined(__GNUC__) || defined(_MSC_VER)
	int cpuinfo[4];
	__cpuid(cpuinfo, 0x80000000);
	int nExIds = cpuinfo[0];

	SDL_memset(g_buffer, 0, 48);
	for(int i = 0x80000000; i <= nExIds; ++i)
	{
		__cpuid(cpuinfo, i);
		if(i == 0x80000002)
		{
			g_buffer[0] = SDL_static_cast(Uint8, cpuinfo[0]); g_buffer[1] = SDL_static_cast(Uint8, cpuinfo[0] >> 8);
			g_buffer[2] = SDL_static_cast(Uint8, cpuinfo[0] >> 16); g_buffer[3] = SDL_static_cast(Uint8, cpuinfo[0] >> 24);
			g_buffer[4] = SDL_static_cast(Uint8, cpuinfo[1]); g_buffer[5] = SDL_static_cast(Uint8, cpuinfo[1] >> 8);
			g_buffer[6] = SDL_static_cast(Uint8, cpuinfo[1] >> 16); g_buffer[7] = SDL_static_cast(Uint8, cpuinfo[1] >> 24);
			g_buffer[8] = SDL_static_cast(Uint8, cpuinfo[2]); g_buffer[9] = SDL_static_cast(Uint8, cpuinfo[2] >> 8);
			g_buffer[10] = SDL_static_cast(Uint8, cpuinfo[2] >> 16); g_buffer[11] = SDL_static_cast(Uint8, cpuinfo[2] >> 24);
			g_buffer[12] = SDL_static_cast(Uint8, cpuinfo[3]); g_buffer[13] = SDL_static_cast(Uint8, cpuinfo[3] >> 8);
			g_buffer[14] = SDL_static_cast(Uint8, cpuinfo[3] >> 16); g_buffer[15] = SDL_static_cast(Uint8, cpuinfo[3] >> 24);
		}
		else if(i == 0x80000003)
		{
			g_buffer[16] = SDL_static_cast(Uint8, cpuinfo[0]); g_buffer[17] = SDL_static_cast(Uint8, cpuinfo[0] >> 8);
			g_buffer[18] = SDL_static_cast(Uint8, cpuinfo[0] >> 16); g_buffer[19] = SDL_static_cast(Uint8, cpuinfo[0] >> 24);
			g_buffer[20] = SDL_static_cast(Uint8, cpuinfo[1]); g_buffer[21] = SDL_static_cast(Uint8, cpuinfo[1] >> 8);
			g_buffer[22] = SDL_static_cast(Uint8, cpuinfo[1] >> 16); g_buffer[23] = SDL_static_cast(Uint8, cpuinfo[1] >> 24);
			g_buffer[24] = SDL_static_cast(Uint8, cpuinfo[2]); g_buffer[25] = SDL_static_cast(Uint8, cpuinfo[2] >> 8);
			g_buffer[26] = SDL_static_cast(Uint8, cpuinfo[2] >> 16); g_buffer[27] = SDL_static_cast(Uint8, cpuinfo[2] >> 24);
			g_buffer[28] = SDL_static_cast(Uint8, cpuinfo[3]); g_buffer[29] = SDL_static_cast(Uint8, cpuinfo[3] >> 8);
			g_buffer[30] = SDL_static_cast(Uint8, cpuinfo[3] >> 16); g_buffer[31] = SDL_static_cast(Uint8, cpuinfo[3] >> 24);
		}
		else if(i == 0x80000004)
		{
			g_buffer[32] = SDL_static_cast(Uint8, cpuinfo[0]); g_buffer[33] = SDL_static_cast(Uint8, cpuinfo[0] >> 8);
			g_buffer[34] = SDL_static_cast(Uint8, cpuinfo[0] >> 16); g_buffer[35] = SDL_static_cast(Uint8, cpuinfo[0] >> 24);
			g_buffer[36] = SDL_static_cast(Uint8, cpuinfo[1]); g_buffer[37] = SDL_static_cast(Uint8, cpuinfo[1] >> 8);
			g_buffer[38] = SDL_static_cast(Uint8, cpuinfo[1] >> 16); g_buffer[39] = SDL_static_cast(Uint8, cpuinfo[1] >> 24);
			g_buffer[40] = SDL_static_cast(Uint8, cpuinfo[2]); g_buffer[41] = SDL_static_cast(Uint8, cpuinfo[2] >> 8);
			g_buffer[42] = SDL_static_cast(Uint8, cpuinfo[2] >> 16); g_buffer[43] = SDL_static_cast(Uint8, cpuinfo[2] >> 24);
			g_buffer[44] = SDL_static_cast(Uint8, cpuinfo[3]); g_buffer[45] = SDL_static_cast(Uint8, cpuinfo[3] >> 8);
			g_buffer[46] = SDL_static_cast(Uint8, cpuinfo[3] >> 16); g_buffer[47] = SDL_static_cast(Uint8, cpuinfo[3] >> 24);
		}
	}
	for(int i = 47; i > 0; --i)
	{
		//Intel - trim the GHz counter
		if(g_buffer[i] == '@')
			g_buffer[i] = '\0';
	}
	for(int i = 47; i > 0; --i)
	{
		//Trim the spaces if there's any
		if(g_buffer[i] != ' ' && g_buffer[i] != '\0')
			break;

		g_buffer[i] = '\0';
	}
	return g_buffer;
	#else
	return NULL;
	#endif
}

Uint32 UTIL_ctz(Uint32 mask)
{
	#if ((__GNUC__ >= 4) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 4)))
	return __builtin_ctz(mask);
	#elif defined(__GNUC__)
	Uint32 pos;
	asm("bsf %1, %0" : "=r" (pos) : "rm" (mask));
	return pos;
	#elif (_MSC_VER >= 1300)
	DWORD pos;
	_BitScanForward(&pos, mask);
	return SDL_static_cast(Uint32, pos);
	#elif defined(_MSC_VER)
	Uint32 pos;
	__asm bsf pos, mask
	return pos;
	#else
	Uint32 pos = 1;
	if(!(mask & 0x0000FFFFU)) {pos += 16; mask >>= 16;}
	if(!(mask & 0x000000FFU)) {pos += 8; mask >>= 8;}
	if(!(mask & 0x0000000FU)) {pos += 4; mask >>= 4;}
	if(!(mask & 0x00000003U)) {pos += 2; mask >>= 2;}
	pos -= mask & 1;
	return pos;
	#endif
}

Uint32 UTIL_clz(Uint32 mask)
{
	#if ((__GNUC__ >= 4) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 4)))
	return __builtin_clz(mask);
	#elif defined(__GNUC__)
	Uint32 pos;
	asm("bsr %1, %0" : "=r" (pos) : "rm" (mask));
	return pos;
	#elif (_MSC_VER >= 1300)
	DWORD pos;
	_BitScanReverse(&pos, mask);
	return SDL_static_cast(Uint32, pos);
	#elif defined(_MSC_VER)
	Uint32 pos;
	__asm bsr pos, mask
	return pos;
	#else
	for(Sint32 bits = 31; bits > 0; --bits)
	{
		if(mask & (1 << bits))
			return bits;
	}
	return 0;
	#endif
}

void UTIL_integer_scale(Sint32& w, Sint32& h, Sint32 expectW, Sint32 expectH, Sint32 limitW, Sint32 limitH)
{
	bool haveXvalue = false, haveYvalue = false;
	for(Sint32 i = 2; i < 50; ++i)
	{
		if(!haveXvalue)
		{
			Sint32 tW = w * i;
			if(tW >= expectW)
			{
				w = tW;
				haveXvalue = true;
				if(haveYvalue) return;
			}
			else if(tW > limitW)
			{
				tW = w * (i - 1);
				w = tW;
				haveXvalue = true;
				if(haveYvalue) return;
			}
		}
		if(!haveYvalue)
		{
			Sint32 tH = h * i;
			if(tH >= expectH)
			{
				h = tH;
				haveYvalue = true;
				if(haveXvalue) return;
			}
			else if(tH > limitW)
			{
				tH = h * (i - 1);
				h = tH;
				haveYvalue = true;
				if(haveXvalue) return;
			}
		}
	}
	w = limitW;
	h = limitH;
}

std::string SDL_ReadLEString(SDL_RWops* src)
{
	std::string str;
	Uint16 len = SDL_ReadLE16(src);
	if(len > 0)
	{
		str.resize(len, '\0');
		SDL_RWread(src, &str[0], 1, len);
	}
	return str;
}

void SDL_WriteLEString(SDL_RWops* src, const std::string& text)
{
	Uint16 len = SDL_static_cast(Uint16, text.size());
	if(len > 0)
	{
		SDL_WriteLE16(src, len);
		SDL_RWwrite(src, text.c_str(), 1, len);
	}
	else
		SDL_WriteLE16(src, 0);
}

Uint32 SDL_ReadProtobufTag(SDL_RWops* src)
{
	Uint32 res = SDL_static_cast(Uint32, SDL_ReadU8(src));
	if(res < 128)
		return res;

	for(Uint32 i = 1; i < 5; ++i)
	{
		Uint32 byte = SDL_static_cast(Uint32, SDL_ReadU8(src));
		res += (byte - 1) << (7 * i);
		if(byte < 128)
			return res;
	}
	return 0;
}

Uint32 SDL_ReadProtobufSize(SDL_RWops* src)
{
	Uint32 res = SDL_static_cast(Uint32, SDL_ReadU8(src));
	if(res < 128)
		return res;

	Uint32 tmp = 1;
	for(Uint32 i = 0; i < 4; ++i)
	{
		Uint32 byte = SDL_static_cast(Uint32, SDL_ReadU8(src));
		tmp += (byte - 1) << (i * 7);
		if(byte < 128)
			break;
	}
	if(tmp >= (1 << 24) - 16)
		return 0;
	return (tmp << 7) + res - 0x80;
}

Uint64 SDL_ReadProtobufVariant(SDL_RWops* src)
{
	Uint32 res32 = SDL_static_cast(Uint32, SDL_ReadU8(src));
	if(res32 < 128)
		return SDL_static_cast(Uint64, res32);

	Uint32 read32 = SDL_static_cast(Uint32, SDL_ReadU8(src));
	res32 += (read32 - 1) << 7;
	if(read32 < 128)
		return SDL_static_cast(Uint64, res32);

	Uint64 res64 = SDL_static_cast(Uint64, res32);
	for(Uint32 i = 2; i < 10; ++i)
	{
		Uint64 read64 = SDL_static_cast(Uint64, SDL_ReadU8(src));
		res64 += (read64 - 1) << (7 * i);
		if(read64 < 128)
			return res64;
	}
	return 0;
}

std::string SDL_ReadProtobufString(SDL_RWops* src)
{
	std::string str;
	Uint32 len = SDL_ReadProtobufSize(src);
	if(len > 0)
	{
		str.resize(len, '\0');
		SDL_RWread(src, &str[0], 1, len);
	}
	return str;
}

void UTIL_SetClipboardTextLatin1(const char* clipboardText)
{
	char* utf8Text = SDL_iconv_string("UTF-8", "ISO-8859-1", clipboardText, SDL_strlen(clipboardText) + 1);
	if(utf8Text)
	{
		SDL_SetClipboardText(utf8Text);
		SDL_free(utf8Text);
	}
}

char* UTIL_GetClipboardTextLatin1()
{
	char* clipboardText = SDL_GetClipboardText();
	if(!clipboardText)
		return NULL;

	char* latin1Text = SDL_iconv_string("ISO-8859-1", "UTF-8", clipboardText, SDL_strlen(clipboardText) + 1);
	SDL_free(clipboardText);
	return latin1Text;
}

void UTIL_MessageBox(bool fatalError, const char* message)
{
	if(fatalError)//If you want UTF-8 support you must convert your message(if you haven't already)
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "The Forgotten Client - Fatal Error", message, NULL);
	else
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "The Forgotten Client - Error", message, NULL);
}

void UTIL_OpenURL(const char* url)
{
	//SDL don't have any native function to open a web browser with given url so write one ourselves
	#ifdef SDL_VIDEO_DRIVER_WINDOWS
	ShellExecuteA(GetDesktopWindow(), "open", url, NULL, NULL, SW_SHOWNORMAL);
	#else
	system(g_buffer);
	/*char* apps[] = {"xdg-open", "x-www-browser", "firefox", "chrome", "opera", "mozilla", "galeon", "konqueror", "safari", "open", NULL};
	for(int i = 0; apps[i]; ++i)
	{
		SDL_snprintf(g_buffer, sizeof(g_buffer), "which %s >/dev/null", apps[i]);
		if(system(g_buffer) == 0)
		{
			SDL_snprintf(g_buffer, sizeof(g_buffer), "%s %s", apps[i], url);
			system(g_buffer);
			return;
		}
	}*/
	#endif
}

void UTIL_TakeScreenshot(const char* filename, Uint32 flags)
{
	SDL_Event event;
	event.type = SDL_USEREVENT;
	event.user.code = CLIENT_EVENT_TAKESCREENSHOT;
	event.user.data1 = SDL_reinterpret_cast(void*, SDL_strdup(filename));
	event.user.data2 = SDL_reinterpret_cast(void*, SDL_static_cast(size_t, flags));
	SDL_PushEvent(&event);
}

void UTIL_ResizeEvent(Uint32 windowId, Sint32 width, Sint32 height)
{
	SDL_Event event;
	event.type = SDL_WINDOWEVENT;
	event.window.windowID = windowId;
	event.window.event = SDL_WINDOWEVENT_RESIZED;
	event.window.data1 = width;
	event.window.data2 = height;
	SDL_PushEvent(&event);
}

void UTIL_SafeEventHandler(void* eHandler, Uint32 param, Sint32 status)
{
	SDL_Event event;
	event.type = SDL_USEREVENT;
	event.user.code = CLIENT_EVENT_SAFEEVENTHANDLER;
	event.user.windowID = SDL_static_cast(Uint32, status);
	event.user.data1 = eHandler;
	event.user.data2 = SDL_reinterpret_cast(void*, SDL_static_cast(size_t, param));
	SDL_PushEvent(&event);
}

void UTIL_UpdatePanels(void* pPanel, Sint32 x, Sint32 y)
{
	SDL_Event event;
	event.type = SDL_USEREVENT;
	event.user.code = CLIENT_EVENT_UPDATEPANELS;
	event.user.windowID = x;
	event.user.data1 = pPanel;
	event.user.data2 = SDL_reinterpret_cast(void*, SDL_static_cast(size_t, y));
	SDL_PushEvent(&event);
}

void UTIL_ResizePanel(void* pPanel, Sint32 x, Sint32 y)
{
	SDL_Event event;
	event.type = SDL_USEREVENT;
	event.user.code = CLIENT_EVENT_RESIZEPANEL;
	event.user.windowID = x;
	event.user.data1 = pPanel;
	event.user.data2 = SDL_reinterpret_cast(void*, SDL_static_cast(size_t, y));
	SDL_PushEvent(&event);
}

void UTIL_replaceString(std::string& str, const std::string& sought, const std::string& replacement)
{
	size_t pos = 0;
	size_t start = 0;
	size_t soughtLen = sought.length();
	size_t replaceLen = replacement.length();
	while((pos = str.find(sought, start)) != std::string::npos)
	{
		str.assign(str.substr(0, pos).append(replacement).append(str.substr(pos + soughtLen)));
		start = pos + replaceLen;
	}
}

StringVector UTIL_explodeString(const std::string& inString, const std::string& separator, Sint32 limit)
{
	StringVector returnVector;
	std::string::size_type start = 0, end = 0;
	while(--limit != -1 && (end = inString.find(separator, start)) != std::string::npos)
	{
		returnVector.push_back(inString.substr(start, end - start));
		start = end + separator.size();
	}

	returnVector.push_back(inString.substr(start));
	return returnVector;
}

Sint32 UTIL_random(Sint32 min_range, Sint32 max_range)
{
	static std::default_random_engine generator(SDL_static_cast(unsigned, time(NULL)));
	static std::uniform_int_distribution<Sint32> uniformRand;
	if(min_range == max_range)
		return min_range;
	else if(min_range > max_range)
		std::swap(min_range, max_range);

	return uniformRand(generator, std::uniform_int_distribution<Sint32>::param_type(min_range, max_range));
}

Uint16 UTIL_parseModifiers(Uint16 mods)
{
	Uint16 result = KMOD_NONE;
	if(mods & KMOD_SHIFT)
		result += KMOD_SHIFT;
	if(mods & KMOD_CTRL)
		result += KMOD_CTRL;
	if(mods & KMOD_ALT)
		result += KMOD_ALT;
	if(mods & KMOD_GUI)
		result += KMOD_GUI;
	return result;
}

std::string UTIL_ipv4_tostring(Uint32 ipV4)
{
	unsigned char* bytes = SDL_reinterpret_cast(unsigned char*, &ipV4);
	SDL_snprintf(g_buffer, sizeof(g_buffer), "%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], bytes[3]);
	return std::string(g_buffer);
}

std::string UTIL_formatCreatureName(const std::string& name)
{
	std::string formatedName = name;
	if(!formatedName.empty())
	{
		bool upchnext = true;
		for(size_t i = 0, len = formatedName.length(); i < len; ++i)
		{
			char character = formatedName[i];
			if(upchnext)
			{
				formatedName[i] = (SDL_static_cast(unsigned char, character - 'a') <= ('Z' - 'A') ? character - 0x20 : character);
				upchnext = false;
			}
			else
			{
				if(character == ' ')
					upchnext = true;
				else
					formatedName[i] = (SDL_static_cast(unsigned char, character - 'A') <= ('z' - 'a') ? character + 0x20 : character);
			}
		}
	}
	return formatedName;
}

std::string UTIL_formatConsoleText(const std::string& text)
{
	std::string result;
	for(size_t i = 0, end = text.length(); i < end; ++i)
	{
		Uint8 character = SDL_static_cast(Uint8, text[i]);
		if(character == 0x0E)
		{
			if(i + 4 < end)
				i += 3;
			else
				i = end;

			continue;
		}
		else if(character == 0x0F)
			continue;
		else
			result.push_back(character);
	}
	return result;
}

std::string UTIL_formatStringCommas(const std::string& v)
{
	std::string result;
	size_t i = v.length();
	for(Sint32 j = 0; i-- > 0;)
	{
		if(v[i] != '-' && j++ == 3)
		{
			result.insert(result.begin(), ',');
			j = 1;
		}
		result.insert(result.begin(), v[i]);
	}
	return result;
}

#if (defined(_MSC_VER) && (_MSC_VER >= 1400))
#pragma warning(push)
#pragma warning(disable:4996)
#endif
std::string UTIL_formatDate(const char* format, time_t time)
{
	if(!time)
		return std::string();

	char buffer[128];
	struct tm tms;
	tms = *localtime(&time);
	size_t len = strftime(buffer, sizeof(buffer), format, &tms);
	return std::string(buffer, len);
}
#if (defined(_MSC_VER) && (_MSC_VER >= 1400))
#pragma warning(pop)
#endif

void UTIL_parseSizedText(const std::string& text, size_t start, Uint8 fontId, Uint32 allowedWidth, void* _THIS, size_t(*callback)(void* __THIS, bool skipLine, size_t start, size_t length))
{
	Uint32 width = 0;
	Uint32 goodPosWidth = 0;

	size_t goodPos = 0;
	size_t i = start;
	size_t strLen = text.length();
	while(i < strLen)
	{
		const Uint8 character = SDL_static_cast(Uint8, text[i]);
		if(character == '\n')
		{
			strLen += callback(_THIS, false, start, i - start + 1);
			start = i + 1;
			i = start;
			width = 0;
			goodPosWidth = 0;
			goodPos = 0;
			continue;
		}
		else if(character == 0x0E)//Special case - change rendering color
		{
			if(i + 4 < strLen)
				i += 4;//3 bytes color + special case character
			else
				i = strLen;
			continue;
		}
		else if(character == 0x0E)//Special case - change back standard color
		{
			++i;//special case character
			continue;
		}

		width += g_engine.calculateFontGlyphWidth(fontId, character);
		if((width + g_engine.getFontSpace(fontId)) > allowedWidth)
		{
			if(goodPos != 0)
			{
				strLen += callback(_THIS, false, start, goodPos - start + 1);
				start = goodPos + 1;
				width -= goodPosWidth + g_engine.calculateFontGlyphWidth(fontId, ' ');
				goodPosWidth = 0;
				goodPos = 0;
			}
			else
			{
				width = 0;
				strLen += callback(_THIS, true, start, i - start);
				start = i;
			}
		}
		else if(character == ' ')
		{
			goodPosWidth = width;
			goodPos = i;
		}

		++i;
	}
	if(i > start)
		callback(_THIS, false, start, i - start + 1);
}

#ifdef __USE_AVX2__
bool XTEA_decrypt_AVX2(Uint8* buffer, size_t size, const Uint32* keys)
{
	if((size & 7) != 0)
		return false;

	const Uint32 k[] = {keys[0], keys[1], keys[2], keys[3]};
	Sint64 messageLength = SDL_static_cast(Sint64, size) - 128;
	Sint64 readPos = 0;

	Uint32 precachedControlSum[32][2];
	Uint32 sum = 0xC6EF3720;
	Sint32 i = 0;
	do
	{
		#define PRECACHE_CONTROL_SUM(a)										\
			do {															\
				precachedControlSum[i + a][0] = (sum + k[(sum >> 11) & 3]);	\
				sum += xtea_delta;											\
				precachedControlSum[i + a][1] = (sum + k[sum & 3]);			\
			} while(0)

		PRECACHE_CONTROL_SUM(0);
		PRECACHE_CONTROL_SUM(1);
		PRECACHE_CONTROL_SUM(2);
		PRECACHE_CONTROL_SUM(3);
		#undef PRECACHE_CONTROL_SUM

		i += 4;
	} while(i < 32);
	while(readPos <= messageLength)
	{
		const __m256i data0 = _mm256_shuffle_epi32(_mm256_loadu_si256(SDL_reinterpret_cast(const __m256i*, buffer + readPos)), _MM_SHUFFLE(3, 1, 2, 0));
		const __m256i data1 = _mm256_shuffle_epi32(_mm256_loadu_si256(SDL_reinterpret_cast(const __m256i*, buffer + readPos + 32)), _MM_SHUFFLE(3, 1, 2, 0));
		const __m256i data2 = _mm256_shuffle_epi32(_mm256_loadu_si256(SDL_reinterpret_cast(const __m256i*, buffer + readPos + 64)), _MM_SHUFFLE(3, 1, 2, 0));
		const __m256i data3 = _mm256_shuffle_epi32(_mm256_loadu_si256(SDL_reinterpret_cast(const __m256i*, buffer + readPos + 96)), _MM_SHUFFLE(3, 1, 2, 0));
		__m256i vdata0 = _mm256_unpacklo_epi64(data0, data1);
		__m256i vdata1 = _mm256_unpackhi_epi64(data0, data1);
		__m256i vdata2 = _mm256_unpacklo_epi64(data2, data3);
		__m256i vdata3 = _mm256_unpackhi_epi64(data2, data3);

		Sint32 j = 0;
		do
		{
			__m256i controlSum1 = _mm256_set1_epi32(precachedControlSum[j][0]), controlSum2 = _mm256_set1_epi32(precachedControlSum[j][1]);
			vdata1 = _mm256_sub_epi32(vdata1, _mm256_xor_si256(_mm256_add_epi32(_mm256_xor_si256(_mm256_slli_epi32(vdata0, 4), _mm256_srli_epi32(vdata0, 5)), vdata0), controlSum1));
			vdata0 = _mm256_sub_epi32(vdata0, _mm256_xor_si256(_mm256_add_epi32(_mm256_xor_si256(_mm256_slli_epi32(vdata1, 4), _mm256_srli_epi32(vdata1, 5)), vdata1), controlSum2));
			vdata3 = _mm256_sub_epi32(vdata3, _mm256_xor_si256(_mm256_add_epi32(_mm256_xor_si256(_mm256_slli_epi32(vdata2, 4), _mm256_srli_epi32(vdata2, 5)), vdata2), controlSum1));
			vdata2 = _mm256_sub_epi32(vdata2, _mm256_xor_si256(_mm256_add_epi32(_mm256_xor_si256(_mm256_slli_epi32(vdata3, 4), _mm256_srli_epi32(vdata3, 5)), vdata3), controlSum2));
		} while(++j < 32);

		_mm256_storeu_si256(SDL_reinterpret_cast(__m256i*, buffer + readPos), _mm256_unpacklo_epi32(vdata0, vdata1));
		readPos += 32;
		_mm256_storeu_si256(SDL_reinterpret_cast(__m256i*, buffer + readPos), _mm256_unpackhi_epi32(vdata0, vdata1));
		readPos += 32;
		_mm256_storeu_si256(SDL_reinterpret_cast(__m256i*, buffer + readPos), _mm256_unpacklo_epi32(vdata2, vdata3));
		readPos += 32;
		_mm256_storeu_si256(SDL_reinterpret_cast(__m256i*, buffer + readPos), _mm256_unpackhi_epi32(vdata2, vdata3));
		readPos += 32;
	}
	messageLength += 64;
	if(readPos <= messageLength)
	{
		const __m256i data0 = _mm256_shuffle_epi32(_mm256_loadu_si256(SDL_reinterpret_cast(const __m256i*, buffer + readPos)), _MM_SHUFFLE(3, 1, 2, 0));
		const __m256i data1 = _mm256_shuffle_epi32(_mm256_loadu_si256(SDL_reinterpret_cast(const __m256i*, buffer + readPos + 32)), _MM_SHUFFLE(3, 1, 2, 0));
		__m256i vdata0 = _mm256_unpacklo_epi64(data0, data1);
		__m256i vdata1 = _mm256_unpackhi_epi64(data0, data1);

		Sint32 j = 0;
		do
		{
			vdata1 = _mm256_sub_epi32(vdata1, _mm256_xor_si256(_mm256_add_epi32(_mm256_xor_si256(_mm256_slli_epi32(vdata0, 4), _mm256_srli_epi32(vdata0, 5)), vdata0), _mm256_set1_epi32(precachedControlSum[j][0])));
			vdata0 = _mm256_sub_epi32(vdata0, _mm256_xor_si256(_mm256_add_epi32(_mm256_xor_si256(_mm256_slli_epi32(vdata1, 4), _mm256_srli_epi32(vdata1, 5)), vdata1), _mm256_set1_epi32(precachedControlSum[j][1])));
		} while(++j < 32);

		_mm256_storeu_si256(SDL_reinterpret_cast(__m256i*, buffer + readPos), _mm256_unpacklo_epi32(vdata0, vdata1));
		readPos += 32;
		_mm256_storeu_si256(SDL_reinterpret_cast(__m256i*, buffer + readPos), _mm256_unpackhi_epi32(vdata0, vdata1));
		readPos += 32;
	}
	messageLength += 32;
	if(readPos <= messageLength)
	{
		const __m128i data0 = _mm_shuffle_epi32(_mm_loadu_si128(SDL_reinterpret_cast(const __m128i*, buffer + readPos)), _MM_SHUFFLE(3, 1, 2, 0));
		const __m128i data1 = _mm_shuffle_epi32(_mm_loadu_si128(SDL_reinterpret_cast(const __m128i*, buffer + readPos + 16)), _MM_SHUFFLE(3, 1, 2, 0));
		__m128i vdata0 = _mm_unpacklo_epi64(data0, data1);
		__m128i vdata1 = _mm_unpackhi_epi64(data0, data1);

		Sint32 j = 0;
		do
		{
			vdata1 = _mm_sub_epi32(vdata1, _mm_xor_si128(_mm_add_epi32(_mm_xor_si128(_mm_slli_epi32(vdata0, 4), _mm_srli_epi32(vdata0, 5)), vdata0), _mm_set1_epi32(precachedControlSum[j][0])));
			vdata0 = _mm_sub_epi32(vdata0, _mm_xor_si128(_mm_add_epi32(_mm_xor_si128(_mm_slli_epi32(vdata1, 4), _mm_srli_epi32(vdata1, 5)), vdata1), _mm_set1_epi32(precachedControlSum[j][1])));
		} while(++j < 32);

		_mm_storeu_si128(SDL_reinterpret_cast(__m128i*, buffer + readPos), _mm_unpacklo_epi32(vdata0, vdata1));
		readPos += 16;
		_mm_storeu_si128(SDL_reinterpret_cast(__m128i*, buffer + readPos), _mm_unpackhi_epi32(vdata0, vdata1));
		readPos += 16;
	}
	messageLength += 32;
	while(readPos < messageLength)
	{
		Uint32 vData[2];
		vData[0] = SDL_static_cast(Uint32, buffer[readPos]) | (SDL_static_cast(Uint32, buffer[readPos + 1]) << 8)
			| (SDL_static_cast(Uint32, buffer[readPos + 2]) << 16) | (SDL_static_cast(Uint32, buffer[readPos + 3]) << 24);
		vData[1] = SDL_static_cast(Uint32, buffer[readPos + 4]) | (SDL_static_cast(Uint32, buffer[readPos + 5]) << 8)
			| (SDL_static_cast(Uint32, buffer[readPos + 6]) << 16) | (SDL_static_cast(Uint32, buffer[readPos + 7]) << 24);

		Sint32 j = 0;
		do
		{
			vData[1] -= ((vData[0] << 4 ^ vData[0] >> 5) + vData[0]) ^ precachedControlSum[j][0];
			vData[0] -= ((vData[1] << 4 ^ vData[1] >> 5) + vData[1]) ^ precachedControlSum[j][1];
		} while(++j < 32);

		buffer[readPos++] = SDL_static_cast(Uint8, vData[0]);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[0] >> 8);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[0] >> 16);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[0] >> 24);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[1]);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[1] >> 8);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[1] >> 16);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[1] >> 24);
	}
	return true;
}

bool XTEA_encrypt_AVX2(Uint8* buffer, size_t size, const Uint32* keys)
{
	if((size & 7) != 0)
		return false;

	const Uint32 k[] = {keys[0], keys[1], keys[2], keys[3]};
	Sint64 messageLength = SDL_static_cast(Sint64, size) - 128;
	Sint64 readPos = 0;

	Uint32 precachedControlSum[32][2];
	Uint32 sum = 0;
	Sint32 i = 0;
	do
	{
		#define PRECACHE_CONTROL_SUM(a)										\
			do {															\
				precachedControlSum[i + a][0] = (sum + k[sum & 3]);			\
				sum -= xtea_delta;											\
				precachedControlSum[i + a][1] = (sum + k[(sum >> 11) & 3]);	\
			} while(0)

		PRECACHE_CONTROL_SUM(0);
		PRECACHE_CONTROL_SUM(1);
		PRECACHE_CONTROL_SUM(2);
		PRECACHE_CONTROL_SUM(3);
		#undef PRECACHE_CONTROL_SUM

		i += 4;
	} while(i < 32);
	while(readPos <= messageLength)
	{
		const __m256i data0 = _mm256_shuffle_epi32(_mm256_loadu_si256(SDL_reinterpret_cast(const __m256i*, buffer + readPos)), _MM_SHUFFLE(3, 1, 2, 0));
		const __m256i data1 = _mm256_shuffle_epi32(_mm256_loadu_si256(SDL_reinterpret_cast(const __m256i*, buffer + readPos + 32)), _MM_SHUFFLE(3, 1, 2, 0));
		const __m256i data2 = _mm256_shuffle_epi32(_mm256_loadu_si256(SDL_reinterpret_cast(const __m256i*, buffer + readPos + 64)), _MM_SHUFFLE(3, 1, 2, 0));
		const __m256i data3 = _mm256_shuffle_epi32(_mm256_loadu_si256(SDL_reinterpret_cast(const __m256i*, buffer + readPos + 96)), _MM_SHUFFLE(3, 1, 2, 0));
		__m256i vdata0 = _mm256_unpacklo_epi64(data0, data1);
		__m256i vdata1 = _mm256_unpackhi_epi64(data0, data1);
		__m256i vdata2 = _mm256_unpacklo_epi64(data2, data3);
		__m256i vdata3 = _mm256_unpackhi_epi64(data2, data3);

		Sint32 j = 0;
		do
		{
			__m256i controlSum1 = _mm256_set1_epi32(precachedControlSum[j][0]), controlSum2 = _mm256_set1_epi32(precachedControlSum[j][1]);
			vdata0 = _mm256_add_epi32(vdata0, _mm256_xor_si256(_mm256_add_epi32(_mm256_xor_si256(_mm256_slli_epi32(vdata1, 4), _mm256_srli_epi32(vdata1, 5)), vdata1), controlSum1));
			vdata1 = _mm256_add_epi32(vdata1, _mm256_xor_si256(_mm256_add_epi32(_mm256_xor_si256(_mm256_slli_epi32(vdata0, 4), _mm256_srli_epi32(vdata0, 5)), vdata0), controlSum2));
			vdata2 = _mm256_add_epi32(vdata2, _mm256_xor_si256(_mm256_add_epi32(_mm256_xor_si256(_mm256_slli_epi32(vdata3, 4), _mm256_srli_epi32(vdata3, 5)), vdata3), controlSum1));
			vdata3 = _mm256_add_epi32(vdata3, _mm256_xor_si256(_mm256_add_epi32(_mm256_xor_si256(_mm256_slli_epi32(vdata2, 4), _mm256_srli_epi32(vdata2, 5)), vdata2), controlSum2));
		} while(++j < 32);

		_mm256_storeu_si256(SDL_reinterpret_cast(__m256i*, buffer + readPos), _mm256_unpacklo_epi32(vdata0, vdata1));
		readPos += 32;
		_mm256_storeu_si256(SDL_reinterpret_cast(__m256i*, buffer + readPos), _mm256_unpackhi_epi32(vdata0, vdata1));
		readPos += 32;
		_mm256_storeu_si256(SDL_reinterpret_cast(__m256i*, buffer + readPos), _mm256_unpacklo_epi32(vdata2, vdata3));
		readPos += 32;
		_mm256_storeu_si256(SDL_reinterpret_cast(__m256i*, buffer + readPos), _mm256_unpackhi_epi32(vdata2, vdata3));
		readPos += 32;
	}
	messageLength += 64;
	if(readPos <= messageLength)
	{
		const __m256i data0 = _mm256_shuffle_epi32(_mm256_loadu_si256(SDL_reinterpret_cast(const __m256i*, buffer + readPos)), _MM_SHUFFLE(3, 1, 2, 0));
		const __m256i data1 = _mm256_shuffle_epi32(_mm256_loadu_si256(SDL_reinterpret_cast(const __m256i*, buffer + readPos + 32)), _MM_SHUFFLE(3, 1, 2, 0));
		__m256i vdata0 = _mm256_unpacklo_epi64(data0, data1);
		__m256i vdata1 = _mm256_unpackhi_epi64(data0, data1);

		Sint32 j = 0;
		do
		{
			vdata0 = _mm256_add_epi32(vdata0, _mm256_xor_si256(_mm256_add_epi32(_mm256_xor_si256(_mm256_slli_epi32(vdata1, 4), _mm256_srli_epi32(vdata1, 5)), vdata1), _mm256_set1_epi32(precachedControlSum[j][0])));
			vdata1 = _mm256_add_epi32(vdata1, _mm256_xor_si256(_mm256_add_epi32(_mm256_xor_si256(_mm256_slli_epi32(vdata0, 4), _mm256_srli_epi32(vdata0, 5)), vdata0), _mm256_set1_epi32(precachedControlSum[j][1])));
		} while(++j < 32);

		_mm256_storeu_si256(SDL_reinterpret_cast(__m256i*, buffer + readPos), _mm256_unpacklo_epi32(vdata0, vdata1));
		readPos += 32;
		_mm256_storeu_si256(SDL_reinterpret_cast(__m256i*, buffer + readPos), _mm256_unpackhi_epi32(vdata0, vdata1));
		readPos += 32;
	}
	messageLength += 32;
	if(readPos <= messageLength)
	{
		const __m128i data0 = _mm_shuffle_epi32(_mm_loadu_si128(SDL_reinterpret_cast(const __m128i*, buffer + readPos)), _MM_SHUFFLE(3, 1, 2, 0));
		const __m128i data1 = _mm_shuffle_epi32(_mm_loadu_si128(SDL_reinterpret_cast(const __m128i*, buffer + readPos + 16)), _MM_SHUFFLE(3, 1, 2, 0));
		__m128i vdata0 = _mm_unpacklo_epi64(data0, data1);
		__m128i vdata1 = _mm_unpackhi_epi64(data0, data1);

		Sint32 j = 0;
		do
		{
			vdata0 = _mm_add_epi32(vdata0, _mm_xor_si128(_mm_add_epi32(_mm_xor_si128(_mm_slli_epi32(vdata1, 4), _mm_srli_epi32(vdata1, 5)), vdata1), _mm_set1_epi32(precachedControlSum[j][0])));
			vdata1 = _mm_add_epi32(vdata1, _mm_xor_si128(_mm_add_epi32(_mm_xor_si128(_mm_slli_epi32(vdata0, 4), _mm_srli_epi32(vdata0, 5)), vdata0), _mm_set1_epi32(precachedControlSum[j][1])));
		} while(++j < 32);

		_mm_storeu_si128(SDL_reinterpret_cast(__m128i*, buffer + readPos), _mm_unpacklo_epi32(vdata0, vdata1));
		readPos += 16;
		_mm_storeu_si128(SDL_reinterpret_cast(__m128i*, buffer + readPos), _mm_unpackhi_epi32(vdata0, vdata1));
		readPos += 16;
	}
	messageLength += 32;
	while(readPos < messageLength)
	{
		Uint32 vData[2];
		vData[0] = SDL_static_cast(Uint32, buffer[readPos]) | (SDL_static_cast(Uint32, buffer[readPos + 1]) << 8)
			| (SDL_static_cast(Uint32, buffer[readPos + 2]) << 16) | (SDL_static_cast(Uint32, buffer[readPos + 3]) << 24);
		vData[1] = SDL_static_cast(Uint32, buffer[readPos + 4]) | (SDL_static_cast(Uint32, buffer[readPos + 5]) << 8)
			| (SDL_static_cast(Uint32, buffer[readPos + 6]) << 16) | (SDL_static_cast(Uint32, buffer[readPos + 7]) << 24);

		Sint32 j = 0;
		do
		{
			vData[0] += ((vData[1] << 4 ^ vData[1] >> 5) + vData[1]) ^ precachedControlSum[j][0];
			vData[1] += ((vData[0] << 4 ^ vData[0] >> 5) + vData[0]) ^ precachedControlSum[j][1];
		} while(++j < 32);

		buffer[readPos++] = SDL_static_cast(Uint8, vData[0]);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[0] >> 8);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[0] >> 16);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[0] >> 24);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[1]);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[1] >> 8);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[1] >> 16);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[1] >> 24);
	}
	return true;
}
#endif

#ifdef __USE_SSE2__
bool XTEA_decrypt_SSE2(Uint8* buffer, size_t size, const Uint32* keys)
{
	if((size & 7) != 0)
		return false;

	const Uint32 k[] = {keys[0], keys[1], keys[2], keys[3]};
	Sint64 messageLength = SDL_static_cast(Sint64, size) - 64;
	Sint64 readPos = 0;

	Uint32 precachedControlSum[32][2];
	Uint32 sum = 0xC6EF3720;
	Sint32 i = 0;
	do
	{
		#define PRECACHE_CONTROL_SUM(a)										\
			do {															\
				precachedControlSum[i + a][0] = (sum + k[(sum >> 11) & 3]);	\
				sum += xtea_delta;											\
				precachedControlSum[i + a][1] = (sum + k[sum & 3]);			\
			} while(0)

		PRECACHE_CONTROL_SUM(0);
		PRECACHE_CONTROL_SUM(1);
		PRECACHE_CONTROL_SUM(2);
		PRECACHE_CONTROL_SUM(3);
		#undef PRECACHE_CONTROL_SUM

		i += 4;
	} while(i < 32);
	while(readPos <= messageLength)
	{
		const __m128i data0 = _mm_shuffle_epi32(_mm_loadu_si128(SDL_reinterpret_cast(const __m128i*, buffer + readPos)), _MM_SHUFFLE(3, 1, 2, 0));
		const __m128i data1 = _mm_shuffle_epi32(_mm_loadu_si128(SDL_reinterpret_cast(const __m128i*, buffer + readPos + 16)), _MM_SHUFFLE(3, 1, 2, 0));
		const __m128i data2 = _mm_shuffle_epi32(_mm_loadu_si128(SDL_reinterpret_cast(const __m128i*, buffer + readPos + 32)), _MM_SHUFFLE(3, 1, 2, 0));
		const __m128i data3 = _mm_shuffle_epi32(_mm_loadu_si128(SDL_reinterpret_cast(const __m128i*, buffer + readPos + 48)), _MM_SHUFFLE(3, 1, 2, 0));
		__m128i vdata0 = _mm_unpacklo_epi64(data0, data1);
		__m128i vdata1 = _mm_unpackhi_epi64(data0, data1);
		__m128i vdata2 = _mm_unpacklo_epi64(data2, data3);
		__m128i vdata3 = _mm_unpackhi_epi64(data2, data3);

		Sint32 j = 0;
		do
		{
			__m128i controlSum1 = _mm_set1_epi32(precachedControlSum[j][0]), controlSum2 = _mm_set1_epi32(precachedControlSum[j][1]);
			vdata1 = _mm_sub_epi32(vdata1, _mm_xor_si128(_mm_add_epi32(_mm_xor_si128(_mm_slli_epi32(vdata0, 4), _mm_srli_epi32(vdata0, 5)), vdata0), controlSum1));
			vdata0 = _mm_sub_epi32(vdata0, _mm_xor_si128(_mm_add_epi32(_mm_xor_si128(_mm_slli_epi32(vdata1, 4), _mm_srli_epi32(vdata1, 5)), vdata1), controlSum2));
			vdata3 = _mm_sub_epi32(vdata3, _mm_xor_si128(_mm_add_epi32(_mm_xor_si128(_mm_slli_epi32(vdata2, 4), _mm_srli_epi32(vdata2, 5)), vdata2), controlSum1));
			vdata2 = _mm_sub_epi32(vdata2, _mm_xor_si128(_mm_add_epi32(_mm_xor_si128(_mm_slli_epi32(vdata3, 4), _mm_srli_epi32(vdata3, 5)), vdata3), controlSum2));
		} while(++j < 32);

		_mm_storeu_si128(SDL_reinterpret_cast(__m128i*, buffer + readPos), _mm_unpacklo_epi32(vdata0, vdata1));
		readPos += 16;
		_mm_storeu_si128(SDL_reinterpret_cast(__m128i*, buffer + readPos), _mm_unpackhi_epi32(vdata0, vdata1));
		readPos += 16;
		_mm_storeu_si128(SDL_reinterpret_cast(__m128i*, buffer + readPos), _mm_unpacklo_epi32(vdata2, vdata3));
		readPos += 16;
		_mm_storeu_si128(SDL_reinterpret_cast(__m128i*, buffer + readPos), _mm_unpackhi_epi32(vdata2, vdata3));
		readPos += 16;
	}
	messageLength += 32;
	if(readPos <= messageLength)
	{
		const __m128i data0 = _mm_shuffle_epi32(_mm_loadu_si128(SDL_reinterpret_cast(const __m128i*, buffer + readPos)), _MM_SHUFFLE(3, 1, 2, 0));
		const __m128i data1 = _mm_shuffle_epi32(_mm_loadu_si128(SDL_reinterpret_cast(const __m128i*, buffer + readPos + 16)), _MM_SHUFFLE(3, 1, 2, 0));
		__m128i vdata0 = _mm_unpacklo_epi64(data0, data1);
		__m128i vdata1 = _mm_unpackhi_epi64(data0, data1);

		Sint32 j = 0;
		do
		{
			vdata1 = _mm_sub_epi32(vdata1, _mm_xor_si128(_mm_add_epi32(_mm_xor_si128(_mm_slli_epi32(vdata0, 4), _mm_srli_epi32(vdata0, 5)), vdata0), _mm_set1_epi32(precachedControlSum[j][0])));
			vdata0 = _mm_sub_epi32(vdata0, _mm_xor_si128(_mm_add_epi32(_mm_xor_si128(_mm_slli_epi32(vdata1, 4), _mm_srli_epi32(vdata1, 5)), vdata1), _mm_set1_epi32(precachedControlSum[j][1])));
		} while(++j < 32);

		_mm_storeu_si128(SDL_reinterpret_cast(__m128i*, buffer + readPos), _mm_unpacklo_epi32(vdata0, vdata1));
		readPos += 16;
		_mm_storeu_si128(SDL_reinterpret_cast(__m128i*, buffer + readPos), _mm_unpackhi_epi32(vdata0, vdata1));
		readPos += 16;
	}
	messageLength += 32;
	while(readPos < messageLength)
	{
		Uint32 vData[2];
		vData[0] = SDL_static_cast(Uint32, buffer[readPos]) | (SDL_static_cast(Uint32, buffer[readPos + 1]) << 8)
			| (SDL_static_cast(Uint32, buffer[readPos + 2]) << 16) | (SDL_static_cast(Uint32, buffer[readPos + 3]) << 24);
		vData[1] = SDL_static_cast(Uint32, buffer[readPos + 4]) | (SDL_static_cast(Uint32, buffer[readPos + 5]) << 8)
			| (SDL_static_cast(Uint32, buffer[readPos + 6]) << 16) | (SDL_static_cast(Uint32, buffer[readPos + 7]) << 24);

		Sint32 j = 0;
		do
		{
			vData[1] -= ((vData[0] << 4 ^ vData[0] >> 5) + vData[0]) ^ precachedControlSum[j][0];
			vData[0] -= ((vData[1] << 4 ^ vData[1] >> 5) + vData[1]) ^ precachedControlSum[j][1];
		} while(++j < 32);

		buffer[readPos++] = SDL_static_cast(Uint8, vData[0]);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[0] >> 8);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[0] >> 16);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[0] >> 24);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[1]);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[1] >> 8);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[1] >> 16);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[1] >> 24);
	}
	return true;
}

bool XTEA_encrypt_SSE2(Uint8* buffer, size_t size, const Uint32* keys)
{
	if((size & 7) != 0)
		return false;

	const Uint32 k[] = {keys[0], keys[1], keys[2], keys[3]};
	Sint64 messageLength = SDL_static_cast(Sint64, size) - 64;
	Sint64 readPos = 0;

	Uint32 precachedControlSum[32][2];
	Uint32 sum = 0;
	Sint32 i = 0;
	do
	{
		#define PRECACHE_CONTROL_SUM(a)										\
			do {															\
				precachedControlSum[i + a][0] = (sum + k[sum & 3]);			\
				sum -= xtea_delta;											\
				precachedControlSum[i + a][1] = (sum + k[(sum >> 11) & 3]);	\
			} while(0)

		PRECACHE_CONTROL_SUM(0);
		PRECACHE_CONTROL_SUM(1);
		PRECACHE_CONTROL_SUM(2);
		PRECACHE_CONTROL_SUM(3);
		#undef PRECACHE_CONTROL_SUM

		i += 4;
	} while(i < 32);
	while(readPos <= messageLength)
	{
		const __m128i data0 = _mm_shuffle_epi32(_mm_loadu_si128(SDL_reinterpret_cast(const __m128i*, buffer + readPos)), _MM_SHUFFLE(3, 1, 2, 0));
		const __m128i data1 = _mm_shuffle_epi32(_mm_loadu_si128(SDL_reinterpret_cast(const __m128i*, buffer + readPos + 16)), _MM_SHUFFLE(3, 1, 2, 0));
		const __m128i data2 = _mm_shuffle_epi32(_mm_loadu_si128(SDL_reinterpret_cast(const __m128i*, buffer + readPos + 32)), _MM_SHUFFLE(3, 1, 2, 0));
		const __m128i data3 = _mm_shuffle_epi32(_mm_loadu_si128(SDL_reinterpret_cast(const __m128i*, buffer + readPos + 48)), _MM_SHUFFLE(3, 1, 2, 0));
		__m128i vdata0 = _mm_unpacklo_epi64(data0, data1);
		__m128i vdata1 = _mm_unpackhi_epi64(data0, data1);
		__m128i vdata2 = _mm_unpacklo_epi64(data2, data3);
		__m128i vdata3 = _mm_unpackhi_epi64(data2, data3);

		Sint32 j = 0;
		do
		{
			__m128i controlSum1 = _mm_set1_epi32(precachedControlSum[j][0]), controlSum2 = _mm_set1_epi32(precachedControlSum[j][1]);
			vdata0 = _mm_add_epi32(vdata0, _mm_xor_si128(_mm_add_epi32(_mm_xor_si128(_mm_slli_epi32(vdata1, 4), _mm_srli_epi32(vdata1, 5)), vdata1), controlSum1));
			vdata1 = _mm_add_epi32(vdata1, _mm_xor_si128(_mm_add_epi32(_mm_xor_si128(_mm_slli_epi32(vdata0, 4), _mm_srli_epi32(vdata0, 5)), vdata0), controlSum2));
			vdata2 = _mm_add_epi32(vdata2, _mm_xor_si128(_mm_add_epi32(_mm_xor_si128(_mm_slli_epi32(vdata3, 4), _mm_srli_epi32(vdata3, 5)), vdata3), controlSum1));
			vdata3 = _mm_add_epi32(vdata3, _mm_xor_si128(_mm_add_epi32(_mm_xor_si128(_mm_slli_epi32(vdata2, 4), _mm_srli_epi32(vdata2, 5)), vdata2), controlSum2));
		} while(++j < 32);

		_mm_storeu_si128(SDL_reinterpret_cast(__m128i*, buffer + readPos), _mm_unpacklo_epi32(vdata0, vdata1));
		readPos += 16;
		_mm_storeu_si128(SDL_reinterpret_cast(__m128i*, buffer + readPos), _mm_unpackhi_epi32(vdata0, vdata1));
		readPos += 16;
		_mm_storeu_si128(SDL_reinterpret_cast(__m128i*, buffer + readPos), _mm_unpacklo_epi32(vdata2, vdata3));
		readPos += 16;
		_mm_storeu_si128(SDL_reinterpret_cast(__m128i*, buffer + readPos), _mm_unpackhi_epi32(vdata2, vdata3));
		readPos += 16;
	}
	messageLength += 32;
	if(readPos <= messageLength)
	{
		const __m128i data0 = _mm_shuffle_epi32(_mm_loadu_si128(SDL_reinterpret_cast(const __m128i*, buffer + readPos)), _MM_SHUFFLE(3, 1, 2, 0));
		const __m128i data1 = _mm_shuffle_epi32(_mm_loadu_si128(SDL_reinterpret_cast(const __m128i*, buffer + readPos + 16)), _MM_SHUFFLE(3, 1, 2, 0));
		__m128i vdata0 = _mm_unpacklo_epi64(data0, data1);
		__m128i vdata1 = _mm_unpackhi_epi64(data0, data1);

		Sint32 j = 0;
		do
		{
			vdata0 = _mm_add_epi32(vdata0, _mm_xor_si128(_mm_add_epi32(_mm_xor_si128(_mm_slli_epi32(vdata1, 4), _mm_srli_epi32(vdata1, 5)), vdata1), _mm_set1_epi32(precachedControlSum[j][0])));
			vdata1 = _mm_add_epi32(vdata1, _mm_xor_si128(_mm_add_epi32(_mm_xor_si128(_mm_slli_epi32(vdata0, 4), _mm_srli_epi32(vdata0, 5)), vdata0), _mm_set1_epi32(precachedControlSum[j][1])));
		} while(++j < 32);

		_mm_storeu_si128(SDL_reinterpret_cast(__m128i*, buffer + readPos), _mm_unpacklo_epi32(vdata0, vdata1));
		readPos += 16;
		_mm_storeu_si128(SDL_reinterpret_cast(__m128i*, buffer + readPos), _mm_unpackhi_epi32(vdata0, vdata1));
		readPos += 16;
	}
	messageLength += 32;
	while(readPos < messageLength)
	{
		Uint32 vData[2];
		vData[0] = SDL_static_cast(Uint32, buffer[readPos]) | (SDL_static_cast(Uint32, buffer[readPos + 1]) << 8)
			| (SDL_static_cast(Uint32, buffer[readPos + 2]) << 16) | (SDL_static_cast(Uint32, buffer[readPos + 3]) << 24);
		vData[1] = SDL_static_cast(Uint32, buffer[readPos + 4]) | (SDL_static_cast(Uint32, buffer[readPos + 5]) << 8)
			| (SDL_static_cast(Uint32, buffer[readPos + 6]) << 16) | (SDL_static_cast(Uint32, buffer[readPos + 7]) << 24);

		Sint32 j = 0;
		do
		{
			vData[0] += ((vData[1] << 4 ^ vData[1] >> 5) + vData[1]) ^ precachedControlSum[j][0];
			vData[1] += ((vData[0] << 4 ^ vData[0] >> 5) + vData[0]) ^ precachedControlSum[j][1];
		} while(++j < 32);

		buffer[readPos++] = SDL_static_cast(Uint8, vData[0]);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[0] >> 8);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[0] >> 16);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[0] >> 24);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[1]);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[1] >> 8);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[1] >> 16);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[1] >> 24);
	}
	return true;
}
#endif

bool XTEA_decrypt_scalar(Uint8* buffer, size_t size, const Uint32* keys)
{
	if((size & 7) != 0)
		return false;

	const Uint32 k[] = {keys[0], keys[1], keys[2], keys[3]};
	size_t readPos = 0;
	while(readPos < size)
	{
		Uint32 vData[2];
		vData[0] = SDL_static_cast(Uint32, buffer[readPos]) | (SDL_static_cast(Uint32, buffer[readPos + 1]) << 8)
			| (SDL_static_cast(Uint32, buffer[readPos + 2]) << 16) | (SDL_static_cast(Uint32, buffer[readPos + 3]) << 24);
		vData[1] = SDL_static_cast(Uint32, buffer[readPos + 4]) | (SDL_static_cast(Uint32, buffer[readPos + 5]) << 8)
			| (SDL_static_cast(Uint32, buffer[readPos + 6]) << 16) | (SDL_static_cast(Uint32, buffer[readPos + 7]) << 24);

		Uint32 sum = 0xC6EF3720;
		Sint32 i = 0;
		do
		{
			vData[1] -= ((vData[0] << 4 ^ vData[0] >> 5) + vData[0]) ^ (sum + k[(sum >> 11) & 3]);
			sum += xtea_delta;
			vData[0] -= ((vData[1] << 4 ^ vData[1] >> 5) + vData[1]) ^ (sum + k[sum & 3]);
		} while(++i < 32);

		buffer[readPos++] = SDL_static_cast(Uint8, vData[0]);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[0] >> 8);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[0] >> 16);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[0] >> 24);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[1]);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[1] >> 8);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[1] >> 16);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[1] >> 24);
	}
	return true;
}

bool XTEA_encrypt_scalar(Uint8* buffer, size_t size, const Uint32* keys)
{
	if((size & 7) != 0)
		return false;

	const Uint32 k[] = {keys[0], keys[1], keys[2], keys[3]};
	size_t readPos = 0;
	while(readPos < size)
	{
		Uint32 vData[2];
		vData[0] = SDL_static_cast(Uint32, buffer[readPos]) | (SDL_static_cast(Uint32, buffer[readPos + 1]) << 8)
			| (SDL_static_cast(Uint32, buffer[readPos + 2]) << 16) | (SDL_static_cast(Uint32, buffer[readPos + 3]) << 24);
		vData[1] = SDL_static_cast(Uint32, buffer[readPos + 4]) | (SDL_static_cast(Uint32, buffer[readPos + 5]) << 8)
			| (SDL_static_cast(Uint32, buffer[readPos + 6]) << 16) | (SDL_static_cast(Uint32, buffer[readPos + 7]) << 24);

		Uint32 sum = 0;
		Sint32 i = 0;
		do
		{
			vData[0] += ((vData[1] << 4 ^ vData[1] >> 5) + vData[1]) ^ (sum + k[sum & 3]);
			sum -= xtea_delta;
			vData[1] += ((vData[0] << 4 ^ vData[0] >> 5) + vData[0]) ^ (sum + k[(sum >> 11) & 3]);
		} while(++i < 32);

		buffer[readPos++] = SDL_static_cast(Uint8, vData[0]);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[0] >> 8);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[0] >> 16);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[0] >> 24);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[1]);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[1] >> 8);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[1] >> 16);
		buffer[readPos++] = SDL_static_cast(Uint8, vData[1] >> 24);
	}
	return true;
}

#ifdef __USE_SSE2__
Uint32 adler32Checksum_SSE2(const Uint8* data, size_t length)
{
	const __m128i h16 = _mm_setr_epi16(16, 15, 14, 13, 12, 11, 10, 9);
	const __m128i h8 = _mm_setr_epi16(8, 7, 6, 5, 4, 3, 2, 1);
	const __m128i zeros = _mm_setzero_si128();

	Uint32 a = 1, b = 0;
	while(length > 0)
	{
		size_t temp = length > 5552 ? 5552 : length;
		length -= temp;
		while(temp >= 16)
		{
			__m128i vdata = _mm_loadu_si128(SDL_reinterpret_cast(const __m128i*, data));
			__m128i v = _mm_sad_epu8(vdata, zeros);
			__m128i v32 = _mm_add_epi32(_mm_madd_epi16(_mm_unpacklo_epi8(vdata, zeros), h16), _mm_madd_epi16(_mm_unpackhi_epi8(vdata, zeros), h8));
			v32 = _mm_add_epi32(v32, _mm_shuffle_epi32(v32, _MM_SHUFFLE(2, 3, 0, 1)));
			v32 = _mm_add_epi32(v32, _mm_shuffle_epi32(v32, _MM_SHUFFLE(0, 1, 2, 3)));
			v = _mm_add_epi32(v, _mm_shuffle_epi32(v, _MM_SHUFFLE(1, 0, 3, 2)));
			b += (a << 4) + _mm_cvtsi128_si32(v32);
			a += _mm_cvtsi128_si32(v);

			data += 16;
			temp -= 16;
		}

		while(temp--)
		{
			a += *data++;
			b += a;
		}

		a %= 65521;
		b %= 65521;
	}
	return (b << 16) | a;
}
#endif

Uint32 adler32Checksum_scalar(const Uint8* data, size_t length)
{
	Uint32 a = 1, b = 0;
	while(length > 0)
	{
		size_t temp = length > 5552 ? 5552 : length;
		length -= temp;

		do
		{
			a += *data++;
			b += a;
		} while(--temp);

		a %= 65521;
		b %= 65521;
	}
	return (b << 16) | a;
}

void UTIL_FastCopy_Standard(Uint8* dst, const Uint8* src, size_t size)
{
	#if defined(_WIN32)
	__movsb(dst, src, size);
	#elif defined(__i386__) || defined(__x86_64___)
	__asm__ __volatile__("rep movsb" : "+D"(dst), "+S"(src), "+c"(size) : : "memory");
	#else
	SDL_memcpy(dst, src, size);
	#endif
}

#ifdef __USE_SSE__
void UTIL_FastCopy_SSE(Uint8* dst, const Uint8* src, size_t size)
{
	if(size <= 128)
	{
		UTIL_FastCopy_Standard(dst, src, size);
		return;
	}
	size_t padding = (16 - (SDL_reinterpret_cast(size_t, dst) & 15)) & 15;
	if(padding > 0)
	{
		_mm_storeu_ps(SDL_reinterpret_cast(float*, dst), _mm_loadu_ps(SDL_reinterpret_cast(const float*, src)));
		dst += padding;
		src += padding;
		size -= padding;
	}
	__m128 values[4];
	if((SDL_reinterpret_cast(size_t, src) & 15) == 0)
	{
		_mm_prefetch(SDL_reinterpret_cast(const char*, src), _MM_HINT_NTA);
		for(size_t i = size / 64; i--;)
		{
			values[0] = _mm_load_ps(SDL_reinterpret_cast(const float*, src + 0));
			values[1] = _mm_load_ps(SDL_reinterpret_cast(const float*, src + 16));
			values[2] = _mm_load_ps(SDL_reinterpret_cast(const float*, src + 32));
			values[3] = _mm_load_ps(SDL_reinterpret_cast(const float*, src + 48));
			_mm_prefetch(SDL_reinterpret_cast(const char*, src + 64), _MM_HINT_NTA);
			_mm_stream_ps(SDL_reinterpret_cast(float*, dst + 0),  values[0]);
			_mm_stream_ps(SDL_reinterpret_cast(float*, dst + 16), values[1]);
			_mm_stream_ps(SDL_reinterpret_cast(float*, dst + 32), values[2]);
			_mm_stream_ps(SDL_reinterpret_cast(float*, dst + 48), values[3]);
			src += 64;
			dst += 64;
		}
	}
	else
	{
		_mm_prefetch(SDL_reinterpret_cast(const char*, src), _MM_HINT_NTA);
		for(size_t i = size / 64; i--;)
		{
			values[0] = _mm_loadu_ps(SDL_reinterpret_cast(const float*, src + 0));
			values[1] = _mm_loadu_ps(SDL_reinterpret_cast(const float*, src + 16));
			values[2] = _mm_loadu_ps(SDL_reinterpret_cast(const float*, src + 32));
			values[3] = _mm_loadu_ps(SDL_reinterpret_cast(const float*, src + 48));
			_mm_prefetch(SDL_reinterpret_cast(const char*, src + 64), _MM_HINT_NTA);
			_mm_stream_ps(SDL_reinterpret_cast(float*, dst + 0),  values[0]);
			_mm_stream_ps(SDL_reinterpret_cast(float*, dst + 16), values[1]);
			_mm_stream_ps(SDL_reinterpret_cast(float*, dst + 32), values[2]);
			_mm_stream_ps(SDL_reinterpret_cast(float*, dst + 48), values[3]);
			src += 64;
			dst += 64;
		}
	}
	_mm_sfence();
	size &= 63;
	if(size)
		UTIL_FastCopy_Standard(dst, src, size);
}
#endif

#ifdef __USE_SSE4_1__
void UTIL_FastCopy_SSE41(Uint8* dst, const Uint8* src, size_t size)
{
	if(size <= 128)
	{
		UTIL_FastCopy_Standard(dst, src, size);
		return;
	}
	size_t padding = (16 - (SDL_reinterpret_cast(size_t, dst) & 15)) & 15;
	if(padding > 0)
	{
		_mm_storeu_si128(SDL_reinterpret_cast(__m128i*, dst), _mm_loadu_si128(SDL_reinterpret_cast(const __m128i*, src)));
		dst += padding;
		src += padding;
		size -= padding;
	}
	__m128i values[4];
	if((SDL_reinterpret_cast(size_t, src) & 15) == 0)
	{
		for(size_t i = size / 64; i--;)
		{
			values[0] = _mm_stream_load_si128(SDL_reinterpret_cast(const __m128i*, src + 0));
			values[1] = _mm_stream_load_si128(SDL_reinterpret_cast(const __m128i*, src + 16));
			values[2] = _mm_stream_load_si128(SDL_reinterpret_cast(const __m128i*, src + 32));
			values[3] = _mm_stream_load_si128(SDL_reinterpret_cast(const __m128i*, src + 48));
			_mm_stream_si128(SDL_reinterpret_cast(__m128i*, dst + 0),  values[0]);
			_mm_stream_si128(SDL_reinterpret_cast(__m128i*, dst + 16), values[1]);
			_mm_stream_si128(SDL_reinterpret_cast(__m128i*, dst + 32), values[2]);
			_mm_stream_si128(SDL_reinterpret_cast(__m128i*, dst + 48), values[3]);
			src += 64;
			dst += 64;
		}
	}
	else
	{
		_mm_prefetch(SDL_reinterpret_cast(const char*, src), _MM_HINT_NTA);
		for(size_t i = size / 64; i--;)
		{
			values[0] = _mm_loadu_si128(SDL_reinterpret_cast(const __m128i*, src + 0));
			values[1] = _mm_loadu_si128(SDL_reinterpret_cast(const __m128i*, src + 16));
			values[2] = _mm_loadu_si128(SDL_reinterpret_cast(const __m128i*, src + 32));
			values[3] = _mm_loadu_si128(SDL_reinterpret_cast(const __m128i*, src + 48));
			_mm_prefetch(SDL_reinterpret_cast(const char*, src + 64), _MM_HINT_NTA);
			_mm_stream_si128(SDL_reinterpret_cast(__m128i*, dst + 0),  values[0]);
			_mm_stream_si128(SDL_reinterpret_cast(__m128i*, dst + 16), values[1]);
			_mm_stream_si128(SDL_reinterpret_cast(__m128i*, dst + 32), values[2]);
			_mm_stream_si128(SDL_reinterpret_cast(__m128i*, dst + 48), values[3]);
			src += 64;
			dst += 64;
		}
	}
	_mm_sfence();
	size &= 63;
	if(size)
		UTIL_FastCopy_Standard(dst, src, size);
}
#endif

size_t UTIL_Faststrstr_Scalar(const char* haystack, size_t haystackSize, const char* needle, size_t needleSize)
{
	if(haystackSize == needleSize)
		return (memcmp(haystack, needle, haystackSize) == 0 ? 0 : std::string::npos);
	else if(needleSize > haystackSize)
		return std::string::npos;

	const size_t lastpos = haystackSize - needleSize + 1;
	for(size_t i = 0; i < lastpos; ++i)
	{
		if(haystack[i] == needle[0])
		{
			if(memcmp(haystack + i, needle, needleSize) == 0)
				return i;
		}
	}
	return std::string::npos;
}

#if SIZEOF_VOIDP == 4
size_t UTIL_Faststrstr_Swar32(const char* haystack, size_t haystackSize, const char* needle, size_t needleSize)
{
	size_t result = std::string::npos;
	if(needleSize > haystackSize)
		return result;

	const Uint32 first = 0x01010101u * SDL_static_cast(Uint8, needle[0]);
	const Uint32 last = 0x01010101u * SDL_static_cast(Uint8, needle[needleSize - 1]);

	Uint32* block_first = SDL_reinterpret_cast(Uint32*, SDL_const_cast(char*, haystack));
	Uint32* block_last = SDL_reinterpret_cast(Uint32*, SDL_const_cast(char*, haystack + needleSize - 1));

	// sequence scan
	for(size_t i = 0; i < haystackSize; i += 4, ++block_first, ++block_last)
	{
		// 0 bytes in eq indicate matching chars
		const Uint32 eq = (*block_first ^ first) | (*block_last ^ last);

		// 7th bit set if lower 7 bits are zero
		const Uint32 t0 = (~eq & 0x7f7f7f7fu) + 0x01010101u;
		// 7th bit set if 7th bit is zero
		const Uint32 t1 = (~eq & 0x80808080u);
		Uint32 zeroes = t0 & t1;

		size_t j = 0;
		while(zeroes)
		{
			if(zeroes & 0x80)
			{
				const char* substr = SDL_reinterpret_cast(char*, block_first) + j + 1;
				if(memcmp(substr, needle + 1, needleSize - 2) == 0)
				{
					result = (i + j);
					goto Continue_Result;
				}
			}

			zeroes >>= 8;
			++j;
		}
	}

	//Since we can read ahead the haystack it is possible that we find needle somewhere we don't want
	Continue_Result:
	if(result <= haystackSize - needleSize)
		return result;

	return std::string::npos;
}
#elif SIZEOF_VOIDP == 8
size_t UTIL_Faststrstr_Swar64(const char* haystack, size_t haystackSize, const char* needle, size_t needleSize)
{
	size_t result = std::string::npos;
	if(needleSize > haystackSize)
		return result;

	const Uint64 first = 0x0101010101010101llu * SDL_static_cast(Uint8, needle[0]);
	const Uint64 last = 0x0101010101010101llu * SDL_static_cast(Uint8, needle[needleSize - 1]);

	Uint64* block_first = SDL_reinterpret_cast(Uint64*, SDL_const_cast(char*, haystack));
	Uint64* block_last = SDL_reinterpret_cast(Uint64*, SDL_const_cast(char*, haystack + needleSize - 1));

	// sequence scan
	for(size_t i = 0; i < haystackSize; i += 8, ++block_first, ++block_last)
	{
		// 0 bytes in eq indicate matching chars
		const Uint64 eq = (*block_first ^ first) | (*block_last ^ last);

		// 7th bit set if lower 7 bits are zero
		const Uint64 t0 = (~eq & 0x7f7f7f7f7f7f7f7fllu) + 0x0101010101010101llu;
		// 7th bit set if 7th bit is zero
		const Uint64 t1 = (~eq & 0x8080808080808080llu);
		Uint64 zeroes = t0 & t1;

		size_t j = 0;
		while(zeroes)
		{
			if(zeroes & 0x80)
			{
				const char* substr = SDL_reinterpret_cast(char*, block_first) + j + 1;
				if(memcmp(substr, needle + 1, needleSize - 2) == 0)
				{
					result = (i + j);
					goto Continue_Result;
				}
			}

			zeroes >>= 8;
			++j;
		}
	}

	//Since we can read ahead the haystack it is possible that we find needle somewhere we don't want
	Continue_Result:
	if(result <= haystackSize - needleSize)
		return result;

	return std::string::npos;
}
#endif

#ifdef __USE_SSE2__
size_t UTIL_Faststrstr_SSE2(const char* haystack, size_t haystackSize, const char* needle, size_t needleSize)
{
	size_t result = std::string::npos;
	if(needleSize > haystackSize)
		return result;

	const __m128i first = _mm_set1_epi8(needle[0]);
	const __m128i last = _mm_set1_epi8(needle[needleSize - 1]);

	// sequence scan
	for(size_t i = 0; i < haystackSize; i += 16)
	{
		const __m128i block_first = _mm_loadu_si128(SDL_reinterpret_cast(const __m128i*, haystack + i));
		const __m128i block_last = _mm_loadu_si128(SDL_reinterpret_cast(const __m128i*, haystack + i + needleSize - 1));

		const __m128i eq_first = _mm_cmpeq_epi8(first, block_first);
		const __m128i eq_last = _mm_cmpeq_epi8(last, block_last);

		Uint32 mask = _mm_movemask_epi8(_mm_and_si128(eq_first, eq_last));
		while(mask != 0)
		{
			const Uint32 bitpos = UTIL_ctz(mask);
			if(memcmp(haystack + i + bitpos + 1, needle + 1, needleSize - 2) == 0)
			{
				result = (i + bitpos);
				goto Continue_Result;
			}

			mask &= (mask - 1);
		}
	}

	//Since we can read ahead the haystack it is possible that we find needle somewhere we don't want
	Continue_Result:
	if(result <= haystackSize - needleSize)
		return result;

	return std::string::npos;
}
#endif

#ifdef __USE_AVX2__
size_t UTIL_Faststrstr_AVX2(const char* haystack, size_t haystackSize, const char* needle, size_t needleSize)
{
	size_t result = std::string::npos;
	if(needleSize > haystackSize)
		return result;

	const __m256i first = _mm256_set1_epi8(needle[0]);
	const __m256i last = _mm256_set1_epi8(needle[needleSize - 1]);

	// sequence scan
	for(size_t i = 0; i < haystackSize; i += 32)
	{
		const __m256i block_first = _mm256_loadu_si256(SDL_reinterpret_cast(const __m256i*, haystack + i));
		const __m256i block_last = _mm256_loadu_si256(SDL_reinterpret_cast(const __m256i*, haystack + i + needleSize - 1));

		const __m256i eq_first = _mm256_cmpeq_epi8(first, block_first);
		const __m256i eq_last = _mm256_cmpeq_epi8(last, block_last);

		Uint32 mask = _mm256_movemask_epi8(_mm256_and_si256(eq_first, eq_last));
		while(mask != 0)
		{
			const Uint32 bitpos = UTIL_ctz(mask);
			if(memcmp(haystack + i + bitpos + 1, needle + 1, needleSize - 2) == 0)
			{
				result = (i + bitpos);
				goto Continue_Result;
			}

			mask &= (mask - 1);
		}
	}

	//Since we can read ahead the haystack it is possible that we find needle somewhere we don't want
	Continue_Result:
	if(result <= haystackSize - needleSize)
		return result;

	return std::string::npos;
}
#endif

void UTIL_initSubsystem()
{
	{
		#if SIZEOF_VOIDP == 4
		UTIL_Faststrstr = SDL_reinterpret_cast(LPUTIL_Faststrstr, UTIL_Faststrstr_Swar32);
		#elif SIZEOF_VOIDP == 8
		UTIL_Faststrstr = SDL_reinterpret_cast(LPUTIL_Faststrstr, UTIL_Faststrstr_Swar64);
		#else
		UTIL_Faststrstr = SDL_reinterpret_cast(LPUTIL_Faststrstr, UTIL_Faststrstr_Scalar);
		#endif
		#ifdef __USE_SSE2__
		if(SDL_HasSSE2())
		{
			UTIL_Faststrstr = SDL_reinterpret_cast(LPUTIL_Faststrstr, UTIL_Faststrstr_SSE2);
			#ifdef __USE_AVX2__
			if(SDL_HasAVX2())
				UTIL_Faststrstr = SDL_reinterpret_cast(LPUTIL_Faststrstr, UTIL_Faststrstr_AVX2);
			#endif
		}
		#endif
	}
	UTIL_FastCopy = SDL_reinterpret_cast(LPUTIL_FastCopy, UTIL_FastCopy_Standard);
	XTEA_decrypt = SDL_reinterpret_cast(LPXTEA_DECRYPT, XTEA_decrypt_scalar);
	XTEA_encrypt = SDL_reinterpret_cast(LPXTEA_ENCRYPT, XTEA_encrypt_scalar);
	adler32Checksum = SDL_reinterpret_cast(LPADLER32CHECKSUM, adler32Checksum_scalar);
	#ifdef __USE_SSE2__
	if(SDL_HasSSE2())
	{
		XTEA_decrypt = SDL_reinterpret_cast(LPXTEA_DECRYPT, XTEA_decrypt_SSE2);
		XTEA_encrypt = SDL_reinterpret_cast(LPXTEA_ENCRYPT, XTEA_encrypt_SSE2);
		adler32Checksum = SDL_reinterpret_cast(LPADLER32CHECKSUM, adler32Checksum_SSE2);
		#ifdef __USE_AVX2__
		if(SDL_HasAVX2())
		{
			XTEA_decrypt = SDL_reinterpret_cast(LPXTEA_DECRYPT, XTEA_decrypt_AVX2);
			XTEA_encrypt = SDL_reinterpret_cast(LPXTEA_ENCRYPT, XTEA_encrypt_AVX2);
		}
		#endif
	}
	#endif
	#ifdef __USE_SSE__
	if(SDL_HasSSE())
	{
		UTIL_FastCopy = SDL_reinterpret_cast(LPUTIL_FastCopy, UTIL_FastCopy_SSE);
		#ifdef __USE_SSE4_1__
		if(SDL_HasSSE41())
			UTIL_FastCopy = SDL_reinterpret_cast(LPUTIL_FastCopy, UTIL_FastCopy_SSE41);
		#endif
		//AVX doesn't help much with the speed so avoid using it
	}
	#endif
}
