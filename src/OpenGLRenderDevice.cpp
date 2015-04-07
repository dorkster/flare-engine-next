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
	, texture(-1)
	, normalTexture(-1) {
}

OpenGLImage::~OpenGLImage() {
}

int OpenGLImage::getWidth() const {
	int width = 0;
	if ((int)texture == -1) return width;

	glBindTexture(GL_TEXTURE_2D, texture);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
	return width;
}

int OpenGLImage::getHeight() const {
	int height = 0;
	if ((int)texture == -1) return height;

	glBindTexture(GL_TEXTURE_2D, texture);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
	return height;
}

void OpenGLImage::fillWithColor(Uint32 color) {
	if ((int)texture == -1) return;

	int channels = 4;
	int bytes = getWidth() * getHeight() * channels;

	unsigned char* buffer = (unsigned char*)malloc(bytes);

	for(int i = 0; i < bytes; i++)
	{
		buffer[i] = color;
	}

	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, channels, getWidth(), getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

	free(buffer);
}

/*
 * Set the pixel at (x, y) to the given value
 */
void OpenGLImage::drawPixel(int x, int y, Uint32 pixel) {
	if ((int)texture == -1) return;
}

Uint32 OpenGLImage::MapRGB(Uint8 r, Uint8 g, Uint8 b) {
	if ((int)texture == -1) return 0;
	return 0;
}

Uint32 OpenGLImage::MapRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	if ((int)texture == -1) return 0;
	return 0;
}

/**
 * Resizes an image
 * Deletes the original image and returns a pointer to the resized version
 */
Image* OpenGLImage::resize(int width, int height) {
	if((int)texture == -1 || width <= 0 || height <= 0)
		return NULL;
	// UNIMPLEMENTED
	return NULL;
}

Uint32 OpenGLImage::readPixel(int x, int y) {
	if ((int)texture == -1) return 0;
	// UNIMPLEMENTED
	return 0;
}

OpenGLRenderDevice::OpenGLRenderDevice()
	: window(NULL)
	, renderer(NULL)
	, titlebar_icon(NULL)
	, title(NULL)
{
	logInfo("Using Render Device: OpenGLRenderDevice (hardware, SDL2/OpenGL)");

	fullscreen = FULLSCREEN;
	hwsurface = HWSURFACE;
	vsync = VSYNC;
	texture_filter = TEXTURE_FILTER;

	min_screen.x = MIN_SCREEN_W;
	min_screen.y = MIN_SCREEN_H;

	positionData[0] = -1.0f; positionData[1] = -1.0f;
	positionData[2] =  1.0f; positionData[3] = -1.0f;
	positionData[4] = -1.0f; positionData[5] =  1.0f;
	positionData[6] =  1.0f; positionData[7] =  1.0f;

	elementBufferData[0] = 0;
	elementBufferData[1] = 1;
	elementBufferData[2] = 2;
	elementBufferData[3] = 3;
}

int OpenGLRenderDevice::createContext() {
	bool settings_changed = (fullscreen != FULLSCREEN || hwsurface != HWSURFACE || vsync != VSYNC || texture_filter != TEXTURE_FILTER);

	Uint32 w_flags = 0;
	Uint32 r_flags = 0;
	int window_w = SCREEN_W;
	int window_h = SCREEN_H;

	if (FULLSCREEN) {
		w_flags = w_flags | SDL_WINDOW_FULLSCREEN_DESKTOP;

		// make the window the same size as the desktop resolution
		SDL_DisplayMode desktop;
		if (SDL_GetDesktopDisplayMode(0, &desktop) == 0) {
			window_w = desktop.w;
			window_h = desktop.h;
		}
	}
	else if (fullscreen && is_initialized) {
		// if the game was previously in fullscreen, resize the window when returning to windowed mode
		window_w = MIN_SCREEN_W;
		window_h = MIN_SCREEN_H;
		w_flags = w_flags | SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;
	}
	else {
		w_flags = w_flags | SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;
	}

	w_flags = w_flags | SDL_WINDOW_RESIZABLE;

	if (HWSURFACE) {
		r_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE;
	}
	else {
		r_flags = SDL_RENDERER_SOFTWARE | SDL_RENDERER_TARGETTEXTURE;
		VSYNC = false; // can't have software mode & vsync at the same time
	}
	if (VSYNC) r_flags = r_flags | SDL_RENDERER_PRESENTVSYNC;

	if (settings_changed || !is_initialized) {
		if (is_initialized) {
			destroyContext();
		}

		window = SDL_CreateWindow(NULL, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_w, window_h, w_flags);
		if (window) {
			renderer = SDL_GL_CreateContext(window);
			if (renderer) {
				if (TEXTURE_FILTER && !IGNORE_TEXTURE_FILTER)
					SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
				else
					SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

				windowResize();
			}

			SDL_SetWindowMinimumSize(window, MIN_SCREEN_W, MIN_SCREEN_H);
			// setting minimum size might move the window, so set position again
			SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
		}

		if (!is_initialized)
		{
			init(&renderer);
			buildResources();
		}

		bool window_created = window != NULL && renderer != NULL;

		if (!window_created && !is_initialized) {
			// If this is the first attempt and it failed we are not
			// getting anywhere.
			logError("SDLOpenGLRenderDevice: createContext() failed: %s", SDL_GetError());
			SDL_Quit();
			exit(1);
		}
		else if (!window_created) {
			// try previous setting first
			FULLSCREEN = fullscreen;
			HWSURFACE = hwsurface;
			VSYNC = vsync;
			TEXTURE_FILTER = texture_filter;
			if (createContext() == -1) {
				// last resort, try turning everything off
				FULLSCREEN = false;
				HWSURFACE = false;
				VSYNC = false;
				TEXTURE_FILTER = false;
				return createContext();
			}
			else {
				return 0;
			}
		}
		else {
			fullscreen = FULLSCREEN;
			hwsurface = HWSURFACE;
			vsync = VSYNC;
			texture_filter = TEXTURE_FILTER;
			is_initialized = true;
		}
	}

	if (is_initialized) {
		// update minimum window size if it has changed
		if (min_screen.x != MIN_SCREEN_W || min_screen.y != MIN_SCREEN_H) {
			min_screen.x = MIN_SCREEN_W;
			min_screen.y = MIN_SCREEN_H;
			SDL_SetWindowMinimumSize(window, MIN_SCREEN_W, MIN_SCREEN_H);
			SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
		}

		windowResize();

		// update title bar text and icon
		updateTitleBar();

		// load persistent resources
		SharedResources::loadIcons();
		curs = new CursorManager();
	}

	return (is_initialized ? 0 : -1);
}

Rect OpenGLRenderDevice::getContextSize() {
	Rect size;
	size.x = size.y = 0;
	SDL_GetWindowSize(window, &size.w, &size.h);

	return size;
}

int OpenGLRenderDevice::render(Renderable& r, Rect dest) {
	SDL_Rect src = r.src;
	SDL_Rect _dest = dest;

	m_offset[0] = 2.0f * (float)_dest.x/VIEW_W;
	m_offset[1] = 2.0f * (float)_dest.y/VIEW_H;

	m_offset[2] = (float)src.w/VIEW_W;
	m_offset[3] = (float)src.h/VIEW_H;

	int height = static_cast<OpenGLImage *>(r.image)->getHeight();
	int width = static_cast<OpenGLImage *>(r.image)->getWidth();

	m_texelOffset[0] = (float)width / src.w;
	m_texelOffset[1] = (float)src.x / width;

	m_texelOffset[2] = (float)height / src.h;
	m_texelOffset[3] = (float)src.y / height;
 
    GLuint texture = static_cast<OpenGLImage *>(r.image)->texture;
	GLuint normalTexture = static_cast<OpenGLImage *>(r.image)->normalTexture;

    if (texture == 0)
        return 1;

	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

	bool normals = ((int)normalTexture != -1);
	if (normals)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normalTexture);
	}

	composeFrame(m_offset, m_texelOffset, normals);

	return 0;
}

void* OpenGLRenderDevice::openShaderFile(const char *filename, GLint *length)
{
    FILE *f = fopen(filename, "r");
    void *buffer;

    if (!f) {
        logError("Unable to open shader file %s for reading", filename);
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
        logError("Failed to compile %s:", filename);
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
        logError("Failed to link shader program:");
        glDeleteProgram(program);
        return 1;
    }
    return program;
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
	vertex_buffer = createBuffer(GL_ARRAY_BUFFER, positionData, sizeof(positionData));
    element_buffer = createBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferData, sizeof(elementBufferData));

    m_vertex_shader = getShader(GL_VERTEX_SHADER, "shaders/vertex.glsl");
    if (m_vertex_shader == 0)
        return 1;

    m_fragment_shader = getShader(GL_FRAGMENT_SHADER, "shaders/fragment.glsl");
    if (m_fragment_shader == 0)
        return 1;

	m_program = createProgram(m_vertex_shader, m_fragment_shader);
    if (m_program == 0)
        return 1;

    attributes.position = glGetAttribLocation(m_program, "position");

    uniforms.texture = glGetUniformLocation(m_program, "texture");
	uniforms.offset = glGetUniformLocation(m_program, "offset");
	uniforms.texelOffset = glGetUniformLocation(m_program, "texelOffset");

    uniforms.normals = glGetUniformLocation(m_program, "normals");
    uniforms.light = glGetUniformLocation(m_program, "lightEnabled");

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

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

	m_offset[0] = 2.0f * (float)dest.x/VIEW_W;
	m_offset[1] = 2.0f * (float)dest.y/VIEW_H;

	m_offset[2] = (float)src.w/VIEW_W;
	m_offset[3] = (float)src.h/VIEW_H;

	int height = r->getGraphics()->getHeight();
	int width = r->getGraphics()->getWidth();

	m_texelOffset[0] = (float)width / src.w;
	m_texelOffset[1] = (float)src.x / width;

	m_texelOffset[2] = (float)height / src.h;
	m_texelOffset[3] = (float)src.y / height;

    GLuint texture = static_cast<OpenGLImage *>(r->getGraphics())->texture;
    GLuint normalTexture = static_cast<OpenGLImage *>(r->getGraphics())->normalTexture;

    if (texture == 0)
        return 1;

	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

	bool normals = ((int)normalTexture != -1);
	if (normals)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normalTexture);
	}

	composeFrame(m_offset, m_texelOffset , normals);

	return 0;
}

void OpenGLRenderDevice::composeFrame(GLfloat* offset, GLfloat* texelOffset, bool withLight)
{
	glUseProgram(m_program);

    glUniform1i(uniforms.texture, 0);

	if (withLight)
	{
		glUniform1i(uniforms.light, 1);
		glUniform1i(uniforms.normals, 1);
	}
	else
	{
		glUniform1i(uniforms.light, 0);
	}


	glUniform4fv(uniforms.offset, 1, offset);
	glUniform4fv(uniforms.texelOffset, 1, texelOffset);

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

	SDL_Surface* src_surface = copyTextureToSurface(static_cast<OpenGLImage *>(src_image)->texture);

	SDL_Surface* dst_surface = copyTextureToSurface(static_cast<OpenGLImage *>(dest_image)->texture);

	if (dest_is_transparent)
		SDL_gfxBlitRGBA(src_surface, &_src, dst_surface, &_dest);
	else
		SDL_BlitSurface(src_surface, &_src, dst_surface, &_dest);

	SDL_FreeSurface(src_surface);

	glBindTexture(GL_TEXTURE_2D, static_cast<OpenGLImage *>(dest_image)->texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dst_surface->w, dst_surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, dst_surface->pixels);
	SDL_FreeSurface(dst_surface);

	return 0;
}

SDL_Surface* OpenGLRenderDevice::copyTextureToSurface(GLuint texture)
{
	// FIXME: there is a heap corruption somewhere here when viewing Credits screen
	Uint32 rmask, gmask, bmask, amask;
	setSDL_RGBA(&rmask, &gmask, &bmask, &amask);

	GLint width, height, format;
	GLint bytes = 0;
	int pitch = 0;

	glBindTexture(GL_TEXTURE_2D, texture);

	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPONENTS, &format);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

	bytes = width * height * format;
	pitch = width * BITS_PER_PIXEL / 8;

	unsigned char *pixels = new unsigned char[bytes];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	SDL_Surface* cleanup;

	cleanup = SDL_CreateRGBSurfaceFrom(pixels, width, height, BITS_PER_PIXEL, pitch, rmask, gmask, bmask, amask);
	SDL_Surface* surface = SDL_ConvertSurfaceFormat(cleanup, SDL_PIXELFORMAT_ABGR8888, 0);
	SDL_FreeSurface(cleanup);

	delete [] pixels;

	return surface;
}

int OpenGLRenderDevice::renderText(
	TTF_Font *ttf_font,
	const std::string& text,
	Color color,
	Rect& dest
) {
	// UNIMPLEMENTED
	int ret = 0;

	return ret;
}

Image * OpenGLRenderDevice::renderTextToImage(TTF_Font* ttf_font, const std::string& text, Color color, bool blended) {
	OpenGLImage *image = new OpenGLImage(this);
	if (!image) return NULL;

	SDL_Color _color = color;

	SDL_Surface* surface;
	if (blended)
		surface = TTF_RenderUTF8_Blended(ttf_font, text.c_str(), _color);
	else
		surface = TTF_RenderUTF8_Solid(ttf_font, text.c_str(), _color);

	if (surface)
	{
		Uint32 rmask, gmask, bmask, amask;
		setSDL_RGBA(&rmask, &gmask, &bmask, &amask);

		glGenTextures(1, &(image->texture));

		glBindTexture(GL_TEXTURE_2D, image->texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
		SDL_FreeSurface(surface);
	}

	if ((int)image->texture != -1)
		return image;

	delete image;
	return NULL;
}

void OpenGLRenderDevice::drawPixel(
	int x,
	int y,
	Uint32 color
) {
	// UNIMPLEMENTED
}

void OpenGLRenderDevice::drawLine(
	int x0,
	int y0,
	int x1,
	int y1,
	Uint32 color
) {
	// UNIMPLEMENTED
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
	glFlush();
	SDL_GL_SwapWindow(window);

	return;
}

void OpenGLRenderDevice::destroyContext() {
	SDL_FreeSurface(titlebar_icon);
	titlebar_icon = NULL;

	SDL_GL_DeleteContext(renderer);
	renderer = NULL;

	SDL_DestroyWindow(window);
	window = NULL;

	if (title) {
		free(title);
		title = NULL;
	}

	return;
}

Uint32 OpenGLRenderDevice::MapRGB(Uint8 r, Uint8 g, Uint8 b) {
	Uint32 u_format = SDL_GetWindowPixelFormat(window);
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
	Uint32 u_format = SDL_GetWindowPixelFormat(window);
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

	int channels = 4;
	char* buffer = (char*)calloc(width * height * channels, sizeof(char));

	glGenTextures(1, &(image->texture));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, image->texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, channels, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

	free(buffer);

	return image;
}

void OpenGLRenderDevice::setGamma(float g) {
	Uint16 ramp[256];
	SDL_CalculateGammaRamp(g, ramp);
	SDL_SetWindowGammaRamp(window, ramp, ramp, ramp);
}

void OpenGLRenderDevice::updateTitleBar() {
	if (title) free(title);
	title = NULL;
	if (titlebar_icon) SDL_FreeSurface(titlebar_icon);
	titlebar_icon = NULL;

	if (!window) return;

	title = strdup(msg->get(WINDOW_TITLE).c_str());
	titlebar_icon = IMG_Load(mods->locate("images/logo/icon.png").c_str());

	if (title) SDL_SetWindowTitle(window, title);
	if (titlebar_icon) SDL_SetWindowIcon(window, titlebar_icon);
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
			logError("OpenGLRenderDevice: %s: %s", errormessage.c_str(), IMG_GetError());
		if (IfNotFoundExit) {
			SDL_Quit();
			exit(1);
		}
	}
	else {
		image = new OpenGLImage(this);
		SDL_Surface *surface = SDL_ConvertSurfaceFormat(cleanup, SDL_PIXELFORMAT_ABGR8888, 0);

		glGenTextures(1, &(image->texture));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, image->texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, 4, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);

		SDL_FreeSurface(surface);
		SDL_FreeSurface(cleanup);
	}

	std::string normalFileName = filename.substr(0, filename.size() - 4) + "_N.png";

	SDL_Surface *cleanupN = IMG_Load(mods->locate(normalFileName).c_str());
	if(cleanupN) {
		SDL_Surface *surfaceN = SDL_ConvertSurfaceFormat(cleanupN, SDL_PIXELFORMAT_ABGR8888, 0);

		glGenTextures(1, &(image->normalTexture));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, image->normalTexture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, 4, surfaceN->w, surfaceN->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surfaceN->pixels);

		SDL_FreeSurface(surfaceN);
		SDL_FreeSurface(cleanupN);
	}
	// store image to cache
	cacheStore(filename, image);
	return image;
}

void OpenGLRenderDevice::freeImage(Image *image) {
	if (!image) return;

	cacheRemove(image);

	if ((int)static_cast<OpenGLImage *>(image)->texture != -1)
		glDeleteTextures(1, &(static_cast<OpenGLImage *>(image)->texture));

	if ((int)static_cast<OpenGLImage *>(image)->normalTexture != -1)
		glDeleteTextures(1, &(static_cast<OpenGLImage *>(image)->normalTexture));
}

void OpenGLRenderDevice::windowResize() {
	int w,h;
	SDL_GetWindowSize(window, &w, &h);
	SCREEN_W = w;
	SCREEN_H = h;

	float scale = (float)VIEW_H / (float)SCREEN_H;
	VIEW_W = (int)((float)SCREEN_W * scale);

	// letterbox if too tall
	if (VIEW_W < MIN_SCREEN_W) {
		VIEW_W = MIN_SCREEN_W;
	}

	VIEW_W_HALF = VIEW_W/2;

	// FIXME: fix vertical scaling
	glViewport(0, 0, SCREEN_W, VIEW_H);
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
