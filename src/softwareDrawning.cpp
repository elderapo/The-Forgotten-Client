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

#include "softwareDrawning.h"

static const float inv255f = (1.0f / 255.0f);

LPSDL_DrawTriangle_MOD SDL_DrawTriangle_MOD;
LPSDL_SmoothStretch SDL_SmoothStretch;

struct SOFTWARE_threadData {
	SDL_Thread* thread;
	SDL_mutex* drawMutex;
	SDL_cond* drawCond;
	SDL_cond* waitCond;
	SDL_Surface* src;
	SDL_Surface* dst;
	SDL_Rect sr;
	SDL_Rect dr;
	bool waiting;
	bool drawning;
	bool exiting;
	Uint8 lightning;
};

SOFTWARE_threadData threadDatas[4];
Sint32 threadCores = 1;

typedef struct tColorRGBA {
	Uint8 r;
	Uint8 g;
	Uint8 b;
	Uint8 a;
} tColorRGBA;

#ifdef __USE_SSE__
class Edge_SSE
{
	public:
		__m128 Color1, Color2;
		Sint32 X1, Y1, X2, Y2;
		Edge_SSE(const __m128 color1, Sint32 x1, Sint32 y1, const __m128 color2, Sint32 x2, Sint32 y2)
		{
			if(y1 < y2)
			{
				Color1 = color1;
				X1 = x1;
				Y1 = y1;
				Color2 = color2;
				X2 = x2;
				Y2 = y2;
			}
			else
			{
				Color1 = color2;
				X1 = x2;
				Y1 = y2;
				Color2 = color1;
				X2 = x1;
				Y2 = y1;
			}
		}
};

class Span_SSE
{
	public:
		__m128 Color1, Color2;
		Sint32 X1, X2;
		Span_SSE(const __m128 color1, Sint32 x1, const __m128 color2, Sint32 x2)
		{
			if(x1 < x2)
			{
				Color1 = color1;
				X1 = x1;
				Color2 = color2;
				X2 = x2;
			}
			else
			{
				Color1 = color2;
				X1 = x2;
				Color2 = color1;
				X2 = x1;
			}
		}
};

void DrawSpan_SSE(SDL_Surface* dst, SDL_Rect* clip_rect, const Span_SSE &span, Sint32 y)
{
	Sint32 xdiff = (span.X2 - span.X1);
	if(xdiff == 0)
		return;

	__m128 colordiff = _mm_sub_ps(span.Color2, span.Color1);

	SDL_PixelFormat* dpf = dst->format;
	Uint32* dp = SDL_reinterpret_cast(Uint32*, SDL_reinterpret_cast(Uint8*, dst->pixels) + (y * dst->w + span.X1) * 4);

	float factor = 0.0f;
	float factorStep = 1.0f / xdiff;
	if(dpf->Amask)
	{
		for(Sint32 x = span.X1; x < span.X2; ++x)
		{
			if(SDL_static_cast(Uint32, x - clip_rect->x) < SDL_static_cast(Uint32, clip_rect->w))
			{
				__m128 c = _mm_mul_ps(_mm_add_ps(span.Color1, _mm_mul_ps(colordiff, _mm_set_ps1(factor))), _mm_set_ps1(255.0f));
				Uint32 dstR = _mm_cvtt_ss2si(c) * ((*dp >> dpf->Rshift) & 0xFF) >> 8;
				Uint32 dstG = _mm_cvtt_ss2si(_mm_shuffle_ps(c, c, _MM_SHUFFLE(1, 1, 1, 1))) * ((*dp >> dpf->Gshift) & 0xFF) >> 8;
				Uint32 dstB = _mm_cvtt_ss2si(_mm_shuffle_ps(c, c, _MM_SHUFFLE(2, 2, 2, 2))) * ((*dp >> dpf->Bshift) & 0xFF) >> 8;
				*dp = (dstR << dpf->Rshift) | (dstG << dpf->Gshift) | (dstB << dpf->Bshift) | (0xFF << dpf->Ashift);
			}
			factor += factorStep;
			++dp;
		}
	}
	else
	{
		for(Sint32 x = span.X1; x < span.X2; ++x)
		{
			if(SDL_static_cast(Uint32, x - clip_rect->x) < SDL_static_cast(Uint32, clip_rect->w))
			{
				__m128 c = _mm_mul_ps(_mm_add_ps(span.Color1, _mm_mul_ps(colordiff, _mm_set_ps1(factor))), _mm_set_ps1(255.0f));
				Uint32 dstR = _mm_cvtt_ss2si(c) * ((*dp >> dpf->Rshift) & 0xFF) >> 8;
				Uint32 dstG = _mm_cvtt_ss2si(_mm_shuffle_ps(c, c, _MM_SHUFFLE(1, 1, 1, 1))) * ((*dp >> dpf->Gshift) & 0xFF) >> 8;
				Uint32 dstB = _mm_cvtt_ss2si(_mm_shuffle_ps(c, c, _MM_SHUFFLE(2, 2, 2, 2))) * ((*dp >> dpf->Bshift) & 0xFF) >> 8;
				*dp = (dstR << dpf->Rshift) | (dstG << dpf->Gshift) | (dstB << dpf->Bshift);
			}
			factor += factorStep;
			++dp;
		}
	}
}

void DrawSpansBetweenEdges_SSE(SDL_Surface* dst, SDL_Rect* clip_rect, const Edge_SSE &e1, const Edge_SSE &e2)
{
	float e1ydiff = SDL_static_cast(float, e1.Y2 - e1.Y1);
	if(e1ydiff == 0.0f)
		return;

	float e2ydiff = SDL_static_cast(float, e2.Y2 - e2.Y1);
	if(e2ydiff == 0.0f)
		return;

	float e1xdiff = SDL_static_cast(float, e1.X2 - e1.X1);
	float e2xdiff = SDL_static_cast(float, e2.X2 - e2.X1);
	__m128 e1colordiff = _mm_sub_ps(e1.Color2, e1.Color1);
	__m128 e2colordiff = _mm_sub_ps(e2.Color2, e2.Color1);

	float factor1 = SDL_static_cast(float, e2.Y1 - e1.Y1) / e1ydiff;
	float factorStep1 = 1.0f / e1ydiff;
	float factor2 = 0.0f;
	float factorStep2 = 1.0f / e2ydiff;
	for(Sint32 y = e2.Y1; y < e2.Y2; ++y)
	{
		if(SDL_static_cast(Uint32, y - clip_rect->y) < SDL_static_cast(Uint32, clip_rect->h))
		{
			__m128 c1 = _mm_add_ps(e1.Color1, _mm_mul_ps(e1colordiff, _mm_set_ps1(factor1)));
			__m128 c2 = _mm_add_ps(e2.Color1, _mm_mul_ps(e2colordiff, _mm_set_ps1(factor2)));
			Span_SSE span(c1, e1.X1 + SDL_static_cast(Sint32, e1xdiff * factor1), c2, e2.X1 + SDL_static_cast(Sint32, e2xdiff * factor2));
			DrawSpan_SSE(dst, clip_rect, span, y);
		}
		factor1 += factorStep1;
		factor2 += factorStep2;
	}
}

void DrawTriangle_SSE(SDL_Surface* dst, const float colors[3][3], Sint32 vertices[3][2])
{
	__m128 c1 = _mm_setr_ps(colors[0][0], colors[0][1], colors[0][2], 1.0f);
	__m128 c2 = _mm_setr_ps(colors[1][0], colors[1][1], colors[1][2], 1.0f);
	__m128 c3 = _mm_setr_ps(colors[2][0], colors[2][1], colors[2][2], 1.0f);
	Edge_SSE edges[3] = {
		Edge_SSE(c1, vertices[0][0], vertices[0][1], c2, vertices[1][0], vertices[1][1]),
		Edge_SSE(c2, vertices[1][0], vertices[1][1], c3, vertices[2][0], vertices[2][1]),
		Edge_SSE(c3, vertices[2][0], vertices[2][1], c1, vertices[0][0], vertices[0][1])
	};

	Sint32 maxLength = (edges[0].Y2 - edges[0].Y1);
	Sint32 longEdge = 0;

	Sint32 length = (edges[1].Y2 - edges[1].Y1);
	if(length > maxLength)
	{
		maxLength = length;
		longEdge = 1;
	}
	length = (edges[2].Y2 - edges[2].Y1);
	if(length > maxLength)
	{
		maxLength = length;
		longEdge = 2;
	}

	SDL_Rect clip_rect;
	SDL_GetClipRect(dst, &clip_rect);

	Sint32 shortEdge1 = (longEdge + 1) % 3;
	Sint32 shortEdge2 = (longEdge + 2) % 3;
	DrawSpansBetweenEdges_SSE(dst, &clip_rect, edges[longEdge], edges[shortEdge1]);
	DrawSpansBetweenEdges_SSE(dst, &clip_rect, edges[longEdge], edges[shortEdge2]);
}

#if defined(__USE_FMA3__)
void DrawSpan_FMA3(SDL_Surface* dst, SDL_Rect* clip_rect, const Span_SSE &span, Sint32 y)
{
	Sint32 xdiff = (span.X2 - span.X1);
	if(xdiff == 0)
		return;

	__m128 colordiff = _mm_sub_ps(span.Color2, span.Color1);

	SDL_PixelFormat* dpf = dst->format;
	Uint32* dp = SDL_reinterpret_cast(Uint32*, SDL_reinterpret_cast(Uint8*, dst->pixels) + (y * dst->w + span.X1) * 4);

	float factor = 0.0f;
	float factorStep = 1.0f / xdiff;
	if(dpf->Amask)
	{
		for(Sint32 x = span.X1; x < span.X2; ++x)
		{
			if(SDL_static_cast(Uint32, x - clip_rect->x) < SDL_static_cast(Uint32, clip_rect->w))
			{
				__m128 c = _mm_mul_ps(_mm_fmadd_ps(colordiff, _mm_set_ps1(factor), span.Color1), _mm_set_ps1(255.0f));
				Uint32 dstR = _mm_cvtt_ss2si(c) * ((*dp >> dpf->Rshift) & 0xFF) >> 8;
				Uint32 dstG = _mm_cvtt_ss2si(_mm_shuffle_ps(c, c, _MM_SHUFFLE(1, 1, 1, 1))) * ((*dp >> dpf->Gshift) & 0xFF) >> 8;
				Uint32 dstB = _mm_cvtt_ss2si(_mm_shuffle_ps(c, c, _MM_SHUFFLE(2, 2, 2, 2))) * ((*dp >> dpf->Bshift) & 0xFF) >> 8;
				*dp = (dstR << dpf->Rshift) | (dstG << dpf->Gshift) | (dstB << dpf->Bshift) | (0xFF << dpf->Ashift);
			}
			factor += factorStep;
			++dp;
		}
	}
	else
	{
		for(Sint32 x = span.X1; x < span.X2; ++x)
		{
			if(SDL_static_cast(Uint32, x - clip_rect->x) < SDL_static_cast(Uint32, clip_rect->w))
			{
				__m128 c = _mm_mul_ps(_mm_fmadd_ps(colordiff, _mm_set_ps1(factor), span.Color1), _mm_set_ps1(255.0f));
				Uint32 dstR = _mm_cvtt_ss2si(c) * ((*dp >> dpf->Rshift) & 0xFF) >> 8;
				Uint32 dstG = _mm_cvtt_ss2si(_mm_shuffle_ps(c, c, _MM_SHUFFLE(1, 1, 1, 1))) * ((*dp >> dpf->Gshift) & 0xFF) >> 8;
				Uint32 dstB = _mm_cvtt_ss2si(_mm_shuffle_ps(c, c, _MM_SHUFFLE(2, 2, 2, 2))) * ((*dp >> dpf->Bshift) & 0xFF) >> 8;
				*dp = (dstR << dpf->Rshift) | (dstG << dpf->Gshift) | (dstB << dpf->Bshift);
			}
			factor += factorStep;
			++dp;
		}
	}
}

void DrawSpansBetweenEdges_FMA3(SDL_Surface* dst, SDL_Rect* clip_rect, const Edge_SSE &e1, const Edge_SSE &e2)
{
	float e1ydiff = SDL_static_cast(float, e1.Y2 - e1.Y1);
	if(e1ydiff == 0.0f)
		return;

	float e2ydiff = SDL_static_cast(float, e2.Y2 - e2.Y1);
	if(e2ydiff == 0.0f)
		return;

	float e1xdiff = SDL_static_cast(float, e1.X2 - e1.X1);
	float e2xdiff = SDL_static_cast(float, e2.X2 - e2.X1);
	__m128 e1colordiff = _mm_sub_ps(e1.Color2, e1.Color1);
	__m128 e2colordiff = _mm_sub_ps(e2.Color2, e2.Color1);

	float factor1 = SDL_static_cast(float, e2.Y1 - e1.Y1) / e1ydiff;
	float factorStep1 = 1.0f / e1ydiff;
	float factor2 = 0.0f;
	float factorStep2 = 1.0f / e2ydiff;
	for(Sint32 y = e2.Y1; y < e2.Y2; ++y)
	{
		if(SDL_static_cast(Uint32, y - clip_rect->y) < SDL_static_cast(Uint32, clip_rect->h))
		{
			__m128 c1 = _mm_fmadd_ps(e1colordiff, _mm_set_ps1(factor1), e1.Color1);
			__m128 c2 = _mm_fmadd_ps(e2colordiff, _mm_set_ps1(factor2), e2.Color1);
			Span_SSE span(c1, e1.X1 + SDL_static_cast(Sint32, e1xdiff * factor1), c2, e2.X1 + SDL_static_cast(Sint32, e2xdiff * factor2));
			DrawSpan_SSE(dst, clip_rect, span, y);
		}
		factor1 += factorStep1;
		factor2 += factorStep2;
	}
}

void DrawTriangle_FMA3(SDL_Surface* dst, const float colors[3][3], Sint32 vertices[3][2])
{
	__m128 c1 = _mm_setr_ps(colors[0][0], colors[0][1], colors[0][2], 1.0f);
	__m128 c2 = _mm_setr_ps(colors[1][0], colors[1][1], colors[1][2], 1.0f);
	__m128 c3 = _mm_setr_ps(colors[2][0], colors[2][1], colors[2][2], 1.0f);
	Edge_SSE edges[3] = {
		Edge_SSE(c1, vertices[0][0], vertices[0][1], c2, vertices[1][0], vertices[1][1]),
		Edge_SSE(c2, vertices[1][0], vertices[1][1], c3, vertices[2][0], vertices[2][1]),
		Edge_SSE(c3, vertices[2][0], vertices[2][1], c1, vertices[0][0], vertices[0][1])
	};

	Sint32 maxLength = (edges[0].Y2 - edges[0].Y1);
	Sint32 longEdge = 0;

	Sint32 length = (edges[1].Y2 - edges[1].Y1);
	if(length > maxLength)
	{
		maxLength = length;
		longEdge = 1;
	}
	length = (edges[2].Y2 - edges[2].Y1);
	if(length > maxLength)
	{
		maxLength = length;
		longEdge = 2;
	}

	SDL_Rect clip_rect;
	SDL_GetClipRect(dst, &clip_rect);

	Sint32 shortEdge1 = (longEdge + 1) % 3;
	Sint32 shortEdge2 = (longEdge + 2) % 3;
	DrawSpansBetweenEdges_SSE(dst, &clip_rect, edges[longEdge], edges[shortEdge1]);
	DrawSpansBetweenEdges_SSE(dst, &clip_rect, edges[longEdge], edges[shortEdge2]);
}
#endif

#if defined(__USE_FMA4__)
void DrawSpan_FMA4(SDL_Surface* dst, SDL_Rect* clip_rect, const Span_SSE &span, Sint32 y)
{
	Sint32 xdiff = (span.X2 - span.X1);
	if(xdiff == 0)
		return;

	__m128 colordiff = _mm_sub_ps(span.Color2, span.Color1);

	SDL_PixelFormat* dpf = dst->format;
	Uint32* dp = SDL_reinterpret_cast(Uint32*, SDL_reinterpret_cast(Uint8*, dst->pixels) + (y * dst->w + span.X1) * 4);

	float factor = 0.0f;
	float factorStep = 1.0f / xdiff;
	if(dpf->Amask)
	{
		for(Sint32 x = span.X1; x < span.X2; ++x)
		{
			if(SDL_static_cast(Uint32, x - clip_rect->x) < SDL_static_cast(Uint32, clip_rect->w))
			{
				__m128 c = _mm_mul_ps(_mm_macc_ps(colordiff, _mm_set_ps1(factor), span.Color1), _mm_set_ps1(255.0f));
				Uint32 dstR = _mm_cvtt_ss2si(c) * ((*dp >> dpf->Rshift) & 0xFF) >> 8;
				Uint32 dstG = _mm_cvtt_ss2si(_mm_shuffle_ps(c, c, _MM_SHUFFLE(1, 1, 1, 1))) * ((*dp >> dpf->Gshift) & 0xFF) >> 8;
				Uint32 dstB = _mm_cvtt_ss2si(_mm_shuffle_ps(c, c, _MM_SHUFFLE(2, 2, 2, 2))) * ((*dp >> dpf->Bshift) & 0xFF) >> 8;
				*dp = (dstR << dpf->Rshift) | (dstG << dpf->Gshift) | (dstB << dpf->Bshift) | (0xFF << dpf->Ashift);
			}
			factor += factorStep;
			++dp;
		}
	}
	else
	{
		for(Sint32 x = span.X1; x < span.X2; ++x)
		{
			if(SDL_static_cast(Uint32, x - clip_rect->x) < SDL_static_cast(Uint32, clip_rect->w))
			{
				__m128 c = _mm_mul_ps(_mm_macc_ps(colordiff, _mm_set_ps1(factor), span.Color1), _mm_set_ps1(255.0f));
				Uint32 dstR = _mm_cvtt_ss2si(c) * ((*dp >> dpf->Rshift) & 0xFF) >> 8;
				Uint32 dstG = _mm_cvtt_ss2si(_mm_shuffle_ps(c, c, _MM_SHUFFLE(1, 1, 1, 1))) * ((*dp >> dpf->Gshift) & 0xFF) >> 8;
				Uint32 dstB = _mm_cvtt_ss2si(_mm_shuffle_ps(c, c, _MM_SHUFFLE(2, 2, 2, 2))) * ((*dp >> dpf->Bshift) & 0xFF) >> 8;
				*dp = (dstR << dpf->Rshift) | (dstG << dpf->Gshift) | (dstB << dpf->Bshift);
			}
			factor += factorStep;
			++dp;
		}
	}
}

void DrawSpansBetweenEdges_FMA4(SDL_Surface* dst, SDL_Rect* clip_rect, const Edge_SSE &e1, const Edge_SSE &e2)
{
	float e1ydiff = SDL_static_cast(float, e1.Y2 - e1.Y1);
	if(e1ydiff == 0.0f)
		return;

	float e2ydiff = SDL_static_cast(float, e2.Y2 - e2.Y1);
	if(e2ydiff == 0.0f)
		return;

	float e1xdiff = SDL_static_cast(float, e1.X2 - e1.X1);
	float e2xdiff = SDL_static_cast(float, e2.X2 - e2.X1);
	__m128 e1colordiff = _mm_sub_ps(e1.Color2, e1.Color1);
	__m128 e2colordiff = _mm_sub_ps(e2.Color2, e2.Color1);

	float factor1 = SDL_static_cast(float, e2.Y1 - e1.Y1) / e1ydiff;
	float factorStep1 = 1.0f / e1ydiff;
	float factor2 = 0.0f;
	float factorStep2 = 1.0f / e2ydiff;
	for(Sint32 y = e2.Y1; y < e2.Y2; ++y)
	{
		if(SDL_static_cast(Uint32, y - clip_rect->y) < SDL_static_cast(Uint32, clip_rect->h))
		{
			__m128 c1 = _mm_macc_ps(e1colordiff, _mm_set_ps1(factor1), e1.Color1);
			__m128 c2 = _mm_macc_ps(e2colordiff, _mm_set_ps1(factor2), e2.Color1);
			Span_SSE span(c1, e1.X1 + SDL_static_cast(Sint32, e1xdiff * factor1), c2, e2.X1 + SDL_static_cast(Sint32, e2xdiff * factor2));
			DrawSpan_SSE(dst, clip_rect, span, y);
		}
		factor1 += factorStep1;
		factor2 += factorStep2;
	}
}

void DrawTriangle_FMA4(SDL_Surface* dst, const float colors[3][3], Sint32 vertices[3][2])
{
	__m128 c1 = _mm_setr_ps(colors[0][0], colors[0][1], colors[0][2], 1.0f);
	__m128 c2 = _mm_setr_ps(colors[1][0], colors[1][1], colors[1][2], 1.0f);
	__m128 c3 = _mm_setr_ps(colors[2][0], colors[2][1], colors[2][2], 1.0f);
	Edge_SSE edges[3] = {
		Edge_SSE(c1, vertices[0][0], vertices[0][1], c2, vertices[1][0], vertices[1][1]),
		Edge_SSE(c2, vertices[1][0], vertices[1][1], c3, vertices[2][0], vertices[2][1]),
		Edge_SSE(c3, vertices[2][0], vertices[2][1], c1, vertices[0][0], vertices[0][1])
	};

	Sint32 maxLength = (edges[0].Y2 - edges[0].Y1);
	Sint32 longEdge = 0;

	Sint32 length = (edges[1].Y2 - edges[1].Y1);
	if(length > maxLength)
	{
		maxLength = length;
		longEdge = 1;
	}
	length = (edges[2].Y2 - edges[2].Y1);
	if(length > maxLength)
	{
		maxLength = length;
		longEdge = 2;
	}

	SDL_Rect clip_rect;
	SDL_GetClipRect(dst, &clip_rect);

	Sint32 shortEdge1 = (longEdge + 1) % 3;
	Sint32 shortEdge2 = (longEdge + 2) % 3;
	DrawSpansBetweenEdges_SSE(dst, &clip_rect, edges[longEdge], edges[shortEdge1]);
	DrawSpansBetweenEdges_SSE(dst, &clip_rect, edges[longEdge], edges[shortEdge2]);
}
#endif
#endif

class Edge_Scalar
{
	public:
		float Color1[3], Color2[3];
		Sint32 X1, Y1, X2, Y2;
		Edge_Scalar(const float color1[3], Sint32 x1, Sint32 y1, const float color2[3], Sint32 x2, Sint32 y2)
		{
			if(y1 < y2)
			{
				Color1[0] = color1[0];
				Color1[1] = color1[1];
				Color1[2] = color1[2];
				X1 = x1;
				Y1 = y1;
				Color2[0] = color2[0];
				Color2[1] = color2[1];
				Color2[2] = color2[2];
				X2 = x2;
				Y2 = y2;
			}
			else
			{
				Color1[0] = color2[0];
				Color1[1] = color2[1];
				Color1[2] = color2[2];
				X1 = x2;
				Y1 = y2;
				Color2[0] = color1[0];
				Color2[1] = color1[1];
				Color2[2] = color1[2];
				X2 = x1;
				Y2 = y1;
			}
		}
};

class Span_Scalar
{
	public:
		float Color1[3], Color2[3];
		Sint32 X1, X2;
		Span_Scalar(const float color1[3], Sint32 x1, const float color2[3], Sint32 x2)
		{
			if(x1 < x2)
			{
				Color1[0] = color1[0];
				Color1[1] = color1[1];
				Color1[2] = color1[2];
				X1 = x1;
				Color2[0] = color2[0];
				Color2[1] = color2[1];
				Color2[2] = color2[2];
				X2 = x2;
			}
			else
			{
				Color1[0] = color2[0];
				Color1[1] = color2[1];
				Color1[2] = color2[2];
				X1 = x2;
				Color2[0] = color1[0];
				Color2[1] = color1[1];
				Color2[2] = color1[2];
				X2 = x1;
			}
		}
};

void DrawSpan_Scalar(SDL_Surface* dst, SDL_Rect* clip_rect, const Span_Scalar &span, Sint32 y)
{
	Sint32 xdiff = (span.X2 - span.X1);
	if(xdiff == 0)
		return;

	float colordiff[3];
	colordiff[0] = (span.Color2[0] - span.Color1[0]);
	colordiff[1] = (span.Color2[1] - span.Color1[1]);
	colordiff[2] = (span.Color2[2] - span.Color1[2]);

	SDL_PixelFormat* dpf = dst->format;
	Uint32* dp = SDL_reinterpret_cast(Uint32*, SDL_reinterpret_cast(Uint8*, dst->pixels) + (y * dst->w + span.X1) * 4);

	float factor = 0.0f;
	float factorStep = 1.0f / xdiff;
	if(dpf->Amask)
	{
		for(Sint32 x = span.X1; x < span.X2; ++x)
		{
			if(SDL_static_cast(Uint32, x - clip_rect->x) < SDL_static_cast(Uint32, clip_rect->w))
			{
				Uint32 dstR = SDL_static_cast(Sint32, (span.Color1[0] + (colordiff[0] * factor)) * 255.0f) * ((*dp >> dpf->Rshift) & 0xFF) >> 8;
				Uint32 dstG = SDL_static_cast(Sint32, (span.Color1[1] + (colordiff[1] * factor)) * 255.0f) * ((*dp >> dpf->Gshift) & 0xFF) >> 8;
				Uint32 dstB = SDL_static_cast(Sint32, (span.Color1[2] + (colordiff[2] * factor)) * 255.0f) * ((*dp >> dpf->Bshift) & 0xFF) >> 8;
				*dp = (dstR << dpf->Rshift) | (dstG << dpf->Gshift) | (dstB << dpf->Bshift) | (0xFF << dpf->Ashift);
			}
			factor += factorStep;
			++dp;
		}
	}
	else
	{
		for(Sint32 x = span.X1; x < span.X2; ++x)
		{
			if(SDL_static_cast(Uint32, x - clip_rect->x) < SDL_static_cast(Uint32, clip_rect->w))
			{
				Uint32 dstR = SDL_static_cast(Sint32, (span.Color1[0] + (colordiff[0] * factor)) * 255.0f) * ((*dp >> dpf->Rshift) & 0xFF) >> 8;
				Uint32 dstG = SDL_static_cast(Sint32, (span.Color1[1] + (colordiff[1] * factor)) * 255.0f) * ((*dp >> dpf->Gshift) & 0xFF) >> 8;
				Uint32 dstB = SDL_static_cast(Sint32, (span.Color1[2] + (colordiff[2] * factor)) * 255.0f) * ((*dp >> dpf->Bshift) & 0xFF) >> 8;
				*dp = (dstR << dpf->Rshift) | (dstG << dpf->Gshift) | (dstB << dpf->Bshift);
			}
			factor += factorStep;
			++dp;
		}
	}
}

void DrawSpansBetweenEdges_Scalar(SDL_Surface* dst, SDL_Rect* clip_rect, const Edge_Scalar &e1, const Edge_Scalar &e2)
{
	float e1ydiff = SDL_static_cast(float, e1.Y2 - e1.Y1);
	if(e1ydiff == 0.0f)
		return;

	float e2ydiff = SDL_static_cast(float, e2.Y2 - e2.Y1);
	if(e2ydiff == 0.0f)
		return;

	float e1xdiff = SDL_static_cast(float, e1.X2 - e1.X1);
	float e2xdiff = SDL_static_cast(float, e2.X2 - e2.X1);
	float e1colordiff[3];
	e1colordiff[0] = (e1.Color2[0] - e1.Color1[0]);
	e1colordiff[1] = (e1.Color2[1] - e1.Color1[1]);
	e1colordiff[2] = (e1.Color2[2] - e1.Color1[2]);
	float e2colordiff[3];
	e2colordiff[0] = (e2.Color2[0] - e2.Color1[0]);
	e2colordiff[1] = (e2.Color2[1] - e2.Color1[1]);
	e2colordiff[2] = (e2.Color2[2] - e2.Color1[2]);

	float factor1 = SDL_static_cast(float, e2.Y1 - e1.Y1) / e1ydiff;
	float factorStep1 = 1.0f / e1ydiff;
	float factor2 = 0.0f;
	float factorStep2 = 1.0f / e2ydiff;
	for(Sint32 y = e2.Y1; y < e2.Y2; ++y)
	{
		if(SDL_static_cast(Uint32, y - clip_rect->y) < SDL_static_cast(Uint32, clip_rect->h))
		{
			float c1[3];
			c1[0] = e1.Color1[0] + (e1colordiff[0] * factor1);
			c1[1] = e1.Color1[1] + (e1colordiff[1] * factor1);
			c1[2] = e1.Color1[2] + (e1colordiff[2] * factor1);
			float c2[3];
			c2[0] = e2.Color1[0] + (e2colordiff[0] * factor2);
			c2[1] = e2.Color1[1] + (e2colordiff[1] * factor2);
			c2[2] = e2.Color1[2] + (e2colordiff[2] * factor2);
			Span_Scalar span(c1, e1.X1 + SDL_static_cast(Sint32, e1xdiff * factor1), c2, e2.X1 + SDL_static_cast(Sint32, e2xdiff * factor2));
			DrawSpan_Scalar(dst, clip_rect, span, y);
		}
		factor1 += factorStep1;
		factor2 += factorStep2;
	}
}

void DrawTriangle_Scalar(SDL_Surface* dst, const float colors[3][3], Sint32 vertices[3][2])
{
	Edge_Scalar edges[3] = {
		Edge_Scalar(colors[0], vertices[0][0], vertices[0][1], colors[1], vertices[1][0], vertices[1][1]),
		Edge_Scalar(colors[1], vertices[1][0], vertices[1][1], colors[2], vertices[2][0], vertices[2][1]),
		Edge_Scalar(colors[2], vertices[2][0], vertices[2][1], colors[0], vertices[0][0], vertices[0][1])
	};

	Sint32 maxLength = (edges[0].Y2 - edges[0].Y1);
	Sint32 longEdge = 0;

	Sint32 length = (edges[1].Y2 - edges[1].Y1);
	if(length > maxLength)
	{
		maxLength = length;
		longEdge = 1;
	}
	length = (edges[2].Y2 - edges[2].Y1);
	if(length > maxLength)
	{
		maxLength = length;
		longEdge = 2;
	}

	SDL_Rect clip_rect;
	SDL_GetClipRect(dst, &clip_rect);

	Sint32 shortEdge1 = (longEdge + 1) % 3;
	Sint32 shortEdge2 = (longEdge + 2) % 3;
	DrawSpansBetweenEdges_Scalar(dst, &clip_rect, edges[longEdge], edges[shortEdge1]);
	DrawSpansBetweenEdges_Scalar(dst, &clip_rect, edges[longEdge], edges[shortEdge2]);
}

Sint32 SDL_FillRect_BLEND(SDL_Surface* dst, SDL_Rect* dstrect, Uint32 RGBA, Uint32 A)
{
	if(!dstrect)
	{
		SDL_Rect dstr = {0,0,dst->w,dst->h};
		dstrect = &dstr;
	}
	else
	{
		SDL_Rect clip_rect;
		SDL_GetClipRect(dst, &clip_rect);

		Sint32 dx, dy;
		dx = clip_rect.x - dstrect->x;
		if(dx > 0)
		{
			dstrect->w -= dx;
			dstrect->x += dx;
		}

		dx = dstrect->x + dstrect->w - clip_rect.x - clip_rect.w;
		if(dx > 0)
			dstrect->w -= dx;

		dy = clip_rect.y - dstrect->y;
		if(dy > 0)
		{
			dstrect->h -= dy;
			dstrect->y += dy;
		}

		dy = dstrect->y + dstrect->h - clip_rect.y - clip_rect.h;
		if(dy > 0)
			dstrect->h -= dy;

		if(dstrect->w <= 0 || dstrect->h <= 0)
			return 0;
	}

	SDL_PixelFormat* dpf = dst->format;
	Uint32* dp = SDL_reinterpret_cast(Uint32*, SDL_reinterpret_cast(Uint8*, dst->pixels) + (dstrect->y * dst->w + dstrect->x) * 4);
	Sint32 dgap = (dst->pitch / 4) - dstrect->w;

	Sint32 xEnd = dstrect->w, yEnd = dstrect->h;
	if((dpf->Rmask|dpf->Gmask|dpf->Bmask) == 0x00FFFFFF)//We can go faster patch
	{
		if(A == 128)//Greatly optimized 50% semi-transparency
		{
			for(Sint32 y = 0; y < yEnd; ++y)
			{
				for(Sint32 x = 0; x < xEnd; ++x)
				{
					Uint32 d = *dp;
					*dp++ = (((RGBA & 0x00fefefe) + (d & 0x00fefefe)) >> 1) + (RGBA & d & 0x00010101);
				}
				dp += dgap;
			}
		}
		else
		{
			for(Sint32 y = 0; y < yEnd; ++y)
			{
				for(Sint32 x = 0; x < xEnd; ++x)
				{
					Uint32 s = RGBA;
					Uint32 d = *dp;
					Uint32 s1 = s & 0xff00ff;
					Uint32 d1 = d & 0xff00ff;
					d1 = (d1 + ((s1 - d1) * A >> 8)) & 0xff00ff;
					s &= 0xff00;
					d &= 0xff00;
					d = (d + ((s - d) * A >> 8)) & 0xff00;
					*dp++ = d1 | d;
				}
				dp += dgap;
			}
		}
	}
	else
	{
		if(dpf->Amask)
		{
			for(Sint32 y = 0; y < yEnd; ++y)
			{
				for(Sint32 x = 0; x < xEnd; ++x)
				{
					Uint32 dstR = ((*dp >> dpf->Rshift) & 0xFF);
					Uint32 dstG = ((*dp >> dpf->Gshift) & 0xFF);
					Uint32 dstB = ((*dp >> dpf->Bshift) & 0xFF);
					dstR += (((RGBA >> dpf->Rshift) & 0xFF) - dstR) * A >> 8;
					dstG += (((RGBA >> dpf->Gshift) & 0xFF) - dstG) * A >> 8;
					dstB += (((RGBA >> dpf->Bshift) & 0xFF) - dstB) * A >> 8;
					*dp++ = (dstR << dpf->Rshift) | (dstG << dpf->Gshift) | (dstB << dpf->Bshift) | (0xFF << dpf->Ashift);
				}
				dp += dgap;
			}
		}
		else
		{
			for(Sint32 y = 0; y < yEnd; ++y)
			{
				for(Sint32 x = 0; x < xEnd; ++x)
				{
					Uint32 dstR = ((*dp >> dpf->Rshift) & 0xFF);
					Uint32 dstG = ((*dp >> dpf->Gshift) & 0xFF);
					Uint32 dstB = ((*dp >> dpf->Bshift) & 0xFF);
					dstR += (((RGBA >> dpf->Rshift) & 0xFF) - dstR) * A >> 8;
					dstG += (((RGBA >> dpf->Gshift) & 0xFF) - dstG) * A >> 8;
					dstB += (((RGBA >> dpf->Bshift) & 0xFF) - dstB) * A >> 8;
					*dp++ = (dstR << dpf->Rshift) | (dstG << dpf->Gshift) | (dstB << dpf->Bshift);
				}
				dp += dgap;
			}
		}
	}
	return 0;
}

Sint32 SDL_FillRect_MOD(SDL_Surface* dst, SDL_Rect* dstrect, Uint32 R, Uint32 G, Uint32 B)
{
	if((R & G & B) == 255)
		return 0;
	if(!dstrect)
	{
		SDL_Rect dstr = {0,0,dst->w,dst->h};
		dstrect = &dstr;
	}
	else
	{
		SDL_Rect clip_rect;
		SDL_GetClipRect(dst, &clip_rect);

		Sint32 dx, dy;
		dx = clip_rect.x - dstrect->x;
		if(dx > 0)
		{
			dstrect->w -= dx;
			dstrect->x += dx;
		}

		dx = dstrect->x + dstrect->w - clip_rect.x - clip_rect.w;
		if(dx > 0)
			dstrect->w -= dx;

		dy = clip_rect.y - dstrect->y;
		if(dy > 0)
		{
			dstrect->h -= dy;
			dstrect->y += dy;
		}

		dy = dstrect->y + dstrect->h - clip_rect.y - clip_rect.h;
		if(dy > 0)
			dstrect->h -= dy;

		if(dstrect->w <= 0 || dstrect->h <= 0)
			return 0;
	}

	SDL_PixelFormat* dpf = dst->format;
	Uint32* dp = SDL_reinterpret_cast(Uint32*, SDL_reinterpret_cast(Uint8*, dst->pixels) + (dstrect->y * dst->w + dstrect->x) * 4);
	Sint32 dgap = (dst->pitch / 4) - dstrect->w;

	Sint32 xEnd = dstrect->w, yEnd = dstrect->h;
	if(dpf->Amask)
	{
		for(Sint32 y = 0; y < yEnd; ++y)
		{
			for(Sint32 x = 0; x < xEnd; ++x)
			{
				Uint32 dstR = R * ((*dp >> dpf->Rshift) & 0xFF) >> 8;
				Uint32 dstG = G * ((*dp >> dpf->Gshift) & 0xFF) >> 8;
				Uint32 dstB = B * ((*dp >> dpf->Bshift) & 0xFF) >> 8;
				*dp++ = (dstR << dpf->Rshift) | (dstG << dpf->Gshift) | (dstB << dpf->Bshift) | (0xFF << dpf->Ashift);
			}
			dp += dgap;
		}
	}
	else
	{
		for(Sint32 y = 0; y < yEnd; ++y)
		{
			for(Sint32 x = 0; x < xEnd; ++x)
			{
				Uint32 dstR = R * ((*dp >> dpf->Rshift) & 0xFF) >> 8;
				Uint32 dstG = G * ((*dp >> dpf->Gshift) & 0xFF) >> 8;
				Uint32 dstB = B * ((*dp >> dpf->Bshift) & 0xFF) >> 8;
				*dp++ = (dstR << dpf->Rshift) | (dstG << dpf->Gshift) | (dstB << dpf->Bshift);
			}
			dp += dgap;
		}
	}
	return 0;
}

#ifdef __USE_SSE2__
SDL_FORCE_INLINE __m128i _sym_mm_mullo_epu32(__m128i a, __m128i b)
{
	__m128i dest02 = _mm_mul_epu32(a, b);
	__m128i dest13 = _mm_mul_epu32(_mm_shuffle_epi32(a, 0xF5), _mm_shuffle_epi32(b, 0xF5));
	return _mm_unpacklo_epi32(_mm_shuffle_epi32(dest02, 0xD8), _mm_shuffle_epi32(dest13, 0xD8));
}

Sint32 SDL_SmoothStretch_SSE2(SDL_Surface* src, SDL_Rect* srcrect, SDL_Surface* dst, SDL_Rect* dstrect)
{
	if(!srcrect)
	{
		SDL_Rect srcr = {0,0,src->w,src->h};
		srcrect = &srcr;
	}
	else
	{
		if(srcrect->x < 0 || srcrect->y < 0 || srcrect->x + srcrect->w > src->w || srcrect->y + srcrect->h > src->h)
			return -1;
	}

	if(!dstrect)
	{
		SDL_Rect dstr = {0,0,dst->w,dst->h};
		dstrect = &dstr;
	}
	else
	{
		if(dstrect->x < 0 || dstrect->y < 0 || dstrect->x + dstrect->w > dst->w || dstrect->y + dstrect->h > dst->h)
			return -1;
	}

	Sint32 salast;
	const __m128i s255 = _mm_set1_epi32(0xFF);

	Sint32 spixelw = (srcrect->w - 1);
	Sint32 spixelh = (srcrect->h - 1);
	Sint32 sx = SDL_static_cast(Sint32, 65536.0f * spixelw / (dstrect->w - 1));
	Sint32 sy = SDL_static_cast(Sint32, 65536.0f * spixelh / (dstrect->h - 1));

	Sint32 ssx = (srcrect->w << 16) - 1;
	Sint32 ssy = (srcrect->h << 16) - 1;

	Uint32* sp = SDL_reinterpret_cast(Uint32*, SDL_reinterpret_cast(Uint8*, src->pixels) + (srcrect->y * src->w + srcrect->x) * 4);
	Sint32* dp = SDL_reinterpret_cast(Sint32*, SDL_reinterpret_cast(Uint8*, dst->pixels) + (dstrect->y * dst->w + dstrect->x) * 4);

	Sint32 dgap = dst->pitch / 4;
	Sint32 spixelgap = src->pitch / 4;

	Sint32 csay = 0;
	Sint32 xEnd = dstrect->w, yEnd = dstrect->h;
	for(Sint32 y = 0; y < yEnd; ++y)
	{
		Sint32* cdp = dp;
		Uint32* csp = sp;
		Sint32 csax = 0;
		for(Sint32 x = 0; x < xEnd; ++x)
		{
			Sint32 ex = (csax & 0xFFFF);
			Sint32 ey = (csay & 0xFFFF);
			Sint32 cx = (csax >> 16);
			Sint32 cy = (csay >> 16);
			bool sstepx = (cx < spixelw);
			Uint32* c00 = sp;
			Uint32* c01 = (sstepx ? c00 + 1 : c00);
			Uint32* c10 = (cy < spixelh ? c00 + spixelgap : c00);
			Uint32* c11 = (sstepx ? c10 + 1 : c10);

			const __m128i sex = _mm_set1_epi32(ex);
			const __m128i sc00 = _mm_unpacklo_epi16(_mm_unpacklo_epi8(_mm_cvtsi32_si128(*c00), _mm_setzero_si128()), _mm_setzero_si128());
			const __m128i sc01 = _mm_unpacklo_epi16(_mm_unpacklo_epi8(_mm_cvtsi32_si128(*c01), _mm_setzero_si128()), _mm_setzero_si128());
			const __m128i sc10 = _mm_unpacklo_epi16(_mm_unpacklo_epi8(_mm_cvtsi32_si128(*c10), _mm_setzero_si128()), _mm_setzero_si128());
			const __m128i sc11 = _mm_unpacklo_epi16(_mm_unpacklo_epi8(_mm_cvtsi32_si128(*c11), _mm_setzero_si128()), _mm_setzero_si128());
			__m128i r1 = _mm_and_si128(_mm_add_epi32(_mm_srli_epi32(_sym_mm_mullo_epu32(_mm_sub_epi32(sc01, sc00), sex), 16), sc00), s255);
			__m128i r2 = _mm_and_si128(_mm_add_epi32(_mm_srli_epi32(_sym_mm_mullo_epu32(_mm_sub_epi32(sc11, sc10), sex), 16), sc10), s255);
			__m128i r = _mm_and_si128(_mm_add_epi32(_mm_srli_epi32(_sym_mm_mullo_epu32(_mm_sub_epi32(r2, r1), _mm_set1_epi32(ey)), 16), r1), s255);
			r = _mm_packs_epi32(r, r);
			_mm_stream_si32(dp, _mm_cvtsi128_si32(_mm_packus_epi16(r, r)));

			salast = csax;
			csax += sx;
			csax = (csax > ssx ? ssx : csax);
			sp += ((csax >> 16) - (salast >> 16));
			++dp;
		}
		salast = csay;
		csay += sy;
		csay = (csay > ssy ? ssy : csay);
		sp = csp + (((csay >> 16) - (salast >> 16)) * spixelgap);
		dp = cdp + dgap;
	}
	_mm_sfence();
	return 0;
}
#endif

#ifdef __USE_SSSE3__
Sint32 SDL_SmoothStretch_SSSE3(SDL_Surface* src, SDL_Rect* srcrect, SDL_Surface* dst, SDL_Rect* dstrect)
{
	if(!srcrect)
	{
		SDL_Rect srcr = {0,0,src->w,src->h};
		srcrect = &srcr;
	}
	else
	{
		if(srcrect->x < 0 || srcrect->y < 0 || srcrect->x + srcrect->w > src->w || srcrect->y + srcrect->h > src->h)
			return -1;
	}

	if(!dstrect)
	{
		SDL_Rect dstr = {0,0,dst->w,dst->h};
		dstrect = &dstr;
	}
	else
	{
		if(dstrect->x < 0 || dstrect->y < 0 || dstrect->x + dstrect->w > dst->w || dstrect->y + dstrect->h > dst->h)
			return -1;
	}

	Sint32 salast;
	const __m128i s255 = _mm_set1_epi32(0xFF);
	const __m128i loadMask = _mm_set_epi8(15, 14, 13, 3, 12, 11, 10, 2, 9, 8, 7, 1, 6, 5, 4, 0);
	const __m128i shuffleMask = _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 8, 4, 0);

	Sint32 spixelw = (srcrect->w - 1);
	Sint32 spixelh = (srcrect->h - 1);
	Sint32 sx = SDL_static_cast(Sint32, 65536.0f * spixelw / (dstrect->w - 1));
	Sint32 sy = SDL_static_cast(Sint32, 65536.0f * spixelh / (dstrect->h - 1));

	Sint32 ssx = (srcrect->w << 16) - 1;
	Sint32 ssy = (srcrect->h << 16) - 1;

	Uint32* sp = SDL_reinterpret_cast(Uint32*, SDL_reinterpret_cast(Uint8*, src->pixels) + (srcrect->y * src->w + srcrect->x) * 4);
	Sint32* dp = SDL_reinterpret_cast(Sint32*, SDL_reinterpret_cast(Uint8*, dst->pixels) + (dstrect->y * dst->w + dstrect->x) * 4);

	Sint32 dgap = dst->pitch / 4;
	Sint32 spixelgap = src->pitch / 4;

	Sint32 csay = 0;
	Sint32 xEnd = dstrect->w, yEnd = dstrect->h;
	for(Sint32 y = 0; y < yEnd; ++y)
	{
		Sint32* cdp = dp;
		Uint32* csp = sp;
		Sint32 csax = 0;
		for(Sint32 x = 0; x < xEnd; ++x)
		{
			Sint32 ex = (csax & 0xFFFF);
			Sint32 ey = (csay & 0xFFFF);
			Sint32 cx = (csax >> 16);
			Sint32 cy = (csay >> 16);
			bool sstepx = (cx < spixelw);
			Uint32* c00 = sp;
			Uint32* c01 = (sstepx ? c00 + 1 : c00);
			Uint32* c10 = (cy < spixelh ? c00 + spixelgap : c00);
			Uint32* c11 = (sstepx ? c10 + 1 : c10);

			const __m128i sex = _mm_set1_epi32(ex);
			const __m128i sc00 = _mm_shuffle_epi8(_mm_cvtsi32_si128(*c00), loadMask);
			const __m128i sc01 = _mm_shuffle_epi8(_mm_cvtsi32_si128(*c01), loadMask);
			const __m128i sc10 = _mm_shuffle_epi8(_mm_cvtsi32_si128(*c10), loadMask);
			const __m128i sc11 = _mm_shuffle_epi8(_mm_cvtsi32_si128(*c11), loadMask);
			__m128i r1 = _mm_and_si128(_mm_add_epi32(_mm_srli_epi32(_sym_mm_mullo_epu32(_mm_sub_epi32(sc01, sc00), sex), 16), sc00), s255);
			__m128i r2 = _mm_and_si128(_mm_add_epi32(_mm_srli_epi32(_sym_mm_mullo_epu32(_mm_sub_epi32(sc11, sc10), sex), 16), sc10), s255);
			__m128i r = _mm_add_epi32(_mm_srli_epi32(_sym_mm_mullo_epu32(_mm_sub_epi32(r2, r1), _mm_set1_epi32(ey)), 16), r1);
			_mm_stream_si32(dp, _mm_cvtsi128_si32(_mm_shuffle_epi8(r, shuffleMask)));

			salast = csax;
			csax += sx;
			csax = (csax > ssx ? ssx : csax);
			sp += ((csax >> 16) - (salast >> 16));
			++dp;
		}
		salast = csay;
		csay += sy;
		csay = (csay > ssy ? ssy : csay);
		sp = csp + (((csay >> 16) - (salast >> 16)) * spixelgap);
		dp = cdp + dgap;
	}
	_mm_sfence();
	return 0;
}
#endif

#ifdef __USE_SSE4_1__
Sint32 SDL_SmoothStretch_SSE41(SDL_Surface* src, SDL_Rect* srcrect, SDL_Surface* dst, SDL_Rect* dstrect)
{
	if(!srcrect)
	{
		SDL_Rect srcr = {0,0,src->w,src->h};
		srcrect = &srcr;
	}
	else
	{
		if(srcrect->x < 0 || srcrect->y < 0 || srcrect->x + srcrect->w > src->w || srcrect->y + srcrect->h > src->h)
			return -1;
	}

	if(!dstrect)
	{
		SDL_Rect dstr = {0,0,dst->w,dst->h};
		dstrect = &dstr;
	}
	else
	{
		if(dstrect->x < 0 || dstrect->y < 0 || dstrect->x + dstrect->w > dst->w || dstrect->y + dstrect->h > dst->h)
			return -1;
	}

	Sint32 salast;
	const __m128i shuffleMask = _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 8, 4, 0);
	const __m128i s255 = _mm_set1_epi32(0xFF);

	Sint32 spixelw = (srcrect->w - 1);
	Sint32 spixelh = (srcrect->h - 1);
	Sint32 sx = SDL_static_cast(Sint32, 65536.0f * spixelw / (dstrect->w - 1));
	Sint32 sy = SDL_static_cast(Sint32, 65536.0f * spixelh / (dstrect->h - 1));

	Sint32 ssx = (srcrect->w << 16) - 1;
	Sint32 ssy = (srcrect->h << 16) - 1;

	Uint32* sp = SDL_reinterpret_cast(Uint32*, SDL_reinterpret_cast(Uint8*, src->pixels) + (srcrect->y * src->w + srcrect->x) * 4);
	Sint32* dp = SDL_reinterpret_cast(Sint32*, SDL_reinterpret_cast(Uint8*, dst->pixels) + (dstrect->y * dst->w + dstrect->x) * 4);

	Sint32 dgap = dst->pitch / 4;
	Sint32 spixelgap = src->pitch / 4;

	Sint32 csay = 0;
	Sint32 xEnd = dstrect->w, yEnd = dstrect->h;
	for(Sint32 y = 0; y < yEnd; ++y)
	{
		Sint32* cdp = dp;
		Uint32* csp = sp;
		Sint32 csax = 0;
		for(Sint32 x = 0; x < xEnd; ++x)
		{
			Sint32 ex = (csax & 0xFFFF);
			Sint32 ey = (csay & 0xFFFF);
			Sint32 cx = (csax >> 16);
			Sint32 cy = (csay >> 16);
			bool sstepx = (cx < spixelw);
			Uint32* c00 = sp;
			Uint32* c01 = (sstepx ? c00 + 1 : c00);
			Uint32* c10 = (cy < spixelh ? c00 + spixelgap : c00);
			Uint32* c11 = (sstepx ? c10 + 1 : c10);

			const __m128i sex = _mm_set1_epi32(ex);
			const __m128i sc00 = _mm_cvtepu8_epi32(_mm_cvtsi32_si128(*c00));
			const __m128i sc01 = _mm_cvtepu8_epi32(_mm_cvtsi32_si128(*c01));
			const __m128i sc10 = _mm_cvtepu8_epi32(_mm_cvtsi32_si128(*c10));
			const __m128i sc11 = _mm_cvtepu8_epi32(_mm_cvtsi32_si128(*c11));
			__m128i r1 = _mm_and_si128(_mm_add_epi32(_mm_srli_epi32(_mm_mullo_epi32(_mm_sub_epi32(sc01, sc00), sex), 16), sc00), s255);
			__m128i r2 = _mm_and_si128(_mm_add_epi32(_mm_srli_epi32(_mm_mullo_epi32(_mm_sub_epi32(sc11, sc10), sex), 16), sc10), s255);
			__m128i r = _mm_add_epi32(_mm_srli_epi32(_mm_mullo_epi32(_mm_sub_epi32(r2, r1), _mm_set1_epi32(ey)), 16), r1);
			_mm_stream_si32(dp, _mm_cvtsi128_si32(_mm_shuffle_epi8(r, shuffleMask)));

			salast = csax;
			csax += sx;
			csax = (csax > ssx ? ssx : csax);
			sp += ((csax >> 16) - (salast >> 16));
			++dp;
		}
		salast = csay;
		csay += sy;
		csay = (csay > ssy ? ssy : csay);
		sp = csp + (((csay >> 16) - (salast >> 16)) * spixelgap);
		dp = cdp + dgap;
	}
	_mm_sfence();
	return 0;
}
#endif

Sint32 SDL_SmoothStretch_scalar(SDL_Surface* src, SDL_Rect* srcrect, SDL_Surface* dst, SDL_Rect* dstrect)
{
	if(!srcrect)
	{
		SDL_Rect srcr = {0,0,src->w,src->h};
		srcrect = &srcr;
	}
	else
	{
		if(srcrect->x < 0 || srcrect->y < 0 || srcrect->x + srcrect->w > src->w || srcrect->y + srcrect->h > src->h)
			return -1;
	}

	if(!dstrect)
	{
		SDL_Rect dstr = {0,0,dst->w,dst->h};
		dstrect = &dstr;
	}
	else
	{
		if(dstrect->x < 0 || dstrect->y < 0 || dstrect->x + dstrect->w > dst->w || dstrect->y + dstrect->h > dst->h)
			return -1;
	}

	Sint32 salast;
	Sint32 spixelw = (srcrect->w - 1);
	Sint32 spixelh = (srcrect->h - 1);
	Sint32 sx = SDL_static_cast(Sint32, 65536.0f * spixelw / (dstrect->w - 1));
	Sint32 sy = SDL_static_cast(Sint32, 65536.0f * spixelh / (dstrect->h - 1));

	Sint32 ssx = (srcrect->w << 16) - 1;
	Sint32 ssy = (srcrect->h << 16) - 1;

	tColorRGBA* sp = SDL_reinterpret_cast(tColorRGBA*, SDL_reinterpret_cast(Uint8*, src->pixels) + (srcrect->y * src->w + srcrect->x) * 4);
	tColorRGBA* dp = SDL_reinterpret_cast(tColorRGBA*, SDL_reinterpret_cast(Uint8*, dst->pixels) + (dstrect->y * dst->w + dstrect->x) * 4);

	Sint32 dgap = dst->pitch / 4;
	Sint32 spixelgap = src->pitch / 4;

	Sint32 csay = 0;
	Sint32 xEnd = dstrect->w, yEnd = dstrect->h;
	for(Sint32 y = 0; y < yEnd; ++y)
	{
		tColorRGBA* cdp = dp;
		tColorRGBA* csp = sp;
		Sint32 csax = 0;
		for(Sint32 x = 0; x < xEnd; ++x)
		{
			Sint32 ex = (csax & 0xFFFF);
			Sint32 ey = (csay & 0xFFFF);
			Sint32 cx = (csax >> 16);
			Sint32 cy = (csay >> 16);
			bool sstepx = (cx < spixelw);
			tColorRGBA* c00 = sp;
			tColorRGBA* c01 = (sstepx ? c00 + 1 : c00);
			tColorRGBA* c10 = (cy < spixelh ? c00 + spixelgap : c00);
			tColorRGBA* c11 = (sstepx ? c10 + 1 : c10);

			Sint32 t1 = ((((c01->r - c00->r) * ex) >> 16) + c00->r) & 0xFF;
			Sint32 t2 = ((((c11->r - c10->r) * ex) >> 16) + c10->r) & 0xFF;
			dp->r = SDL_static_cast(Uint8, (((t2 - t1) * ey) >> 16) + t1);
			t1 = ((((c01->g - c00->g) * ex) >> 16) + c00->g) & 0xFF;
			t2 = ((((c11->g - c10->g) * ex) >> 16) + c10->g) & 0xFF;
			dp->g = SDL_static_cast(Uint8, (((t2 - t1) * ey) >> 16) + t1);
			t1 = ((((c01->b - c00->b) * ex) >> 16) + c00->b) & 0xFF;
			t2 = ((((c11->b - c10->b) * ex) >> 16) + c10->b) & 0xFF;
			dp->b = SDL_static_cast(Uint8, (((t2 - t1) * ey) >> 16) + t1);
			t1 = ((((c01->a - c00->a) * ex) >> 16) + c00->a) & 0xFF;
			t2 = ((((c11->a - c10->a) * ex) >> 16) + c10->a) & 0xFF;
			dp->a = SDL_static_cast(Uint8, (((t2 - t1) * ey) >> 16) + t1);

			salast = csax;
			csax += sx;
			csax = (csax > ssx ? ssx : csax);
			sp += ((csax >> 16) - (salast >> 16));
			++dp;
		}
		salast = csay;
		csay += sy;
		csay = (csay > ssy ? ssy : csay);
		sp = csp + (((csay >> 16) - (salast >> 16)) * spixelgap);
		dp = SDL_reinterpret_cast(tColorRGBA*, cdp + dgap);
	}
	return 0;
}

void SDL_DrawLightMap_old(SDL_Surface* src, LightMap* lightmap, Sint32 x, Sint32 y, Sint32 scale, Sint32 width, Sint32 start, Sint32 end)
{
	float colors[3][3];
	Sint32 vertices[3][2];

	Sint32 drawY = y - (scale / 2);
	for(Sint32 j = start; j < end; ++j)
	{
		Sint32 offset1 = (j + 1) * width;
		Sint32 offset2 = UTIL_max<Sint32>(j, 0) * width;
		Sint32 drawX = x - scale - (scale / 2);
		for(Sint32 k = -1; k < width - 1; ++k)
		{
			Sint32 offset = UTIL_max<Sint32>(k, 0);
			vertices[0][0] = drawX; vertices[0][1] = (drawY + scale);
			colors[0][0] = lightmap[offset1 + offset].r * inv255f; colors[0][1] = lightmap[offset1 + offset].g * inv255f; colors[0][2] = lightmap[offset1 + offset].b * inv255f;
			vertices[1][0] = drawX; vertices[1][1] = drawY;
			colors[1][0] = lightmap[offset2 + offset].r * inv255f; colors[1][1] = lightmap[offset2 + offset].g * inv255f; colors[1][2] = lightmap[offset2 + offset].b * inv255f;
			drawX += scale;
			vertices[2][0] = drawX; vertices[2][1] = (drawY + scale);
			colors[2][0] = lightmap[offset1 + k + 1].r * inv255f; colors[2][1] = lightmap[offset1 + k + 1].g * inv255f; colors[2][2] = lightmap[offset1 + k + 1].b * inv255f;
			SDL_DrawTriangle_MOD(src, colors, vertices);
			vertices[0][0] = drawX; vertices[0][1] = drawY;
			colors[0][0] = lightmap[offset2 + k + 1].r * inv255f; colors[0][1] = lightmap[offset2 + k + 1].g * inv255f; colors[0][2] = lightmap[offset2 + k + 1].b * inv255f;
			SDL_DrawTriangle_MOD(src, colors, vertices);
		}
		drawY += scale;
	}
}

void SDL_DrawLightMap_new(SDL_Surface* src, LightMap* lightmap, Sint32 x, Sint32 y, Sint32 scale, Sint32 width, Sint32 start, Sint32 end)
{
	float colors[3][3];
	Sint32 vertices[3][2];

	Sint32 halfScale = (scale / 2);
	Sint32 drawY = y - halfScale;
	for(Sint32 j = start; j < end; ++j)
	{
		Sint32 offset1 = (j + 1) * width;
		Sint32 offset2 = UTIL_max<Sint32>(j, 0) * width;
		Sint32 drawX = x - scale - halfScale;
		for(Sint32 k = -1; k < width - 1; ++k)
		{
			Sint32 offset = UTIL_max<Sint32>(k, 0);

			//Get the colors of tile corners
			float topCenter[2][3], leftCenter[2][3], bottomCenter[2][3], rightCenter[2][3], center[2][3], topLeft[2][3], topRight[2][3], bottomLeft[2][3], bottomRight[2][3];
			topLeft[0][0] = lightmap[offset2 + offset].r * inv255f;
			topLeft[0][1] = lightmap[offset2 + offset].g * inv255f;
			topLeft[0][2] = lightmap[offset2 + offset].b * inv255f;
			topRight[0][0] = lightmap[offset2 + k + 1].r * inv255f;
			topRight[0][1] = lightmap[offset2 + k + 1].g * inv255f;
			topRight[0][2] = lightmap[offset2 + k + 1].b * inv255f;
			bottomLeft[0][0] = lightmap[offset1 + offset].r * inv255f;
			bottomLeft[0][1] = lightmap[offset1 + offset].g * inv255f;
			bottomLeft[0][2] = lightmap[offset1 + offset].b * inv255f;
			bottomRight[0][0] = lightmap[offset1 + k + 1].r * inv255f;
			bottomRight[0][1] = lightmap[offset1 + k + 1].g * inv255f;
			bottomRight[0][2] = lightmap[offset1 + k + 1].b * inv255f;

			//Get the colors of first triangle
			float v[3][2], p[2];
			v[0][0] = SDL_static_cast(float, drawX);
			v[0][1] = SDL_static_cast(float, drawY);
			v[1][0] = SDL_static_cast(float, drawX + scale);
			v[1][1] = SDL_static_cast(float, drawY);
			v[2][0] = SDL_static_cast(float, drawX);
			v[2][1] = SDL_static_cast(float, drawY + scale);
			p[0] = SDL_static_cast(float, drawX + scale);
			p[1] = SDL_static_cast(float, drawY);
			getTrianglePointFloat(v[0], v[1], v[2], topLeft[0], topRight[0], bottomLeft[0], p, topRight[1]);
			p[0] = SDL_static_cast(float, drawX + halfScale);
			p[1] = SDL_static_cast(float, drawY);
			getTrianglePointFloat(v[0], v[1], v[2], topLeft[0], topRight[0], bottomLeft[0], p, topCenter[0]);
			p[0] = SDL_static_cast(float, drawX);
			p[1] = SDL_static_cast(float, drawY + halfScale);
			getTrianglePointFloat(v[0], v[1], v[2], topLeft[0], topRight[0], bottomLeft[0], p, leftCenter[0]);
			p[0] = SDL_static_cast(float, drawX + halfScale) - 0.5f;
			p[1] = SDL_static_cast(float, drawY + halfScale) - 0.5f;
			getTrianglePointFloat(v[0], v[1], v[2], topLeft[0], topRight[0], bottomLeft[0], p, center[0]);
			v[0][0] = SDL_static_cast(float, drawX + scale);
			v[0][1] = SDL_static_cast(float, drawY + scale);
			p[0] = SDL_static_cast(float, drawX);
			p[1] = SDL_static_cast(float, drawY + scale);
			getTrianglePointFloat(v[0], v[1], v[2], bottomRight[0], topRight[0], bottomLeft[0], p, bottomLeft[1]);
			p[0] = SDL_static_cast(float, drawX + scale);
			p[1] = SDL_static_cast(float, drawY + halfScale);
			getTrianglePointFloat(v[0], v[1], v[2], bottomRight[0], topRight[0], bottomLeft[0], p, rightCenter[0]);
			p[0] = SDL_static_cast(float, drawX + halfScale);
			p[1] = SDL_static_cast(float, drawY + scale);
			getTrianglePointFloat(v[0], v[1], v[2], bottomRight[0], topRight[0], bottomLeft[0], p, bottomCenter[0]);

			//Get the colors of second triangle
			v[0][0] = SDL_static_cast(float, drawX);
			v[0][1] = SDL_static_cast(float, drawY + scale);
			v[1][0] = SDL_static_cast(float, drawX);
			v[1][1] = SDL_static_cast(float, drawY);
			v[2][0] = SDL_static_cast(float, drawX + scale);
			v[2][1] = SDL_static_cast(float, drawY + scale);
			p[0] = SDL_static_cast(float, drawX);
			p[1] = SDL_static_cast(float, drawY);
			getTrianglePointFloat(v[0], v[1], v[2], bottomLeft[0], topLeft[0], bottomRight[0], p, topLeft[1]);
			p[0] = SDL_static_cast(float, drawX);
			p[1] = SDL_static_cast(float, drawY + halfScale);
			getTrianglePointFloat(v[0], v[1], v[2], bottomLeft[0], topLeft[0], bottomRight[0], p, leftCenter[1]);
			p[0] = SDL_static_cast(float, drawX + halfScale);
			p[1] = SDL_static_cast(float, drawY + scale);
			getTrianglePointFloat(v[0], v[1], v[2], bottomLeft[0], topLeft[0], bottomRight[0], p, bottomCenter[1]);
			v[0][0] = SDL_static_cast(float, drawX + scale);
			v[0][1] = SDL_static_cast(float, drawY);
			p[0] = SDL_static_cast(float, drawX + scale);
			p[1] = SDL_static_cast(float, drawY + scale);
			getTrianglePointFloat(v[0], v[1], v[2], topRight[0], topLeft[0], bottomRight[0], p, bottomRight[1]);
			p[0] = SDL_static_cast(float, drawX + scale);
			p[1] = SDL_static_cast(float, drawY + halfScale);
			getTrianglePointFloat(v[0], v[1], v[2], topRight[0], topLeft[0], bottomRight[0], p, rightCenter[1]);
			p[0] = SDL_static_cast(float, drawX + halfScale);
			p[1] = SDL_static_cast(float, drawY);
			getTrianglePointFloat(v[0], v[1], v[2], topRight[0], topLeft[0], bottomRight[0], p, topCenter[1]);
			p[0] = SDL_static_cast(float, drawX + halfScale) + 0.5f;
			p[1] = SDL_static_cast(float, drawY + halfScale) + 0.5f;
			getTrianglePointFloat(v[0], v[1], v[2], topRight[0], topLeft[0], bottomRight[0], p, center[1]);

			//Use brighter color from triangles for our square
			#define VEC_MAX(a)										\
				do {												\
					a[0][0] = UTIL_max<float>(a[0][0], a[1][0]);	\
					a[0][1] = UTIL_max<float>(a[0][1], a[1][1]);	\
					a[0][2] = UTIL_max<float>(a[0][2], a[1][2]);	\
				} while(0)

			VEC_MAX(topLeft);
			VEC_MAX(topRight);
			VEC_MAX(bottomLeft);
			VEC_MAX(bottomRight);
			VEC_MAX(leftCenter);
			VEC_MAX(bottomCenter);
			VEC_MAX(rightCenter);
			VEC_MAX(topCenter);
			VEC_MAX(center);
			#undef VEC_MAX

			//Draw Top-Left square
			vertices[0][0] = drawX; vertices[0][1] = (drawY + halfScale);
			colors[0][0] = leftCenter[0][0]; colors[0][1] = leftCenter[0][1]; colors[0][2] = leftCenter[0][2];
			vertices[1][0] = drawX; vertices[1][1] = drawY;
			colors[1][0] = topLeft[0][0]; colors[1][1] = topLeft[0][1]; colors[1][2] = topLeft[0][2];
			vertices[2][0] = (drawX + halfScale); vertices[2][1] = (drawY + halfScale);
			colors[2][0] = center[0][0]; colors[2][1] = center[0][1]; colors[2][2] = center[0][2];
			SDL_DrawTriangle_MOD(src, colors, vertices);
			vertices[0][0] = (drawX + halfScale); vertices[0][1] = drawY;
			colors[0][0] = topCenter[0][0]; colors[0][1] = topCenter[0][1]; colors[0][2] = topCenter[0][2];
			SDL_DrawTriangle_MOD(src, colors, vertices);

			//Draw Bottom-Left square
			vertices[0][0] = drawX; vertices[0][1] = (drawY + halfScale);
			colors[0][0] = leftCenter[0][0]; colors[0][1] = leftCenter[0][1]; colors[0][2] = leftCenter[0][2];
			vertices[1][0] = drawX; vertices[1][1] = (drawY + scale);
			colors[1][0] = bottomLeft[0][0]; colors[1][1] = bottomLeft[0][1]; colors[1][2] = bottomLeft[0][2];
			vertices[2][0] = (drawX + halfScale); vertices[2][1] = (drawY + halfScale);
			colors[2][0] = center[0][0]; colors[2][1] = center[0][1]; colors[2][2] = center[0][2];
			SDL_DrawTriangle_MOD(src, colors, vertices);
			vertices[0][0] = (drawX + halfScale); vertices[0][1] = (drawY + scale);
			colors[0][0] = bottomCenter[0][0]; colors[0][1] = bottomCenter[0][1]; colors[0][2] = bottomCenter[0][2];
			SDL_DrawTriangle_MOD(src, colors, vertices);

			//Draw Top-Right square
			vertices[0][0] = (drawX + halfScale); vertices[0][1] = drawY;
			colors[0][0] = topCenter[0][0]; colors[0][1] = topCenter[0][1]; colors[0][2] = topCenter[0][2];
			vertices[1][0] = (drawX + halfScale); vertices[1][1] = (drawY + halfScale);
			colors[1][0] = center[0][0]; colors[1][1] = center[0][1]; colors[1][2] = center[0][2];
			vertices[2][0] = (drawX + scale); vertices[2][1] = drawY;
			colors[2][0] = topRight[0][0]; colors[2][1] = topRight[0][1]; colors[2][2] = topRight[0][2];
			SDL_DrawTriangle_MOD(src, colors, vertices);
			vertices[0][0] = (drawX + scale); vertices[0][1] = (drawY + halfScale);
			colors[0][0] = rightCenter[0][0]; colors[0][1] = rightCenter[0][1]; colors[0][2] = rightCenter[0][2];
			SDL_DrawTriangle_MOD(src, colors, vertices);

			//Draw Bottom-Right square
			vertices[0][0] = (drawX + halfScale); vertices[0][1] = (drawY + scale);
			colors[0][0] = bottomCenter[0][0]; colors[0][1] = bottomCenter[0][1]; colors[0][2] = bottomCenter[0][2];
			vertices[1][0] = (drawX + halfScale); vertices[1][1] = (drawY + halfScale);
			colors[1][0] = center[0][0]; colors[1][1] = center[0][1]; colors[1][2] = center[0][2];
			vertices[2][0] = (drawX + scale); vertices[2][1] = (drawY + scale);
			colors[2][0] = bottomRight[0][0]; colors[2][1] = bottomRight[0][1]; colors[2][2] = bottomRight[0][2];
			SDL_DrawTriangle_MOD(src, colors, vertices);
			vertices[0][0] = (drawX + scale); vertices[0][1] = (drawY + halfScale);
			colors[0][0] = rightCenter[0][0]; colors[0][1] = rightCenter[0][1]; colors[0][2] = rightCenter[0][2];
			SDL_DrawTriangle_MOD(src, colors, vertices);

			drawX += scale;
		}
		drawY += scale;
	}
}

static int SOFTWARE_drawthread(void* data)
{
	SOFTWARE_threadData* thread = SDL_reinterpret_cast(SOFTWARE_threadData*, data);
	SDL_SetThreadPriority(SDL_THREAD_PRIORITY_HIGH);
	while(true)
	{
		SDL_LockMutex(thread->drawMutex);
		if(!thread->drawning) SDL_CondWait(thread->drawCond, thread->drawMutex);
		if(thread->exiting)
		{
			SDL_UnlockMutex(thread->drawMutex);
			return 0;
		}

		if(thread->lightning == 2)
			SDL_DrawLightMap_new(thread->src, SDL_reinterpret_cast(LightMap*, thread->dst), thread->sr.x, thread->sr.y, thread->sr.w, thread->dr.x, thread->dr.w, thread->dr.h);
		else if(thread->lightning == 1)
			SDL_DrawLightMap_old(thread->src, SDL_reinterpret_cast(LightMap*, thread->dst), thread->sr.x, thread->sr.y, thread->sr.w, thread->dr.x, thread->dr.w, thread->dr.h);
		else
			SDL_SmoothStretch(thread->src, &thread->sr, thread->dst, &thread->dr);

		thread->drawning = false;
		SDL_UnlockMutex(thread->drawMutex);
		if(thread->waiting) SDL_CondSignal(thread->waitCond);
	}
	return 0;
}

void SDL_DrawLightMap_old_MT(SDL_Surface* src, LightMap* lightmap, Sint32 x, Sint32 y, Sint32 scale, Sint32 width, Sint32 height)
{
	if(threadCores == 1)
		return SDL_DrawLightMap_old(src, lightmap, x, y, scale, width, 0, height - 1);

	#define QUEUE_WORK(tr, start, end)									\
		do {															\
			thread = &threadDatas[tr];									\
			thread->src = src;											\
			thread->dst = SDL_reinterpret_cast(SDL_Surface*, lightmap);	\
			thread->lightning = 1;										\
			thread->drawning = true;									\
			thread->sr = {x, y + (start * scale), scale, scale};		\
			thread->dr = {width, height, start, end};					\
			SDL_CondSignal(thread->drawCond);							\
		} while(0)

	SOFTWARE_threadData* thread;
	if(threadCores == 4)
	{
		Sint32 trHeight = (height + 3) / 4;
		Sint32 firstStart = -1;
		Sint32 firstEnd = firstStart + trHeight;
		Sint32 secondStart = firstStart + trHeight;
		Sint32 secondEnd = secondStart + trHeight;
		Sint32 thirdStart = secondStart + trHeight;
		Sint32 thirdEnd = thirdStart + trHeight;
		Sint32 fourthStart = thirdStart + trHeight;
		Sint32 fourthEnd = height - 1;
		QUEUE_WORK(0, firstStart, firstEnd);
		QUEUE_WORK(1, secondStart, secondEnd);
		QUEUE_WORK(2, thirdStart, thirdEnd);
		QUEUE_WORK(3, fourthStart, fourthEnd);
	}
	else
	{
		Sint32 trHeight = (height + 1) / 2;
		Sint32 firstStart = -1;
		Sint32 firstEnd = firstStart + trHeight;
		Sint32 secondStart = firstStart + trHeight;
		Sint32 secondEnd = height - 1;
		QUEUE_WORK(0, firstStart, firstEnd);
		QUEUE_WORK(1, secondStart, secondEnd);
	}

	for(Sint32 i = 0; i < threadCores; ++i)
	{
		thread = &threadDatas[i];
		SDL_LockMutex(thread->drawMutex);
		if(thread->drawning)
		{
			thread->waiting = true;
			SDL_CondWait(thread->waitCond, thread->drawMutex);
			thread->waiting = false;
		}
		SDL_UnlockMutex(thread->drawMutex);
	}
	#undef QUEUE_WORK
}

void SDL_DrawLightMap_new_MT(SDL_Surface* src, LightMap* lightmap, Sint32 x, Sint32 y, Sint32 scale, Sint32 width, Sint32 height)
{
	if(threadCores == 1)
		return SDL_DrawLightMap_new(src, lightmap, x, y, scale, width, 0, height - 1);

	#define QUEUE_WORK(tr, start, end)									\
		do {															\
			thread = &threadDatas[tr];									\
			thread->src = src;											\
			thread->dst = SDL_reinterpret_cast(SDL_Surface*, lightmap);	\
			thread->lightning = 2;										\
			thread->drawning = true;									\
			thread->sr = {x, y + (start * scale), scale, scale};		\
			thread->dr = {width, height, start, end};					\
			SDL_CondSignal(thread->drawCond);							\
		} while(0)

	SOFTWARE_threadData* thread;
	if(threadCores == 4)
	{
		Sint32 trHeight = (height + 3) / 4;
		Sint32 firstStart = -1;
		Sint32 firstEnd = firstStart + trHeight;
		Sint32 secondStart = firstStart + trHeight;
		Sint32 secondEnd = secondStart + trHeight;
		Sint32 thirdStart = secondStart + trHeight;
		Sint32 thirdEnd = thirdStart + trHeight;
		Sint32 fourthStart = thirdStart + trHeight;
		Sint32 fourthEnd = height - 1;
		QUEUE_WORK(0, firstStart, firstEnd);
		QUEUE_WORK(1, secondStart, secondEnd);
		QUEUE_WORK(2, thirdStart, thirdEnd);
		QUEUE_WORK(3, fourthStart, fourthEnd);
	}
	else
	{
		Sint32 trHeight = (height + 1) / 2;
		Sint32 firstStart = -1;
		Sint32 firstEnd = firstStart + trHeight;
		Sint32 secondStart = firstStart + trHeight;
		Sint32 secondEnd = height - 1;
		QUEUE_WORK(0, firstStart, firstEnd);
		QUEUE_WORK(1, secondStart, secondEnd);
	}

	for(Sint32 i = 0; i < threadCores; ++i)
	{
		thread = &threadDatas[i];
		SDL_LockMutex(thread->drawMutex);
		if(thread->drawning)
		{
			thread->waiting = true;
			SDL_CondWait(thread->waitCond, thread->drawMutex);
			thread->waiting = false;
		}
		SDL_UnlockMutex(thread->drawMutex);
	}
	#undef QUEUE_WORK
}

Sint32 SDL_SmoothStretch_MT(SDL_Surface* src, SDL_Rect* srcrect, SDL_Surface* dst, SDL_Rect* dstrect)
{
	if(threadCores == 1)
		return SDL_SmoothStretch(src, srcrect, dst, dstrect);

	if(!srcrect)
	{
		SDL_Rect srcr = {0,0,src->w,src->h};
		srcrect = &srcr;
	}
	else
	{
		if(srcrect->x < 0 || srcrect->y < 0 || srcrect->x + srcrect->w > src->w || srcrect->y + srcrect->h > src->h)
			return -1;
	}

	if(!dstrect)
	{
		SDL_Rect dstr = {0,0,dst->w,dst->h};
		dstrect = &dstr;
	}
	else
	{
		if(dstrect->x < 0 || dstrect->y < 0 || dstrect->x + dstrect->w > dst->w || dstrect->y + dstrect->h > dst->h)
			return -1;
	}

	#define QUEUE_WORK(tr, sx, sy, sw, sh, dx, dy, dw, dh)	\
		do {												\
			thread = &threadDatas[tr];						\
			thread->src = src;								\
			thread->dst = dst;								\
			thread->lightning = 0;							\
			thread->drawning = true;						\
			thread->sr = {sx, sy, sw, sh};				    \
			thread->dr = {dx, dy, dw, dh};				    \
			SDL_CondSignal(thread->drawCond);				\
		} while(0)

	SOFTWARE_threadData* thread;
	if(threadCores == 4)
	{
		Sint32 halfSrcWidth = srcrect->w / 2;
		Sint32 halfSrcHeight = srcrect->h / 2;
		Sint32 halfDstWidth = dstrect->w / 2;
		Sint32 halfDstHeight = dstrect->h / 2;
		QUEUE_WORK(0, srcrect->x, srcrect->y, halfSrcWidth + 1, halfSrcHeight + 1, dstrect->x, dstrect->y, halfDstWidth, halfDstHeight);
		QUEUE_WORK(1, srcrect->x + halfSrcWidth, srcrect->y, srcrect->w - halfSrcWidth, halfSrcHeight + 1, dstrect->x + halfDstWidth, dstrect->y, dstrect->w - halfDstWidth, halfDstHeight);
		QUEUE_WORK(2, srcrect->x, srcrect->y + halfSrcHeight, halfSrcWidth + 1, srcrect->h - halfSrcHeight, dstrect->x, dstrect->y + halfDstHeight, halfDstWidth, dstrect->h - halfDstHeight);
		QUEUE_WORK(3, threadDatas[1].sr.x, threadDatas[2].sr.y, threadDatas[1].sr.w, threadDatas[2].sr.h, threadDatas[1].dr.x, threadDatas[2].dr.y, threadDatas[1].dr.w, threadDatas[2].dr.h);
	}
	else
	{
		Sint32 halfSrcHeight = srcrect->h / 2;
		Sint32 halfDstHeight = dstrect->h / 2;
		QUEUE_WORK(0, srcrect->x, srcrect->y, srcrect->w, halfSrcHeight + 1, dstrect->x, dstrect->y, dstrect->w, halfDstHeight);
		QUEUE_WORK(1, srcrect->x, srcrect->y + halfSrcHeight, srcrect->w, srcrect->h - halfSrcHeight, dstrect->x, dstrect->y + halfDstHeight, dstrect->w, dstrect->h - halfDstHeight);
	}

	for(Sint32 i = 0; i < threadCores; ++i)
	{
		thread = &threadDatas[i];
		SDL_LockMutex(thread->drawMutex);
		if(thread->drawning)
		{
			thread->waiting = true;
			SDL_CondWait(thread->waitCond, thread->drawMutex);
			thread->waiting = false;
		}
		SDL_UnlockMutex(thread->drawMutex);
	}
	#undef QUEUE_WORK
	return 0;
}

void SDL_SmoothStretch_init()
{
	Sint32 cores = SDL_GetCPUCount();
	if(cores >= 4)
		threadCores = 4;
	else if(cores >= 2)
		threadCores = 2;
	else
		threadCores = 1;

	SOFTWARE_threadData* thread;
	for(Sint32 i = 0; i < threadCores; ++i)
	{
		thread = &threadDatas[i];
		thread->waiting = false;
		thread->drawning = false;
		thread->exiting = false;
		thread->drawMutex = SDL_CreateMutex();
		thread->drawCond = SDL_CreateCond();
		thread->waitCond = SDL_CreateCond();
		thread->thread = SDL_CreateThread(SOFTWARE_drawthread, "SOFTWARE RASTERIZER", thread);
	}

	SDL_DrawTriangle_MOD = SDL_reinterpret_cast(LPSDL_DrawTriangle_MOD, DrawTriangle_Scalar);
	#ifdef __USE_SSE__
	//need to test if fma4 is faster on amd processors to determine proper check order
	#if (defined(__USE_FMA3__) || defined(__USE_FMA4__))
	if(SDL_HasFMA3())
		SDL_DrawTriangle_MOD = SDL_reinterpret_cast(LPSDL_DrawTriangle_MOD, DrawTriangle_FMA3);
	// else if(SDL_HasFMA4())
	// 	SDL_DrawTriangle_MOD = SDL_reinterpret_cast(LPSDL_DrawTriangle_MOD, DrawTriangle_FMA4);
	else if(SDL_HasSSE())
		SDL_DrawTriangle_MOD = SDL_reinterpret_cast(LPSDL_DrawTriangle_MOD, DrawTriangle_SSE);
	#elif defined(__USE_FMA3__)
	if(SDL_HasFMA3())
		SDL_DrawTriangle_MOD = SDL_reinterpret_cast(LPSDL_DrawTriangle_MOD, DrawTriangle_FMA3);
	else if(SDL_HasSSE())
		SDL_DrawTriangle_MOD = SDL_reinterpret_cast(LPSDL_DrawTriangle_MOD, DrawTriangle_SSE);
	#elif defined(__USE_FMA4__)
	if(SDL_HasFMA4())
		SDL_DrawTriangle_MOD = SDL_reinterpret_cast(LPSDL_DrawTriangle_MOD, DrawTriangle_FMA4);
	else if(SDL_HasSSE())
		SDL_DrawTriangle_MOD = SDL_reinterpret_cast(LPSDL_DrawTriangle_MOD, DrawTriangle_SSE);
	#else
	if(SDL_HasSSE())
		SDL_DrawTriangle_MOD = SDL_reinterpret_cast(LPSDL_DrawTriangle_MOD, DrawTriangle_SSE);
	#endif
	#endif

	#ifdef __USE_SSE4_1__
	if(SDL_HasSSE41())
	{
		//_mm_mullo_epi32 - to make calculations a bit faster
		SDL_SmoothStretch = SDL_reinterpret_cast(LPSDL_SmoothStretch, SDL_SmoothStretch_SSE41);
	}
	else
	#endif
	#ifdef __USE_SSSE3__
	if(SDL_HasSSSE3())
	{
		//_mm_shuffle_epi8 - to avoid unnecesary packing/unpacking
		SDL_SmoothStretch = SDL_reinterpret_cast(LPSDL_SmoothStretch, SDL_SmoothStretch_SSSE3);
	}
	else
	#endif
	#ifdef __USE_SSE2__
	if(SDL_HasSSE2())
	{
		//Use sse2 to vectorize most calculations
		SDL_SmoothStretch = SDL_reinterpret_cast(LPSDL_SmoothStretch, SDL_SmoothStretch_SSE2);
	}
	else
	#endif
	{
		//Standard scalar function - the slowest approach
		SDL_SmoothStretch = SDL_reinterpret_cast(LPSDL_SmoothStretch, SDL_SmoothStretch_scalar);
	}
}

void SDL_SmoothStretch_shutdown()
{
	SOFTWARE_threadData* thread;
	for(Sint32 i = 0; i < threadCores; ++i)
	{
		thread = &threadDatas[i];
		thread->exiting = true;
		SDL_CondSignal(thread->drawCond);
		SDL_WaitThread(thread->thread, NULL);
		SDL_DestroyCond(thread->waitCond);
		SDL_DestroyCond(thread->drawCond);
		SDL_DestroyMutex(thread->drawMutex);
	}
}
