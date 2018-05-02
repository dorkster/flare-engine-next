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

#include <iostream>

#include <stdio.h>
#include <stdlib.h>

#include "SharedResources.h"
#include "Settings.h"

#include "OpenGLRenderDevice.h"

using namespace std;

OpenGLImage::OpenGLImage(RenderDevice *_device)
	: Image(_device)
	, surface(NULL) {
}

OpenGLImage::~OpenGLImage() {
}

int OpenGLImage::getWidth() const {
	return surface ? surface->w : 0;
}

int OpenGLImage::getHeight() const {
	return surface ? surface->h : 0;
}

void OpenGLImage::fillWithColor(Uint32 color) {
	if (!surface) return;

	SDL_FillRect(surface, NULL, color);
}

/*
 * Set the pixel at (x, y) to the given value
 */
void OpenGLImage::drawPixel(int x, int y, Uint32 pixel) {
	if (!surface) return;
}

Uint32 OpenGLImage::MapRGB(Uint8 r, Uint8 g, Uint8 b) {
	if (!surface) return 0;
	return SDL_MapRGB(surface->format, r, g, b);
}

Uint32 OpenGLImage::MapRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	if (!surface) return 0;
	return SDL_MapRGBA(surface->format, r, g, b, a);
}

Image* OpenGLImage::resize(int width, int height) {
	if(!surface || width <= 0 || height <= 0)
		return NULL;

	OpenGLImage *scaled = new OpenGLImage(device);
	if (!scaled) return NULL;

	return NULL;
}

OpenGLRenderDevice::OpenGLRenderDevice()
	: screen(NULL)
	, renderer(NULL)
	, titlebar_icon(NULL) {
	cout << "Using Render Device: SDLHardwareRenderDevice (hardware, SDL 2)" << endl;

	elementBufferData[0] = 0;
	elementBufferData[1] = 1;
	elementBufferData[2] = 2;
	elementBufferData[3] = 3;
}

int OpenGLRenderDevice::createContext(int width, int height) {
	if (is_initialized) {
		SDL_GL_DeleteContext(renderer);
		Uint32 flags = 0;

		if (FULLSCREEN) flags = SDL_WINDOW_FULLSCREEN;
		else flags = SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN;

		SDL_DestroyWindow(screen);
		screen = SDL_CreateWindow(msg->get(WINDOW_TITLE).c_str(),
									SDL_WINDOWPOS_CENTERED,
									SDL_WINDOWPOS_CENTERED,
									width, height,
									flags);

		renderer = SDL_GL_CreateContext(screen);
	}
	else {
		Uint32 flags = 0;

		if (FULLSCREEN) flags = SDL_WINDOW_FULLSCREEN;
		else flags = SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN;

		screen = SDL_CreateWindow(msg->get(WINDOW_TITLE).c_str(),
									SDL_WINDOWPOS_CENTERED,
									SDL_WINDOWPOS_CENTERED,
									width, height,
									flags);

		if (screen != NULL) renderer = SDL_GL_CreateContext(screen);

		// TODO: Can we avoid using GLEW?
		glewInit();

		buildResources();
	}

	if (screen != NULL) {
		is_initialized = true;

		// Add Window Titlebar Icon
		if (titlebar_icon == NULL) {
			titlebar_icon = IMG_Load(mods->locate("images/logo/icon.png").c_str());
			SDL_SetWindowIcon(screen, titlebar_icon);
		}

		return 0;
	}
	else {
		logError("SDLHardwareRenderDevice: createContext() failed: %s\n", SDL_GetError());
		SDL_Quit();
		exit(1);
	}
}

Rect OpenGLRenderDevice::getContextSize() {
	Rect size;
	size.x = size.y = 0;
	SDL_GetWindowSize(screen, &size.w, &size.h);

	return size;
}

int OpenGLRenderDevice::render(Renderable& r, Rect dest) {
	SDL_Rect src = r.src;
	SDL_Rect _dest = dest;
	return 0;
}

void OpenGLRenderDevice::relative(SDL_Rect rect, FPoint& point, FPoint& size)
{
	point.x = ((float)rect.x/VIEW_W - 0.5f)/0.5f;
	point.y = ((float)rect.y/VIEW_H - 0.5f)/0.5f;

	size.x = ((float)rect.w/VIEW_W - 0.5f)/0.5f;
	size.y = ((float)rect.h/VIEW_H - 0.5f)/0.5f;
}

void* OpenGLRenderDevice::openShaderFile(const char *filename, GLint *length)
{
    FILE *f = fopen(filename, "r");
    void *buffer;

    if (!f) {
        logError("Unable to open shader file %s for reading\n", filename);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    *length = ftell(f);
    fseek(f, 0, SEEK_SET);

    buffer = malloc(*length+1);
    *length = fread(buffer, 1, *length, f);
    fclose(f);
    ((char*)buffer)[*length] = '\0';

    return buffer;
}

GLuint OpenGLRenderDevice::getShader(GLenum type, const char *filename)
{
    GLint length;
    GLchar *source = (char*)openShaderFile(filename, &length);
    GLuint shader;
    GLint shader_ok;

    if (!source)
        return 1;

    shader = glCreateShader(type);
    glShaderSource(shader, 1, (const GLchar**)&source, &length);
    free(source);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_ok);
    if (!shader_ok)
	{
        logError("Failed to compile %s:\n", filename);
        glDeleteShader(shader);
        return 1;
    }
    return shader;
}

GLuint OpenGLRenderDevice::createProgram(GLuint vertex_shader)
{
    GLint program_ok;

    GLuint program = glCreateProgram();

    glAttachShader(program, vertex_shader);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &program_ok);
    if (!program_ok)
	{
        logError("Failed to link shader program:\n");
        glDeleteProgram(program);
        return 1;
    }
    return program;
}

GLuint OpenGLRenderDevice::getTexturePatch(OpenGLImage* image, SDL_Rect src)
{
	// TODO: cut out surface patch
	int width = image->getWidth();
	int height = image->getHeight();
	void *pixels = image->surface->pixels;
    GLuint texture;

    if (!pixels)
        return 1;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
    glTexImage2D(
        GL_TEXTURE_2D, 0,           /* target, level */
        GL_RGBA,                    /* internal format */
        width, height, 0,           /* width, height, border */
        GL_RGBA, GL_UNSIGNED_BYTE,   /* external format, type */
        pixels                      /* pixels */
    );

    return texture;
}

GLuint OpenGLRenderDevice::createBuffer(GLenum target, const void *buffer_data, GLsizei buffer_size)
{
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(target, buffer);
    glBufferData(target, buffer_size, buffer_data, GL_STATIC_DRAW);
    return buffer;
}

int OpenGLRenderDevice::buildResources()
{
    element_buffer = createBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferData, sizeof(elementBufferData));

    vertex_shader = getShader(GL_VERTEX_SHADER, "D:\\media\\repos\\flare-engine\\shaders\\vertex.glsl");
    if (vertex_shader == 0)
        return 1;

    program = createProgram(vertex_shader);
    if (program == 0)
        return 1;

    uniformTexture = glGetUniformLocation(program, "texture");
    attributePosition = glGetAttribLocation(program, "position");

    return 0;
}

int OpenGLRenderDevice::render(Sprite *r) {
	if (r == NULL) {
		return -1;
	}

	if ( !localToGlobal(r) ) {
		return -1;
	}

	SDL_Rect src = m_clip;
	SDL_Rect dest = m_dest;

	FPoint point;
	FPoint size;

	relative(dest, point, size);

	std::vector<GLfloat> bufferData(8);

	bufferData[0] = (point.x);          bufferData[1] = (point.y + size.y);
	bufferData[2] = (point.x + size.x); bufferData[3] = (point.y + size.y);
	bufferData[4] = (point.x);          bufferData[5] = (point.y);
	bufferData[6] = (point.x + size.x); bufferData[7] = (point.y);

	vertex_buffer = createBuffer(GL_ARRAY_BUFFER, bufferData.data(), sizeof(bufferData.data()));

    texture = getTexturePatch(static_cast<OpenGLImage *>(r->getGraphics()), src);

    if (texture == 0)
        return 1;

	glUseProgram(program);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(uniformTexture, 0);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glVertexAttribPointer(
        attributePosition,                /* attribute */
        2,                                /* size */
        GL_FLOAT,                         /* type */
        GL_FALSE,                         /* normalized? */
        sizeof(GLfloat)*2,                /* stride */
        (void*)0                          /* array buffer offset */
    );
    glEnableVertexAttribArray(attributePosition);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
    glDrawElements(
        GL_TRIANGLE_STRIP,  /* mode */
        4,                  /* count */
        GL_UNSIGNED_SHORT,  /* type */
        (void*)0            /* element array buffer offset */
    );

    glDisableVertexAttribArray(attributePosition);

	return 0;
}

int OpenGLRenderDevice::renderToImage(Image* src_image, Rect& src, Image* dest_image, Rect& dest, bool dest_is_transparent) {
	return 0;
}

int OpenGLRenderDevice::renderText(
	TTF_Font *ttf_font,
	const std::string& text,
	Color color,
	Rect& dest
) {
	int ret = 0;

	return ret;
}

Image * OpenGLRenderDevice::renderTextToImage(TTF_Font* ttf_font, const std::string& text, Color color, bool blended) {
	OpenGLImage *image = new OpenGLImage(this);

	delete image;
	return NULL;
}

void OpenGLRenderDevice::drawPixel(
	int x,
	int y,
	Uint32 color
) {
}

void OpenGLRenderDevice::drawLine(
	int x0,
	int y0,
	int x1,
	int y1,
	Uint32 color
) {
}

void OpenGLRenderDevice::drawRectangle(
	const Point& p0,
	const Point& p1,
	Uint32 color
) {
	drawLine(p0.x, p0.y, p1.x, p0.y, color);
	drawLine(p1.x, p0.y, p1.x, p1.y, color);
	drawLine(p0.x, p0.y, p0.x, p1.y, color);
	drawLine(p0.x, p1.y, p1.x, p1.y, color);
}

void OpenGLRenderDevice::blankScreen() {
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT);
	return;
}

void OpenGLRenderDevice::commitFrame() {

	SDL_GL_SwapWindow(screen);
	return;
}

void OpenGLRenderDevice::destroyContext() {
	SDL_FreeSurface(titlebar_icon);
	SDL_GL_DeleteContext(renderer);
	SDL_DestroyWindow(screen);

	return;
}

Uint32 OpenGLRenderDevice::MapRGB(Uint8 r, Uint8 g, Uint8 b) {
	Uint32 u_format = SDL_GetWindowPixelFormat(screen);
	SDL_PixelFormat* format = SDL_AllocFormat(u_format);

	if (format) {
		Uint32 ret = SDL_MapRGB(format, r, g, b);
		SDL_FreeFormat(format);
		return ret;
	}
	else {
		return 0;
	}
}

Uint32 OpenGLRenderDevice::MapRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	Uint32 u_format = SDL_GetWindowPixelFormat(screen);
	SDL_PixelFormat* format = SDL_AllocFormat(u_format);

	if (format) {
		Uint32 ret = SDL_MapRGBA(format, r, g, b, a);
		SDL_FreeFormat(format);
		return ret;
	}
	else {
		return 0;
	}
}

/**
 * create blank surface
 */
Image *OpenGLRenderDevice::createImage(int width, int height) {

	OpenGLImage *image = new OpenGLImage(this);

	if (!image)
		return NULL;

	Uint32 rmask, gmask, bmask, amask;
	setSDL_RGBA(&rmask, &gmask, &bmask, &amask);

#if SDL_VERSION_ATLEAST(2,0,0)
		image->surface = SDL_CreateRGBSurface(0, width, height, BITS_PER_PIXEL, rmask, gmask, bmask, amask);
#else
	if (HWSURFACE)
		image->surface = SDL_CreateRGBSurface(SDL_HWSURFACE|SDL_SRCALPHA, width, height, BITS_PER_PIXEL, rmask, gmask, bmask, amask);
	else
		image->surface = SDL_CreateRGBSurface(SDL_SWSURFACE|SDL_SRCALPHA, width, height, BITS_PER_PIXEL, rmask, gmask, bmask, amask);
#endif

	if(image->surface == NULL) {
		logError("SDLSoftwareRenderDevice: CreateRGBSurface failed: %s\n", SDL_GetError());
		delete image;
		return NULL;
	}

	// optimize
	SDL_Surface *cleanup = image->surface;
#if SDL_VERSION_ATLEAST(2,0,0)
	image->surface = SDL_ConvertSurfaceFormat(cleanup, SDL_PIXELFORMAT_ARGB8888, 0);
#else
	image->surface = SDL_DisplayFormatAlpha(cleanup);
#endif
	SDL_FreeSurface(cleanup);

	return image;
}

void OpenGLRenderDevice::setGamma(float g) {
	Uint16 ramp[256];
	SDL_CalculateGammaRamp(g, ramp);
	SDL_SetWindowGammaRamp(screen, ramp, ramp, ramp);
}

void OpenGLRenderDevice::listModes(std::vector<Rect> &modes) {
	int mode_count = SDL_GetNumDisplayModes(0);

	for (int i=0; i<mode_count; i++) {
		SDL_DisplayMode display_mode;
		SDL_GetDisplayMode(0, i, &display_mode);

		if (display_mode.w == 0 || display_mode.h == 0) continue;

		Rect mode_rect;
		mode_rect.w = display_mode.w;
		mode_rect.h = display_mode.h;
		modes.push_back(mode_rect);

		if (display_mode.w < MIN_VIEW_W || display_mode.h < MIN_VIEW_H) {
			// make sure the resolution fits in the constraints of MIN_VIEW_W and MIN_VIEW_H
			modes.pop_back();
		}
		else {
			// check previous resolutions for duplicates. If one is found, drop the one we just added
			for (unsigned j=0; j<modes.size()-1; ++j) {
				if (modes[j].w == display_mode.w && modes[j].h == display_mode.h) {
					modes.pop_back();
					break;
				}
			}
		}
	}
}

Image *OpenGLRenderDevice::loadImage(std::string filename, std::string errormessage, bool IfNotFoundExit) {
	// lookup image in cache
	Image *img;
	img = cacheLookup(filename);
	if (img != NULL) return img;

	// load image
	OpenGLImage *image;
	image = NULL;
	SDL_Surface *cleanup = IMG_Load(mods->locate(filename).c_str());
	if(!cleanup) {
		if (!errormessage.empty())
			logError("SDLSoftwareRenderDevice: %s: %s\n", errormessage.c_str(), IMG_GetError());
		if (IfNotFoundExit) {
			SDL_Quit();
			exit(1);
		}
	}
	else {
		image = new OpenGLImage(this);
#if SDL_VERSION_ATLEAST(2,0,0)
		image->surface = SDL_ConvertSurfaceFormat(cleanup, SDL_PIXELFORMAT_ARGB8888, 0);
#else
		image->surface = SDL_DisplayFormatAlpha(cleanup);
#endif
		SDL_FreeSurface(cleanup);
	}

	// store image to cache
	cacheStore(filename, image);
	return image;
}

void OpenGLRenderDevice::freeImage(Image *image) {
	if (!image) return;

	cacheRemove(image);

	if (static_cast<OpenGLImage *>(image)->surface)
		SDL_FreeSurface(static_cast<OpenGLImage *>(image)->surface);
}

void OpenGLRenderDevice::setSDL_RGBA(Uint32 *rmask, Uint32 *gmask, Uint32 *bmask, Uint32 *amask) {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	*rmask = 0xff000000;
	*gmask = 0x00ff0000;
	*bmask = 0x0000ff00;
	*amask = 0x000000ff;
#else
	*rmask = 0x000000ff;
	*gmask = 0x0000ff00;
	*bmask = 0x00ff0000;
	*amask = 0xff000000;
#endif
}
