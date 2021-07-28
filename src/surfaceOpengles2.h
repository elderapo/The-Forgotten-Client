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

#ifndef __FILE_SURFACE_OPENGLES2_h_
#define __FILE_SURFACE_OPENGLES2_h_

#include "engine.h"

#if defined(SDL_VIDEO_RENDER_OGL_ES2)

#if defined(__linux__)
#include <GL/gl.h>
#endif

#define OPENGLES2_MAX_VERTICES 43656
#define OPENGLES2_MAX_INDICES (OPENGLES2_MAX_VERTICES * 6 / 4)
#if OPENGLES2_MAX_INDICES >= 65500
#define OPENGLES2_USE_UINT_INDICES 1
#else
#define OPENGLES2_USE_UINT_INDICES 0
#endif

typedef void (APIENTRY *PFN_OglActiveTexture)(unsigned int texture);
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
typedef void (APIENTRY *PFN_OglEnableVertexAttribArray)(unsigned int index);
typedef void (APIENTRY *PFN_OglDisableVertexAttribArray)(unsigned int index);
typedef void (APIENTRY *PFN_OglDrawArrays)(unsigned int mode, int first, int count);
typedef void (APIENTRY *PFN_OglDrawElements)(unsigned int mode, int count, unsigned int type, const void* indices);
typedef void (APIENTRY *PFN_OglVertexAttribPointer)(unsigned int index, int size, unsigned int type, unsigned char normalized, int stride, const void* pointer);
typedef void (APIENTRY *PFN_OglBlendFuncSeparate)(unsigned int sfactorRGB, unsigned int dfactorRGB, unsigned int sfactorAlpha, unsigned int dfactorAlpha);
typedef void (APIENTRY *PFN_OglBlendEquationSeparate)(unsigned int modeRGB, unsigned int modeAlpha);
typedef void (APIENTRY *PFN_OglColorMask)(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha);
typedef void (APIENTRY *PFN_OglReadPixels)(int x, int y, int width, int height, unsigned int format, unsigned int type, void* pixels);
typedef void (APIENTRY *PFN_OglGetIntegerv)(unsigned int pname, int* params);
typedef void (APIENTRY *PFN_OglGetBooleanv)(unsigned int pname, unsigned char* params);
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

typedef void (APIENTRY *PFN_OglGenBuffers)(int n, unsigned int* buffers);
typedef void (APIENTRY *PFN_OglDeleteBuffers)(int n, const unsigned int* buffers);
typedef void (APIENTRY *PFN_OglBindBuffer)(unsigned int target, unsigned int buffer);
typedef void (APIENTRY *PFN_OglBufferData)(unsigned int target, ptrdiff_t size, const void* data, unsigned int usage);
typedef void (APIENTRY *PFN_OglBufferSubData)(unsigned int target, ptrdiff_t offset, ptrdiff_t size, const void* data);

typedef unsigned int (APIENTRY *PFN_OglCreateShader)(unsigned int type);
typedef void (APIENTRY *PFN_OglDeleteShader)(unsigned int shader);
typedef void (APIENTRY *PFN_OglCompileShader)(unsigned int shader);
typedef unsigned int (APIENTRY *PFN_OglCreateProgram)();
typedef void (APIENTRY *PFN_OglDeleteProgram)(unsigned int program);
typedef void (APIENTRY *PFN_OglUseProgram)(unsigned int program);
typedef void (APIENTRY *PFN_OglLinkProgram)(unsigned int program);
typedef void (APIENTRY *PFN_OglAttachShader)(unsigned int program, unsigned int shader);
typedef void (APIENTRY *PFN_OglShaderSource)(unsigned int shader, int count, const char** string, const int* length);
typedef void (APIENTRY *PFN_OglGetProgramiv)(unsigned int program, unsigned int pname, int* params);
typedef void (APIENTRY *PFN_OglGetShaderiv)(unsigned int shader, unsigned int pname, int* params);
typedef void (APIENTRY *PFN_OglGetProgramInfoLog)(unsigned int program, int bufSize, int* length, char* infoLog);
typedef void (APIENTRY *PFN_OglGetShaderInfoLog)(unsigned int shader, int bufSize, int* length, char* infoLog);
typedef void (APIENTRY *PFN_OglBindAttribLocation)(unsigned int program, unsigned int index, const char* name);
typedef int (APIENTRY *PFN_OglGetUniformLocation)(unsigned int program, const char* name);
typedef void (APIENTRY *PFN_OglUniform1i)(int location, int v0);
typedef void (APIENTRY *PFN_OglUniform4f)(int location, float v0, float v1, float v2, float v3);
typedef void (APIENTRY *PFN_OglUniformMatrix4fv)(int location, int count, unsigned char transpose, const float* value);

struct OpenglES2Program
{
	OpenglES2Program() : program(0), vert_shader(0), pix_shader(0), projectionLocation(-1) {}

	unsigned int program;
	unsigned int vert_shader;
	unsigned int pix_shader;

	int projectionLocation;
};

struct OpenglES2Texture
{
	OpenglES2Texture() : m_texture(0), m_framebuffer(0) {}

	operator bool() const {return (m_texture != 0);}

	// non-copyable
	OpenglES2Texture(const OpenglES2Texture&) = delete;
	OpenglES2Texture& operator=(const OpenglES2Texture&) = delete;

	// moveable
	OpenglES2Texture(OpenglES2Texture&& rhs) noexcept : m_texture(rhs.m_texture), m_framebuffer(rhs.m_framebuffer),
		m_width(rhs.m_width), m_height(rhs.m_height), m_scaleW(rhs.m_scaleW), m_scaleH(rhs.m_scaleH)
	{
		rhs.m_texture = 0;
		rhs.m_framebuffer = 0;
	}
	OpenglES2Texture& operator=(OpenglES2Texture&& rhs) noexcept
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

struct OpenglES2SpriteData
{
	OpenglES2SpriteData() : m_xOffset(0), m_yOffset(0), m_surface(0), m_lastUsage(0) {}

	Uint32 m_xOffset;
	Uint32 m_yOffset;
	Uint32 m_surface;
	Uint32 m_lastUsage;
};

struct OpenglES2Vertex
{
	OpenglES2Vertex(float x, float y, float u, float v, DWORD color) : x(x), y(y), u(u), v(v), color(color) {}
	OpenglES2Vertex(float x, float y, DWORD color) : x(x), y(y), u(0.0f), v(0.0f), color(color) {}

	float x, y;
	float u, v;
	DWORD color;
};

typedef enum
{
	GLES_ATTRIBUTE_POSITION = 0,
	GLES_ATTRIBUTE_TEXCOORD = 1,
	GLES_ATTRIBUTE_COLORS = 2,
	GLES_ATTRIBUTES = 3,
} GLES_Attribute;

typedef robin_hood::unordered_map<Uint32, OpenglES2Texture> U32BGLES2Textures;
typedef robin_hood::unordered_map<Uint64, OpenglES2SpriteData> U64BGLES2Textures;

class SurfaceOpenglES2 : public Surface
{
	public:
		SurfaceOpenglES2();
		virtual ~SurfaceOpenglES2();

		// non-copyable
		SurfaceOpenglES2(const SurfaceOpenglES2&) = delete;
		SurfaceOpenglES2& operator=(const SurfaceOpenglES2&) = delete;

		// non-moveable
		SurfaceOpenglES2(const SurfaceOpenglES2&&) = delete;
		SurfaceOpenglES2& operator=(const SurfaceOpenglES2&&) = delete;

		bool createOpenGLES2Shader(unsigned int& shader, const char* data, bool vertex_shader);
		bool createOpenGLES2Program(OpenglES2Program& program, unsigned int vertex_shader, unsigned int pixel_shader);
		void selectShader(OpenglES2Program& program);
		void releaseOpenGLES2Program(OpenglES2Program& program);
		void releaseOpenGLES2Shader(unsigned int& shader);

		bool createOpenGLES2Texture(OpenglES2Texture& texture, Uint32 width, Uint32 height, bool linearSampler, bool frameBuffer = false);
		bool updateTextureData(OpenglES2Texture& texture, unsigned char* data);
		void releaseOpenGLES2Texture(OpenglES2Texture& texture);

		void updateVertexBuffer(const void* vertexData, ptrdiff_t dataSizeInBytes);
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

		OpenglES2Texture* getTextureIndex(OpenglES2Texture* texture);
		void drawQuad(OpenglES2Texture* texture, float vertices[8], float texcoords[8]);
		void drawQuad(OpenglES2Texture* texture, float vertices[8], float texcoords[8], DWORD color);
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

		OpenglES2Texture* loadPicture(Uint16 pictureId, bool linear);
		virtual void drawFont(Uint16 pictureId, Sint32 x, Sint32 y, const std::string& text, size_t pos, size_t len, Uint8 r, Uint8 g, Uint8 b, Sint16 cX[256], Sint16 cY[256], Sint16 cW[256], Sint16 cH[256]);
		virtual void drawBackground(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h);
		virtual void drawPictureRepeat(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h);
		virtual void drawPicture(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 x, Sint32 y, Sint32 w, Sint32 h);

		bool loadSprite(Uint32 spriteId, OpenglES2Texture* texture, Uint32 xoff, Uint32 yoff);
		bool loadSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Uint32 outfitColor, OpenglES2Texture* texture, Uint32 xoff, Uint32 yoff);
		virtual void drawSprite(Uint32 spriteId, Sint32 x, Sint32 y);
		virtual void drawSprite(Uint32 spriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh);
		virtual void drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Uint32 outfitColor);
		virtual void drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Uint32 outfitColor);

		OpenglES2Texture* createAutomapTile(Uint32 currentArea);
		void uploadAutomapTile(OpenglES2Texture* texture, Uint8 color[256][256]);
		virtual void drawAutomapTile(Uint32 m_currentArea, bool& m_recreate, Uint8 m_color[256][256], Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh);

	protected:
		std::vector<OpenglES2Vertex> m_vertices;
		std::vector<OpenglES2Texture> m_spritesAtlas;
		U32BGLES2Textures m_automapTiles;
		U64BGLES2Textures m_sprites;
		std::circular_buffer<Uint32, MAX_AUTOMAPTILES> m_automapTilesBuff;
		std::circular_buffer<Uint64, MAX_SPRITES> m_spritesIds;

		OpenglES2Texture* m_pictures = NULL;
		char* m_software = NULL;
		char* m_hardware = NULL;

		SDL_GLContext m_oglContext = NULL;
		OpenglES2Texture* m_renderTarget = NULL;

		OpenglES2Texture m_gameWindow;
		OpenglES2Texture m_scaled_gameWindow;
		OpenglES2Texture* m_binded_texture = NULL;

		PFN_OglActiveTexture OglActiveTexture;
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
		PFN_OglEnableVertexAttribArray OglEnableVertexAttribArray;
		PFN_OglDisableVertexAttribArray OglDisableVertexAttribArray;
		PFN_OglDrawArrays OglDrawArrays;
		PFN_OglDrawElements OglDrawElements;
		PFN_OglVertexAttribPointer OglVertexAttribPointer;
		PFN_OglBlendFuncSeparate OglBlendFuncSeparate;
		PFN_OglBlendEquationSeparate OglBlendEquationSeparate;
		PFN_OglColorMask OglColorMask;
		PFN_OglReadPixels OglReadPixels;
		PFN_OglGetIntegerv OglGetIntegerv;
		PFN_OglGetBooleanv OglGetBooleanv;
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

		PFN_OglGenBuffers OglGenBuffers;
		PFN_OglDeleteBuffers OglDeleteBuffers;
		PFN_OglBindBuffer OglBindBuffer;
		PFN_OglBufferData OglBufferData;
		PFN_OglBufferSubData OglBufferSubData;

		PFN_OglCreateShader OglCreateShader;
		PFN_OglDeleteShader OglDeleteShader;
		PFN_OglCompileShader OglCompileShader;
		PFN_OglCreateProgram OglCreateProgram;
		PFN_OglDeleteProgram OglDeleteProgram;
		PFN_OglUseProgram OglUseProgram;
		PFN_OglLinkProgram OglLinkProgram;
		PFN_OglAttachShader OglAttachShader;
		PFN_OglShaderSource OglShaderSource;
		PFN_OglGetProgramiv OglGetProgramiv;
		PFN_OglGetShaderiv OglGetShaderiv;
		PFN_OglGetProgramInfoLog OglGetProgramInfoLog;
		PFN_OglGetShaderInfoLog OglGetShaderInfoLog;
		PFN_OglBindAttribLocation OglBindAttribLocation;
		PFN_OglGetUniformLocation OglGetUniformLocation;
		PFN_OglUniform1i OglUniform1i;
		PFN_OglUniform4f OglUniform4f;
		PFN_OglUniformMatrix4fv OglUniformMatrix4fv;

		OpenglES2Program m_programSolid;
		OpenglES2Program m_programTexture;
		OpenglES2Program m_programSharpen;

		Sint32 m_maxTextureSize = 1024;
		Sint32 m_integer_scaling_width = 0;
		Sint32 m_integer_scaling_height = 0;

		Sint32 m_sharpen_textureSize = -1;
		unsigned int window_framebuffer = 0;

		unsigned int m_vertex_shader = 0;
		unsigned int m_solid_pixel_shader = 0;
		unsigned int m_texture_pixel_shader = 0;
		unsigned int m_sharpen_pixel_shader = 0;

		unsigned int m_index_buffer = 0;
		unsigned int m_vertex_buffer = 0;
		ptrdiff_t m_vertex_buffer_size = 0;

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

		bool m_haveSharpening = false;
};

#define GLES_VERTEX_SHADER                                       \
"uniform mat4 u_projection;\n"                                   \
"attribute vec2 a_position;\n"                                   \
"attribute vec2 a_texCoord;\n"                                   \
"attribute vec4 a_color;\n"                                      \
"varying vec2 v_texCoord;\n"                                     \
"varying vec4 v_color;\n\n"                                      \
"void main() {\n"                                                \
"    gl_Position = u_projection * vec4(a_position, 0.0, 1.0);\n" \
"    gl_PointSize = 1.0;\n"                                      \
"    v_texCoord = a_texCoord;\n"                                 \
"    v_color = a_color;\n"                                       \
"}"                                                              \

#define GLES_PIXEL_SHADER                                        \
"precision mediump float;\n"                                     \
"varying vec4 v_color;\n\n"                                      \
"void main() {\n"                                                \
"    gl_FragColor = v_color.bgra;\n"                             \
"}"                                                              \

#define GLES_TEXTURE_PIXEL_SHADER                                \
"precision mediump float;\n"                                     \
"uniform sampler2D u_texture;\n"                                 \
"varying vec4 v_color;\n"                                        \
"varying vec2 v_texCoord;\n\n"                                   \
"void main() {\n"                                                \
"    gl_FragColor = texture2D(u_texture, v_texCoord);\n"         \
"    gl_FragColor *= v_color.bgra;\n"                            \
"}"                                                              \

#define GLES_SHARPEN_PIXEL_SHADER                                                              \
"precision mediump float;\n"                                                                   \
"uniform sampler2D u_texture;\n"                                                               \
"uniform vec2 textureSize;\n"                                                                  \
"varying vec2 v_texCoord;\n\n"                                                                 \
"#define sharp_clamp ( 0.050000 )\n"                                                           \
"#define sharp_strength ( 0.500000 )\n"                                                        \
"#define CoefLuma vec3(0.2126, 0.7152, 0.0722)\n\n"                                            \
"void main() {\n"                                                                              \
"    vec3 origset = texture2D(u_texture, v_texCoord).rgb;\n"                                   \
"    vec3 sharp_strength_luma = (CoefLuma * sharp_strength);\n"                                \
"	 vec2 textureSize2 = vec2(textureSize.x, -textureSize.y);\n"                               \
"    vec3 blur_sample;\n"                                                                      \
"    blur_sample  = texture2D(u_texture, v_texCoord + textureSize2).rgb;\n"                    \
"    blur_sample += texture2D(u_texture, v_texCoord - textureSize).rgb;\n"                     \
"    blur_sample += texture2D(u_texture, v_texCoord + textureSize).rgb;\n"                     \
"    blur_sample += texture2D(u_texture, v_texCoord - textureSize2).rgb;\n"                    \
"    blur_sample *= 0.25;\n"                                                                   \
"    vec3 sharp = origset - blur_sample;\n"                                                    \
"    vec4 sharp_strength_luma_clamp = vec4(sharp_strength_luma * (0.5 / sharp_clamp), 0.5);\n" \
"    float sharp_luma = clamp(dot(vec4(sharp, 1.0), sharp_strength_luma_clamp), 0.0, 1.0);\n"  \
"    sharp_luma = (sharp_clamp * 2.0) * sharp_luma - sharp_clamp;\n"                           \
"    vec3 outputcolor = origset + sharp_luma;\n"                                               \
"    gl_FragColor = vec4(clamp(outputcolor, 0.0, 1.0), 1.0);\n"                                \
"}"                                                                                            \

#endif
#endif /* __FILE_SURFACE_OPENGLES2_h_ */
