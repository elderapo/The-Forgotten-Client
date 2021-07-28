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

#ifndef __FILE_SURFACE_OPENGL_h_
#define __FILE_SURFACE_OPENGL_h_

#include "engine.h"
#include <GL/gl.h>

#if defined(SDL_VIDEO_RENDER_OGL)

#define OPENGL_MAX_VERTICES 43656
#define OPENGL_MAX_INDICES (OPENGL_MAX_VERTICES * 6 / 4)
#if OPENGL_MAX_INDICES >= 65500
#define OPENGL_USE_UINT_INDICES 1
#else
#define OPENGL_USE_UINT_INDICES 0
#endif

#ifdef __APPLE__
typedef void* GLhandleARB;
#else
typedef unsigned int GLhandleARB;
#endif

typedef void (APIENTRY *PFN_OglBindTexture)(unsigned int target, unsigned int texture);
typedef void (APIENTRY *PFN_OglGenTextures)(int n, unsigned int* textures);
typedef void (APIENTRY *PFN_OglDeleteTextures)(int n, unsigned int* textures);
typedef void (APIENTRY *PFN_OglTexImage2D)(unsigned int target, int level, int iformat, unsigned int width, unsigned int height, int border, unsigned int format, unsigned int type, const void* pixels);
typedef void (APIENTRY *PFN_OglTexSubImage2D)(unsigned int target, int level, int xoffset, int yoffset, unsigned int width, unsigned int height, unsigned int format, unsigned int type, const void* pixels);
typedef void (APIENTRY *PFN_OglTexParameteri)(unsigned int target, unsigned int pname, int param);
typedef void (APIENTRY *PFN_OglTexEnvi)(unsigned int target, unsigned int pname, int param);
typedef void (APIENTRY *PFN_OglPixelStorei)(unsigned int pname, int param);
typedef void (APIENTRY *PFN_OglDrawBuffer)(unsigned int mode);
typedef void (APIENTRY *PFN_OglReadBuffer)(unsigned int mode);
typedef void (APIENTRY *PFN_OglCopyTexSubImage2D)(unsigned int target, int level, int xoffset, int yoffset, int x, int y, int width, int height);
typedef void (APIENTRY *PFN_OglPolygonMode)(unsigned int face, unsigned int mode);
typedef void (APIENTRY *PFN_OglEnable)(unsigned int cap);
typedef void (APIENTRY *PFN_OglDisable)(unsigned int cap);
typedef void (APIENTRY *PFN_OglEnableClientState)(unsigned int cap);
typedef void (APIENTRY *PFN_OglDisableClientState)(unsigned int cap);
typedef void (APIENTRY *PFN_OglDrawArrays)(unsigned int mode, int first, int count);
typedef void (APIENTRY *PFN_OglDrawElements)(unsigned int mode, int count, unsigned int type, const void* indices);
typedef void (APIENTRY *PFN_OglBlendFunc)(unsigned int sfactorRGB, unsigned int dfactorRGB);
typedef void (APIENTRY *PFN_OglColorMask)(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha);
typedef void (APIENTRY *PFN_OglTexCoordPointer)(int size, unsigned int type, int stride, void* pointer);
typedef void (APIENTRY *PFN_OglVertexPointer)(int size, unsigned int type, int stride, void* pointer);
typedef void (APIENTRY *PFN_OglColorPointer)(int size, unsigned int type, int stride, void* pointer);
typedef void (APIENTRY *PFN_OglReadPixels)(int x, int y, int width, int height, unsigned int format, unsigned int type, void* pixels);
typedef void (APIENTRY *PFN_OglGetIntegerv)(unsigned int pname, int* params);
typedef void (APIENTRY *PFN_OglLoadIdentity)();
typedef void (APIENTRY *PFN_OglMatrixMode)(unsigned int mode);
typedef void (APIENTRY *PFN_OglOrtho)(double left, double right, double bottom, double top, double zNear, double zFar);
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

typedef void (APIENTRY *PFN_OglAttachObjectARB)(GLhandleARB containerObj, GLhandleARB obj);
typedef void (APIENTRY *PFN_OglCompileShaderARB)(GLhandleARB shaderObj);
typedef GLhandleARB (APIENTRY *PFN_OglCreateProgramObjectARB)(void);
typedef GLhandleARB (APIENTRY *PFN_OglCreateShaderObjectARB)(unsigned int shaderType);
typedef void (APIENTRY *PFN_OglDeleteObjectARB)(GLhandleARB obj);
typedef void (APIENTRY *PFN_OglGetObjectParameterivARB)(GLhandleARB obj, unsigned int pname, int* params);
typedef int (APIENTRY *PFN_OglGetUniformLocationARB)(GLhandleARB programObj, const char* name);
typedef void (APIENTRY *PFN_OglLinkProgramARB)(GLhandleARB programObj);
typedef void (APIENTRY *PFN_OglShaderSourceARB)(GLhandleARB shaderObj, int count, const char** string, const int* length);
typedef void (APIENTRY *PFN_OglUniform1iARB)(int location, int v0);
typedef void (APIENTRY *PFN_OglUniform2fARB)(int location, float v0, float v1);
typedef void (APIENTRY *PFN_OglUseProgramObjectARB)(GLhandleARB programObj);

struct VertexOpengl
{
	VertexOpengl(float x, float y, float u, float v, DWORD color) : x(x), y(y), color(color), u(u), v(v) {}
	VertexOpengl(float x, float y, DWORD color) : x(x), y(y), color(color), u(0.0f), v(0.0f) {}

	float x, y;
	float u, v;
	DWORD color;
};

struct OpenglShader
{
	OpenglShader() : program(NULL), vert_shader(NULL), pix_shader(NULL) {}

	GLhandleARB program;
	GLhandleARB vert_shader;
	GLhandleARB pix_shader;
};

struct OpenglTexture
{
	OpenglTexture() : m_texture(0), m_framebuffer(0) {}

	operator bool() const {return (m_texture != 0);}

	// non-copyable
	OpenglTexture(const OpenglTexture&) = delete;
	OpenglTexture& operator=(const OpenglTexture&) = delete;

	// moveable
	OpenglTexture(OpenglTexture&& rhs) noexcept : m_texture(rhs.m_texture), m_framebuffer(rhs.m_framebuffer),
		m_width(rhs.m_width), m_height(rhs.m_height), m_scaleW(rhs.m_scaleW), m_scaleH(rhs.m_scaleH)
	{
		rhs.m_texture = 0;
		rhs.m_framebuffer = 0;
	}
	OpenglTexture& operator=(OpenglTexture&& rhs) noexcept
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

struct OpenglSpriteData
{
	OpenglSpriteData() : m_xOffset(0), m_yOffset(0), m_surface(0), m_lastUsage(0) {}

	Uint32 m_xOffset;
	Uint32 m_yOffset;
	Uint32 m_surface;
	Uint32 m_lastUsage;
};

typedef robin_hood::unordered_map<Uint32, OpenglTexture> U32BGLTextures;
typedef robin_hood::unordered_map<Uint64, OpenglSpriteData> U64BGLTextures;

class SurfaceOpengl : public Surface
{
	public:
		SurfaceOpengl();
		virtual ~SurfaceOpengl();

		// non-copyable
		SurfaceOpengl(const SurfaceOpengl&) = delete;
		SurfaceOpengl& operator=(const SurfaceOpengl&) = delete;

		// non-moveable
		SurfaceOpengl(const SurfaceOpengl&&) = delete;
		SurfaceOpengl& operator=(const SurfaceOpengl&&) = delete;

		bool createOpenGLShader(OpenglShader& program, const char* vertex_shader, const char* pixel_shader);
		void releaseOpenGLShader(OpenglShader& program);

		bool createOpenGLTexture(OpenglTexture& texture, Uint32 width, Uint32 height, bool linearSampler, bool frameBuffer = false);
		bool updateTextureData(OpenglTexture& texture, unsigned char* data);
		void releaseOpenGLTexture(OpenglTexture& texture);

		void updateViewport();
		void updateRenderer();

		virtual bool isSupported();
		virtual const char* getName() {return "OpenGL Legacy";}
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

		OpenglTexture* getTextureIndex(OpenglTexture* texture);
		void drawQuad(OpenglTexture* texture, float vertices[8], float texcoords[8]);
		void drawQuad(OpenglTexture* texture, float vertices[8], float texcoords[8], DWORD color);
		void scheduleBatch();

		bool integer_scaling(Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h);
		virtual void drawLightMap_old(LightMap* lightmap, Sint32 x, Sint32 y, Sint32 scale, Sint32 width, Sint32 height);
		virtual void drawLightMap_new(LightMap* lightmap, Sint32 x, Sint32 y, Sint32 scale, Sint32 width, Sint32 height) ;
		virtual void drawGameScene(Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h);
		virtual void beginGameScene();
		virtual void endGameScene();

		virtual void setClipRect(Sint32 x, Sint32 y, Sint32 w, Sint32 h);
		virtual void disableClipRect();
		virtual void drawRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 lineWidth, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
		virtual void fillRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

		OpenglTexture* loadPicture(Uint16 pictureId, bool linear);
		virtual void drawFont(Uint16 pictureId, Sint32 x, Sint32 y, const std::string& text, size_t pos, size_t len, Uint8 r, Uint8 g, Uint8 b, Sint16 cX[256], Sint16 cY[256], Sint16 cW[256], Sint16 cH[256]);
		virtual void drawBackground(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h);
		virtual void drawPictureRepeat(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h);
		virtual void drawPicture(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 x, Sint32 y, Sint32 w, Sint32 h);

		bool loadSprite(Uint32 spriteId, OpenglTexture* texture, Uint32 xoff, Uint32 yoff);
		bool loadSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Uint32 outfitColor, OpenglTexture* texture, Uint32 xoff, Uint32 yoff);
		virtual void drawSprite(Uint32 spriteId, Sint32 x, Sint32 y);
		virtual void drawSprite(Uint32 spriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh);
		virtual void drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Uint32 outfitColor);
		virtual void drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Uint32 outfitColor);

		OpenglTexture* createAutomapTile(Uint32 currentArea);
		void uploadAutomapTile(OpenglTexture* texture, Uint8 color[256][256]);
		virtual void drawAutomapTile(Uint32 m_currentArea, bool& m_recreate, Uint8 m_color[256][256], Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh);

	protected:
		std::vector<VertexOpengl> m_vertices;
		#if OPENGL_USE_UINT_INDICES > 0
		std::vector<Uint32> m_indices;
		#else
		std::vector<Uint16> m_indices;
		#endif
		std::vector<OpenglTexture> m_spritesAtlas;
		U32BGLTextures m_automapTiles;
		U64BGLTextures m_sprites;
		std::circular_buffer<Uint32, MAX_AUTOMAPTILES> m_automapTilesBuff;
		std::circular_buffer<Uint64, MAX_SPRITES> m_spritesIds;

		OpenglTexture* m_pictures = NULL;
		char* m_software = NULL;
		char* m_hardware = NULL;

		SDL_GLContext m_oglContext = NULL;
		OpenglTexture* m_renderTarget = NULL;

		OpenglTexture m_gameWindow;
		OpenglTexture m_scaled_gameWindow;
		OpenglTexture m_backup;
		OpenglTexture* m_binded_texture = NULL;

		PFN_OglBindTexture OglBindTexture;
		PFN_OglGenTextures OglGenTextures;
		PFN_OglDeleteTextures OglDeleteTextures;
		PFN_OglTexImage2D OglTexImage2D;
		PFN_OglTexSubImage2D OglTexSubImage2D;
		PFN_OglTexParameteri OglTexParameteri;
		PFN_OglTexEnvi OglTexEnvi;
		PFN_OglPixelStorei OglPixelStorei;
		PFN_OglDrawBuffer OglDrawBuffer;
		PFN_OglReadBuffer OglReadBuffer;
		PFN_OglCopyTexSubImage2D OglCopyTexSubImage2D;
		PFN_OglPolygonMode OglPolygonMode;
		PFN_OglEnable OglEnable;
		PFN_OglDisable OglDisable;
		PFN_OglEnableClientState OglEnableClientState;
		PFN_OglDisableClientState OglDisableClientState;
		PFN_OglDrawArrays OglDrawArrays;
		PFN_OglDrawElements OglDrawElements;
		PFN_OglBlendFunc OglBlendFunc;
		PFN_OglColorMask OglColorMask;
		PFN_OglTexCoordPointer OglTexCoordPointer;
		PFN_OglVertexPointer OglVertexPointer;
		PFN_OglColorPointer OglColorPointer;
		PFN_OglReadPixels OglReadPixels;
		PFN_OglGetIntegerv OglGetIntegerv;
		PFN_OglLoadIdentity OglLoadIdentity;
		PFN_OglMatrixMode OglMatrixMode;
		PFN_OglOrtho OglOrtho;
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

		PFN_OglAttachObjectARB OglAttachObjectARB;
		PFN_OglCompileShaderARB OglCompileShaderARB;
		PFN_OglCreateProgramObjectARB OglCreateProgramObjectARB;
		PFN_OglCreateShaderObjectARB OglCreateShaderObjectARB;
		PFN_OglDeleteObjectARB OglDeleteObjectARB;
		PFN_OglGetObjectParameterivARB OglGetObjectParameterivARB;
		PFN_OglGetUniformLocationARB OglGetUniformLocationARB;
		PFN_OglLinkProgramARB OglLinkProgramARB;
		PFN_OglShaderSourceARB OglShaderSourceARB;
		PFN_OglUniform1iARB OglUniform1iARB;
		PFN_OglUniform2fARB OglUniform2fARB;
		PFN_OglUseProgramObjectARB OglUseProgramObjectARB;

		OpenglShader m_program_sharpen;
		Sint32 m_sharpen_textureSize = -1;

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
		bool m_useAuxBuffer = false;
		bool m_useNonPower2 = false;
		bool m_haveSharpening = false;
};

#define OGL_TEXTURE_VERTEX_SHADER                               \
"varying vec4 v_color;\n"                                       \
"varying vec2 v_texCoord;\n\n"                                  \
"void main() {\n"                                               \
"    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n" \
"    v_color = gl_Color;\n"                                     \
"    v_texCoord = vec2(gl_MultiTexCoord0);\n"                   \
"}"                                                             \

#define OGL_SHARPEN_PIXEL_SHADER                                                              \
"varying vec4 v_color;\n"                                                                     \
"varying vec2 v_texCoord;\n"                                                                  \
"uniform sampler2D tex0;\n"                                                                   \
"uniform vec2 textureSize;\n\n"                                                               \
"#define sharp_clamp ( 0.050000 )\n"                                                          \
"#define sharp_strength ( 0.500000 )\n"                                                       \
"#define CoefLuma vec3(0.2126, 0.7152, 0.0722)\n\n"                                           \
"void main() {\n"                                                                             \
"	vec3 origset = texture2D(tex0, v_texCoord).rgb;\n"                                        \
"	vec3 sharp_strength_luma = (CoefLuma * sharp_strength);\n"                                \
"	vec2 textureSize2 = vec2(textureSize.x, -textureSize.y);\n"                               \
"	vec3 blur_sample;\n"                                                                      \
"	blur_sample  = texture2D(tex0, v_texCoord + textureSize2).rgb;\n"                         \
"	blur_sample += texture2D(tex0, v_texCoord - textureSize).rgb;\n"                          \
"	blur_sample += texture2D(tex0, v_texCoord + textureSize).rgb;\n"                          \
"	blur_sample += texture2D(tex0, v_texCoord - textureSize2).rgb;\n"                         \
"	blur_sample *= 0.25;\n"                                                                   \
"	vec3 sharp = origset - blur_sample;\n"                                                    \
"	vec4 sharp_strength_luma_clamp = vec4(sharp_strength_luma * (0.5 / sharp_clamp), 0.5);\n" \
"	float sharp_luma = clamp(dot(vec4(sharp, 1.0), sharp_strength_luma_clamp), 0.0, 1.0);\n"  \
"	sharp_luma = (sharp_clamp * 2.0) * sharp_luma - sharp_clamp;\n"                           \
"	vec3 outputcolor = origset + sharp_luma;\n"                                               \
"	gl_FragColor = vec4(clamp(outputcolor, 0.0, 1.0), 1.0);\n"                                \
"}"                                                                                           \

#endif
#endif /* __FILE_SURFACE_OPENGL_h_ */
