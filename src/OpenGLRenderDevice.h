/*
Copyright © 2014 Igor Paliychuk

This file is part of FLARE.

FLARE is free software: you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.

FLARE is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
FLARE.  If not, see http://www.gnu.org/licenses/
*/

#ifndef OPENGL_RENDERDEVICE_H
#define OPENGL_RENDERDEVICE_H

#include "RenderDevice.h"

/*
 *
 * We should use GLES instead of GLEW
 * #include "SDL_opengles2.h"
 *
 */

#include <GL/glew.h>
#include "SDL_opengl.h"

/** Provide rendering device using OpenGL backend.
 *
 * Provide an OpenGL implementation for renderning a Renderable to
 * the screen.
 *
 * As this is for the FLARE engine, the implementation uses the engine's
 * global settings context, which is included by the interface.
 *
 * @class OpenGLRenderDevice
 * @see RenderDevice
 *
 */


/** SDL Image */

/** SDL Image */
class OpenGLImage : public Image {
public:
	OpenGLImage(RenderDevice *device);
	virtual ~OpenGLImage();
	int getWidth() const;
	int getHeight() const;

	void fillWithColor(Uint32 color);
	void drawPixel(int x, int y, Uint32 color);
	Uint32 MapRGB(Uint8 r, Uint8 g, Uint8 b);
	Uint32 MapRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
	Image* resize(int width, int height);

	GLuint texture;
	GLuint normalTexture;

private:
	Uint32 readPixel(int x, int y);
};

class OpenGLRenderDevice : public RenderDevice {

public:

	OpenGLRenderDevice();
	int createContext(int width, int height);
	Rect getContextSize();

	virtual int render(Renderable& r, Rect dest);
	virtual int render(Sprite* r);
	virtual int renderToImage(Image* src_image, Rect& src, Image* dest_image, Rect& dest, bool dest_is_transparent = false);

	int renderText(TTF_Font *ttf_font, const std::string& text, Color color, Rect& dest);
	Image *renderTextToImage(TTF_Font* ttf_font, const std::string& text, Color color, bool blended = true);
	void drawPixel(int x, int y, Uint32 color);
	void drawRectangle(const Point& p0, const Point& p1, Uint32 color);
	void blankScreen();
	void commitFrame();
	void destroyContext();
	Uint32 MapRGB(Uint8 r, Uint8 g, Uint8 b);
	Uint32 MapRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
	Image *createImage(int width, int height);
	void setGamma(float g);
	void listModes(std::vector<Rect> &modes);
	void freeImage(Image *image);

	Image* loadImage(std::string filename,
								std::string errormessage = "Couldn't load image",
								bool IfNotFoundExit = false);
private:
	void drawLine(int x0, int y0, int x1, int y1, Uint32 color);
	void setSDL_RGBA(Uint32 *rmask, Uint32 *gmask, Uint32 *bmask, Uint32 *amask);

	int buildResources();
	void* openShaderFile(const char *filename, GLint *length);
	GLuint getShader(GLenum type, const char *filename);
	GLuint createProgram(GLuint vertex_shader, GLuint fragment_shader);
	GLuint createBuffer(GLenum target, const void *buffer_data, GLsizei buffer_size);
	void composeFrame(GLfloat* offset, GLfloat* texelOffset, bool withLight = false);
	SDL_Surface* copyTextureToSurface(GLuint texture);

	SDL_Window *screen;
	SDL_GLContext renderer;
	SDL_Surface* titlebar_icon;

    GLuint vertex_buffer, element_buffer;
    GLuint m_vertex_shader, m_fragment_shader, m_program;

    struct {
        GLint texture;
        GLint normals;
		GLint light;
		GLint offset;
		GLint texelOffset;
    } uniforms;

    struct {
        GLint position;
    } attributes;

	GLushort elementBufferData[4];
	GLfloat positionData[8];
	GLfloat m_offset[4];
	GLfloat m_texelOffset[4];
};

#endif // OPENGL_RENDERDEVICE_H
