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
#include <string.h>
#include "SDL_gfxBlitFunc.h"

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

	int bpp = surface->format->BytesPerPixel;
	/* Here p is the address to the pixel we want to set */
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

	switch(bpp) {
		case 1:
			*p = pixel;
			break;

		case 2:
			*(Uint16 *)p = pixel;
			break;

		case 3:
#if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
			p[0] = (pixel >> 16) & 0xff;
			p[1] = (pixel >> 8) & 0xff;
			p[2] = pixel & 0xff;
#else
			p[0] = pixel & 0xff;
			p[1] = (pixel >> 8) & 0xff;
			p[2] = (pixel >> 16) & 0xff;
#endif
			break;

		case 4:
			*(Uint32 *)p = pixel;
			break;
	}
}

Uint32 OpenGLImage::MapRGB(Uint8 r, Uint8 g, Uint8 b) {
	if (!surface) return 0;
	return SDL_MapRGB(surface->format, r, g, b);
}

Uint32 OpenGLImage::MapRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	if (!surface) return 0;
	return SDL_MapRGBA(surface->format, r, g, b, a);
}

/**
 * Resizes an image
 * Deletes the original image and returns a pointer to the resized version
 */
Image* OpenGLImage::resize(int width, int height) {
	if(!surface || width <= 0 || height <= 0)
		return NULL;

	OpenGLImage *scaled = new OpenGLImage(device);

	if (scaled) {
		scaled->surface = SDL_CreateRGBSurface(surface->flags, width, height,
											   surface->format->BitsPerPixel,
											   surface->format->Rmask,
											   surface->format->Gmask,
											   surface->format->Bmask,
											   surface->format->Amask);

		if (scaled->surface) {
			double _stretch_factor_x, _stretch_factor_y;
			_stretch_factor_x = width / (double)surface->w;
			_stretch_factor_y = height / (double)surface->h;

			for(Uint32 y = 0; y < (Uint32)surface->h; y++) {
				for(Uint32 x = 0; x < (Uint32)surface->w; x++) {
					Uint32 spixel = readPixel(x, y);
					for(Uint32 o_y = 0; o_y < _stretch_factor_y; ++o_y) {
						for(Uint32 o_x = 0; o_x < _stretch_factor_x; ++o_x) {
							Uint32 dx = (Sint32)(_stretch_factor_x * x) + o_x;
							Uint32 dy = (Sint32)(_stretch_factor_y * y) + o_y;
							scaled->drawPixel(dx, dy, spixel);
						}
					}
				}
			}
			// delete the old image and return the new one
			this->unref();
			return scaled;
		}
		else {
			delete scaled;
		}
	}

	return NULL;
}

Uint32 OpenGLImage::readPixel(int x, int y) {
	if (!surface) return 0;

	SDL_LockSurface(surface);
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	Uint32 pixel;

	switch (bpp) {
		case 1:
			pixel = *p;
			break;

		case 2:
			pixel = *(Uint16 *)p;
			break;

		case 3:
			if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
				pixel = p[0] << 16 | p[1] << 8 | p[2];
			else
				pixel = p[0] | p[1] << 8 | p[2] << 16;
			break;

		case 4:
			pixel = *(Uint32 *)p;
			break;

		default:
			SDL_UnlockSurface(surface);
			return 0;
	}

	SDL_UnlockSurface(surface);
	return pixel;
}

OpenGLRenderDevice::OpenGLRenderDevice()
	: screen(NULL)
	, renderer(NULL)
	, titlebar_icon(NULL) {
	cout << "Using Render Device: OpenGLRenderDevice (hardware, SDL2/OpenGL)" << endl;

	positionData[0] = -1.0f; positionData[1] = -1.0f;
	positionData[2] =  1.0f; positionData[3] = -1.0f;
	positionData[4] = -1.0f; positionData[5] =  1.0f;
	positionData[6] =  1.0f; positionData[7] =  1.0f;

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
		logError("OpenGLRenderDevice: createContext() failed: %s\n", SDL_GetError());
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

GLuint OpenGLRenderDevice::createProgram(GLuint vertex_shader, GLuint fragment_shader)
{
    GLint program_ok;

    GLuint program = glCreateProgram();

    glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
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
	int width = src.w;
	int height = src.h;
	SDL_Surface* surface;
	void *pixels;
	bool patch = false;

	if (src.x != 0 || src.y != 0 || src.w != image->surface->w || src.h != image->surface->h)
	{
		patch = true;
	}

	if (patch)
	{
		surface = SDL_CreateRGBSurface(image->surface->flags, src.w, src.h, image->surface->format->BitsPerPixel,
																						 image->surface->format->Rmask,
																						 image->surface->format->Gmask,
																						 image->surface->format->Bmask,
																						 image->surface->format->Amask);
		SDL_BlitSurface(image->surface, &src, surface, 0);

		pixels = surface->pixels;
	}
	else
	{
		pixels = image->surface->pixels;
	}
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
        4,                    /* internal format */
        width, height, 0,           /* width, height, border */
		GL_BGRA, GL_UNSIGNED_BYTE,   /* external format, type */
        pixels                      /* pixels */
    );

	if (patch)
		SDL_FreeSurface(surface);

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

int OpenGLRenderDevice::createFrameBuffer()
{
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

	clearBufferTexture();

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, destTexture, 0);

	GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, drawBuffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		return -1;
	}

	return 0;
}

void OpenGLRenderDevice::clearBufferTexture()
{
	glGenTextures(1, &destTexture);
	glBindTexture(GL_TEXTURE_2D, destTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, VIEW_W, VIEW_H, 0, GL_BGRA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

int OpenGLRenderDevice::buildResources()
{
	vertex_buffer = createBuffer(GL_ARRAY_BUFFER, positionData, sizeof(positionData));
    element_buffer = createBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferData, sizeof(elementBufferData));

    vertex_shader = getShader(GL_VERTEX_SHADER, "D:\\media\\repos\\flare-engine\\shaders\\vertex.glsl");
    if (vertex_shader == 0)
        return 1;

	fragment_shader = getShader(GL_FRAGMENT_SHADER, "D:\\media\\repos\\flare-engine\\shaders\\fragment.glsl");
    if (vertex_shader == 0)
        return 1;

	program = createProgram(vertex_shader, fragment_shader);
    if (program == 0)
        return 1;

    attributes.position = glGetAttribLocation(program, "position");

    uniforms.texture = glGetUniformLocation(program, "texture");
	uniforms.scaleX = glGetUniformLocation(program, "scaleX");
	uniforms.scaleY = glGetUniformLocation(program, "scaleY");
	uniforms.offsetX = glGetUniformLocation(program, "offsetX");
	uniforms.offsetY = glGetUniformLocation(program, "offsetY");

	//createFrameBuffer();

    return 0;
}

int OpenGLRenderDevice::render(Sprite *r) {
	if (r == NULL) {
		return -1;
	}

	if ( !localToGlobal(r) ) {
		return -1;
	}

	// negative x and y clip causes weird stretching
	// adjust for that here
	if (m_clip.x < 0) {
		m_clip.w -= abs(m_clip.x);
		m_dest.x += abs(m_clip.x);
		m_clip.x = 0;
	}
	if (m_clip.y < 0) {
		m_clip.h -= abs(m_clip.y);
		m_dest.y += abs(m_clip.y);
		m_clip.y = 0;
	}

	m_dest.w = m_clip.w;
	m_dest.h = m_clip.h;

	SDL_Rect src = m_clip;
	SDL_Rect dest = m_dest;

	scale.x = (float)src.w/VIEW_W;
	scale.y = (float)src.h/VIEW_H;

	offset.x = 2.0f * (float)dest.x/VIEW_W;
	offset.y = 2.0f * (float)dest.y/VIEW_H;

	//glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

    texture = getTexturePatch(static_cast<OpenGLImage *>(r->getGraphics()), src);

    if (texture == 0)
        return 1;

    glActiveTexture(GL_TEXTURE0);
    //glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture);

	composeFrame(scale, offset);

	glDeleteTextures(1, &texture);

	return 0;
}

void OpenGLRenderDevice::composeFrame(FPoint scale, FPoint offset)
{
	glUseProgram(program);

    glUniform1i(uniforms.texture, 0);
	glUniform1f(uniforms.scaleX, scale.x);
	glUniform1f(uniforms.scaleY, scale.y);
	glUniform1f(uniforms.offsetX, offset.x);
	glUniform1f(uniforms.offsetY, offset.y);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glVertexAttribPointer(
        attributes.position,
        2, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat)*2, (void*)0
    );

    glEnableVertexAttribArray(attributes.position);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
    glDrawElements(
        GL_TRIANGLE_STRIP,  /* mode */
        4,                  /* count */
        GL_UNSIGNED_SHORT,  /* type */
        (void*)0            /* element array buffer offset */
    );

    glDisableVertexAttribArray(attributes.position);
}

int OpenGLRenderDevice::renderToImage(Image* src_image, Rect& src, Image* dest_image, Rect& dest, bool dest_is_transparent) {
	if (!src_image || !dest_image) return -1;

	SDL_Rect _src = src;
	SDL_Rect _dest = dest;

	if (dest_is_transparent)
		return SDL_gfxBlitRGBA(static_cast<OpenGLImage *>(src_image)->surface, &_src,
							   static_cast<OpenGLImage *>(dest_image)->surface, &_dest);
	else
		return SDL_BlitSurface(static_cast<OpenGLImage *>(src_image)->surface, &_src,
							   static_cast<OpenGLImage *>(dest_image)->surface, &_dest);
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
	if (!image) return NULL;

	SDL_Color _color = color;

	if (blended)
		image->surface = TTF_RenderUTF8_Blended(ttf_font, text.c_str(), _color);
	else
		image->surface = TTF_RenderUTF8_Solid(ttf_font, text.c_str(), _color);

	if (image->surface)
		return image;

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
	// FIXME: clear buffer texture 
	//glDeleteTextures(1, &destTexture);
	//clearBufferTexture();

	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT);

	return;
}

void OpenGLRenderDevice::commitFrame() {

	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, destTexture);
	//composeFrame(FPoint(), FPoint());

	glFlush();
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
		logError("OpenGLRenderDevice: CreateRGBSurface failed: %s\n", SDL_GetError());
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
			logError("OpenGLRenderDevice: %s: %s\n", errormessage.c_str(), IMG_GetError());
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
