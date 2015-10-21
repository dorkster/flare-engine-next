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

#include "OpenGL_EXT.h"

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

	void fillWithColor(const Color& color);
	void drawPixel(int x, int y, const Color& color);
	Image* resize(int width, int height);

	GLuint texture;
	GLuint normalTexture;
	int width;
	int height;
};

class OpenGLRenderDevice : public RenderDevice {

public:

	OpenGLRenderDevice();
	int createContext();

	virtual int render(Renderable& r, Rect dest);
	virtual int render(Sprite* r);
	virtual int renderToImage(Image* src_image, Rect& src, Image* dest_image, Rect& dest);

	int renderText(FontStyle *font_style, const std::string& text, Color color, Rect& dest);
	Image *renderTextToImage(FontStyle* font_style, const std::string& text, Color color, bool blended = true);
	void drawPixel(int x, int y, const Color& color);
	void drawRectangle(const Point& p0, const Point& p1, const Color& color);
	void blankScreen();
	void commitFrame();
	void destroyContext();
	void windowResize();
	Image *createImage(int width, int height);
	void setGamma(float g);
	void updateTitleBar();
	void freeImage(Image *image);

	Image* loadImage(std::string filename,
								std::string errormessage = "Couldn't load image",
								bool IfNotFoundExit = false);
private:
	void drawLine(int x0, int y0, int x1, int y1, const Color& color);

	int buildResources();
	void* openShaderFile(const char *filename, GLint *length);
	GLuint getShader(GLenum type, const char *filename);
	GLuint createProgram(GLuint vertex_shader, GLuint fragment_shader);
	GLuint createBuffer(GLenum target, const void *buffer_data, GLsizei buffer_size);
	void composeFrame(GLfloat* offset, GLfloat* texelOffset, bool withLight = false);
	void configureFrameBuffer(GLuint frameTexture, int frame_w, int frame_h);
	void disableFrameBuffer(GLint *view_rect);

	SDL_Window *window;
	SDL_GLContext renderer;
	SDL_Surface* titlebar_icon;
	char* title;

	GLuint vertex_buffer, element_buffer;
	GLuint m_vertex_shader, m_fragment_shader, m_program, m_frameBuffer;

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
	GLfloat m_offset[4]; //x, y, width, height
	GLfloat m_texelOffset[4]; // 1/width, x, 1/height, y
};

#endif // OPENGL_RENDERDEVICE_H
