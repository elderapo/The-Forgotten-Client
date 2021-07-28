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

#ifndef __FILE_SURFACE_OPENGLES_h_
#define __FILE_SURFACE_OPENGLES_h_

#include "engine.h"

#if defined(SDL_VIDEO_RENDER_OGL_ES)

#if defined(__linux__)
#include <GL/gl.h>
#endif

#define OPENGLES_MAX_VERTICES 2048
#define OPENGLES_MAX_INDICES (OPENGLES_MAX_VERTICES * 6 / 4)
#if OPENGLES_MAX_INDICES >= 65500
#define OPENGLES_USE_UINT_INDICES 1
#else
#define OPENGLES_USE_UINT_INDICES 0
#endif

typedef void (APIENTRY *PFN_OglBindTexture)(unsigned int target, unsigned int texture);
typedef void (APIENTRY *PFN_OglGenTextures)(int n, unsigned int* textures);
typedef void (APIENTRY *PFN_OglDeleteTextures)(int n, unsigned int* textures);
typedef void (APIENTRY *PFN_OglTexImage2D)(unsigned int target, int level, int iformat, unsigned int width, unsigned int height, int border, unsigned int format, unsigned int type, const void* pixels);
typedef void (APIENTRY *PFN_OglTexSubImage2D)(unsigned int target, int level, int xoffset, int yoffset, unsigned int width, unsigned int height, unsigned int format, unsigned int type, const void* pixels);
typedef void (APIENTRY *PFN_OglTexParameteri)(unsigned int target, unsigned int pname, int param);
typedef void (APIENTRY *PFN_OglTexEnvi)(unsigned int target, unsigned int pname, int param);
typedef void (APIENTRY *PFN_OglPixelStorei)(unsigned int pname, int param);
typedef void (APIENTRY *PFN_OglCopyTexSubImage2D)(unsigned int target, int level, int xoffset, int yoffset, int x, int y, int width, int height);
typedef void (APIENTRY *PFN_OglEnable)(unsigned int cap);
typedef void (APIENTRY *PFN_OglDisable)(unsigned int cap);
typedef void (APIENTRY *PFN_OglEnableClientState)(unsigned int cap);
typedef void (APIENTRY *PFN_OglDisableClientState)(unsigned int cap);
typedef void (APIENTRY *PFN_OglDrawArrays)(unsigned int mode, int first, int count);
typedef void (APIENTRY *PFN_OglDrawElements)(unsigned int mode, int count, unsigned int type, const void* indices);
typedef void (APIENTRY *PFN_OglBlendFunc)(unsigned int sfactor, unsigned int dfactor);
typedef void (APIENTRY *PFN_OglBlendFuncSeparate)(unsigned int sfactorRGB, unsigned int dfactorRGB, unsigned int sfactorAlpha, unsigned int dfactorAlpha);
typedef void (APIENTRY *PFN_OglBlendEquation)(unsigned int mode);
typedef void (APIENTRY *PFN_OglColorMask)(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha);
typedef void (APIENTRY *PFN_OglColor4f)(float red, float green, float blue, float alpha);
typedef void (APIENTRY *PFN_OglTexCoordPointer)(int size, unsigned int type, int stride, void* pointer);
typedef void (APIENTRY *PFN_OglVertexPointer)(int size, unsigned int type, int stride, void* pointer);
typedef void (APIENTRY *PFN_OglColorPointer)(int size, unsigned int type, int stride, void* pointer);
typedef void (APIENTRY *PFN_OglReadPixels)(int x, int y, int width, int height, unsigned int format, unsigned int type, void* pixels);
typedef void (APIENTRY *PFN_OglGetIntegerv)(unsigned int pname, int* params);
typedef void (APIENTRY *PFN_OglLoadIdentity)();
typedef void (APIENTRY *PFN_OglMatrixMode)(unsigned int mode);
typedef void (APIENTRY *PFN_OglOrthof)(float left, float right, float bottom, float top, float zNear, float zFar);
typedef void (APIENTRY *PFN_OglViewport)(int x, int y, int width, int height);
typedef void (APIENTRY *PFN_OglScissor)(int x, int y, int width, int height);
typedef unsigned char* (APIENTRY *PFN_OglGetString)(unsigned int name);
typedef unsigned int (APIENTRY *PFN_OglGetError)();
typedef void (APIENTRY *PFN_OglFinish)();

typedef void (APIENTRY *PFN_OglGenFramebuffers)(int n, unsigned int* framebuffers);
typedef void (APIENTRY *PFN_OglDeleteFramebuffers)(int n, const unsigned int* framebuffers);
typedef void (APIENTRY *PFN_OglFramebufferTexture2D)(unsigned int target, unsigned int attachment, unsigned int textarget, unsigned int texture, int level);
typedef void (APIENTRY *PFN_OglBindFramebuffer)(unsigned int target, unsigned int framebuffer);
typedef unsigned int (APIENTRY *PFN_OglCheckFramebufferStatus)(unsigned int target);

struct VertexOpenglES
{
	VertexOpenglES(float x, float y, float u, float v, DWORD color) : x(x), y(y), color(color), u(u), v(v) {}
	VertexOpenglES(float x, float y, DWORD color) : x(x), y(y), color(color), u(0.0f), v(0.0f) {}

	float x, y;
	float u, v;
	DWORD color;
};

struct OpenglESTexture
{
	OpenglESTexture() : m_texture(0), m_framebuffer(0) {}

	operator bool() const {return (m_texture != 0);}

	// non-copyable
	OpenglESTexture(const OpenglESTexture&) = delete;
	OpenglESTexture& operator=(const OpenglESTexture&) = delete;

	// moveable
	OpenglESTexture(OpenglESTexture&& rhs) noexcept : m_texture(rhs.m_texture), m_framebuffer(rhs.m_framebuffer),
		m_width(rhs.m_width), m_height(rhs.m_height), m_scaleW(rhs.m_scaleW), m_scaleH(rhs.m_scaleH)
	{
		rhs.m_texture = 0;
		rhs.m_framebuffer = 0;
	}
	OpenglESTexture& operator=(OpenglESTexture&& rhs) noexcept
	{
		if(this != &rhs)
		{
			m_texture = rhs.m_texture;
			m_framebuffer = rhs.m_framebuffer;
			m_width = rhs.m_width;
			m_height = rhs.m_height;
			m_scaleW = rhs.m_scaleW;
			m_scaleH = rhs.m_scaleH;
			rhs.m_texture = 0;
			rhs.m_framebuffer = 0;
		}
		return (*this);
	}

	unsigned int m_texture;
	unsigned int m_framebuffer;
	Uint32 m_width;
	Uint32 m_height;
	float m_scaleW;
	float m_scaleH;
};

struct OpenglESSpriteData
{
	OpenglESSpriteData() : m_xOffset(0), m_yOffset(0), m_surface(0), m_lastUsage(0) {}

	Uint32 m_xOffset;
	Uint32 m_yOffset;
	Uint32 m_surface;
	Uint32 m_lastUsage;
};

typedef robin_hood::unordered_map<Uint32, OpenglESTexture> U32BGLESTextures;
typedef robin_hood::unordered_map<Uint64, OpenglESSpriteData> U64BGLESTextures;

class SurfaceOpenglES : public Surface
{
	public:
		SurfaceOpenglES();
		virtual ~SurfaceOpenglES();

		// non-copyable
		SurfaceOpenglES(const SurfaceOpenglES&) = delete;
		SurfaceOpenglES& operator=(const SurfaceOpenglES&) = delete;

		// non-moveable
		SurfaceOpenglES(const SurfaceOpenglES&&) = delete;
		SurfaceOpenglES& operator=(const SurfaceOpenglES&&) = delete;

		bool createOpenGLESTexture(OpenglESTexture& texture, Uint32 width, Uint32 height, bool linearSampler, bool frameBuffer = false);
		bool updateTextureData(OpenglESTexture& texture, unsigned char* data);
		void releaseOpenGLESTexture(OpenglESTexture& texture);

		void changeBlending(unsigned int sfactorRGB, unsigned int dfactorRGB, unsigned int sfactorAlpha, unsigned int dfactorAlpha);
		void updateViewport();
		void updateRenderer();

		virtual bool isSupported();
		virtual const char* getName() {return "OpenGL ES";}
		virtual const char* getSoftware() {return m_software;}
		virtual const char* getHardware() {return m_hardware;}
		virtual Uint32 getVRAM() {return m_totalVRAM;}

		void generateSpriteAtlases();

		virtual void init();
		virtual void doResize(Sint32 w, Sint32 h);
		virtual void spriteManagerReset();
		virtual unsigned char* getScreenPixels(Sint32& width, Sint32& height, bool& bgra);

		virtual void beginScene();
		virtual void endScene();

		void copyFromScreen(Sint32 x, Sint32 y, Uint32 w, Uint32 h);
		void drawBackup(Sint32 x, Sint32 y, Sint32 w, Sint32 h);

		OpenglESTexture* getTextureIndex(OpenglESTexture* texture);
		void drawQuad(OpenglESTexture* texture, float vertices[8], float texcoords[8]);
		void drawQuad(OpenglESTexture* texture, float vertices[8], float texcoords[8], DWORD color);
		void scheduleBatch();

		bool integer_scaling(Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h);
		virtual void drawLightMap_old(LightMap* lightmap, Sint32 x, Sint32 y, Sint32 scale, Sint32 width, Sint32 height);
		virtual void drawLightMap_new(LightMap* lightmap, Sint32 x, Sint32 y, Sint32 scale, Sint32 width, Sint32 height);
		virtual void drawGameScene(Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h);
		virtual void beginGameScene();
		virtual void endGameScene();

		virtual void setClipRect(Sint32 x, Sint32 y, Sint32 w, Sint32 h);
		virtual void disableClipRect();
		virtual void drawRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 lineWidth, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
		virtual void fillRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

		OpenglESTexture* loadPicture(Uint16 pictureId, bool linear);
		virtual void drawFont(Uint16 pictureId, Sint32 x, Sint32 y, const std::string& text, size_t pos, size_t len, Uint8 r, Uint8 g, Uint8 b, Sint16 cX[256], Sint16 cY[256], Sint16 cW[256], Sint16 cH[256]);
		virtual void drawBackground(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h);
		virtual void drawPictureRepeat(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h);
		virtual void drawPicture(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 x, Sint32 y, Sint32 w, Sint32 h);

		bool loadSprite(Uint32 spriteId, OpenglESTexture* texture, Uint32 xoff, Uint32 yoff);
		bool loadSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Uint32 outfitColor, OpenglESTexture* texture, Uint32 xoff, Uint32 yoff);
		virtual void drawSprite(Uint32 spriteId, Sint32 x, Sint32 y);
		virtual void drawSprite(Uint32 spriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh);
		virtual void drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Uint32 outfitColor);
		virtual void drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Uint32 outfitColor);

		OpenglESTexture* createAutomapTile(Uint32 currentArea);
		void uploadAutomapTile(OpenglESTexture* texture, Uint8 color[256][256]);
		virtual void drawAutomapTile(Uint32 m_currentArea, bool& m_recreate, Uint8 m_color[256][256], Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh);

	protected:
		std::vector<VertexOpenglES> m_vertices;
		#if OPENGLES_USE_UINT_INDICES > 0
		std::vector<Uint32> m_indices;
		#else
		std::vector<Uint16> m_indices;
		#endif
		std::vector<OpenglESTexture> m_spritesAtlas;
		U32BGLESTextures m_automapTiles;
		U64BGLESTextures m_sprites;
		std::circular_buffer<Uint32, MAX_AUTOMAPTILES> m_automapTilesBuff;
		std::circular_buffer<Uint64, MAX_SPRITES> m_spritesIds;

		OpenglESTexture* m_pictures = NULL;
		char* m_software = NULL;
		char* m_hardware = NULL;

		SDL_GLContext m_oglContext = NULL;
		OpenglESTexture* m_renderTarget = NULL;

		OpenglESTexture m_gameWindow;
		OpenglESTexture m_scaled_gameWindow;
		OpenglESTexture m_backup;
		OpenglESTexture* m_binded_texture = NULL;

		PFN_OglBindTexture OglBindTexture;
		PFN_OglGenTextures OglGenTextures;
		PFN_OglDeleteTextures OglDeleteTextures;
		PFN_OglTexImage2D OglTexImage2D;
		PFN_OglTexSubImage2D OglTexSubImage2D;
		PFN_OglTexParameteri OglTexParameteri;
		PFN_OglTexEnvi OglTexEnvi;
		PFN_OglPixelStorei OglPixelStorei;
		PFN_OglCopyTexSubImage2D OglCopyTexSubImage2D;
		PFN_OglEnable OglEnable;
		PFN_OglDisable OglDisable;
		PFN_OglEnableClientState OglEnableClientState;
		PFN_OglDisableClientState OglDisableClientState;
		PFN_OglDrawArrays OglDrawArrays;
		PFN_OglDrawElements OglDrawElements;
		PFN_OglBlendFunc OglBlendFunc;
		PFN_OglBlendFuncSeparate OglBlendFuncSeparate;
		PFN_OglBlendEquation OglBlendEquation;
		PFN_OglColorMask OglColorMask;
		PFN_OglColor4f OglColor4f;
		PFN_OglTexCoordPointer OglTexCoordPointer;
		PFN_OglVertexPointer OglVertexPointer;
		PFN_OglColorPointer OglColorPointer;
		PFN_OglReadPixels OglReadPixels;
		PFN_OglGetIntegerv OglGetIntegerv;
		PFN_OglLoadIdentity OglLoadIdentity;
		PFN_OglMatrixMode OglMatrixMode;
		PFN_OglOrthof OglOrthof;
		PFN_OglViewport OglViewport;
		PFN_OglScissor OglScissor;
		PFN_OglGetString OglGetString;
		PFN_OglGetError OglGetError;
		PFN_OglFinish OglFinish;

		PFN_OglGenFramebuffers OglGenFramebuffers;
		PFN_OglDeleteFramebuffers OglDeleteFramebuffers;
		PFN_OglFramebufferTexture2D OglFramebufferTexture2D;
		PFN_OglBindFramebuffer OglBindFramebuffer;
		PFN_OglCheckFramebufferStatus OglCheckFramebufferStatus;

		unsigned int window_framebuffer = 0;

		Sint32 m_maxTextureSize = 1024;
		Sint32 m_integer_scaling_width = 0;
		Sint32 m_integer_scaling_height = 0;

		Uint32 m_totalVRAM = 0;
		Uint32 m_spriteChecker = 0;
		Uint32 m_currentFrame = 0;
		Uint32 m_cachedVertices = 0;

		Uint32 m_spriteAtlases = 0;
		Uint32 m_spritesPerAtlas = 0;
		Uint32 m_spritesPerModulo = 0;

		Sint32 m_viewPortX = 0;
		Sint32 m_viewPortY = 0;
		Sint32 m_viewPortW = 0;
		Sint32 m_viewPortH = 0;

		bool m_useFBO = false;
		bool m_useNonPower2 = false;
		bool m_useAlphaBlending = false;
		bool m_useBlendEquation = false;
};
#endif
#endif /* __FILE_SURFACE_OPENGLES_h_ */
