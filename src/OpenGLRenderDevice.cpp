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

		glExt::init(&renderer);

		//glEnable(GL_CULL_FACE);

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

    shader = glExt::glCreateShader(type);
    glExt::glShaderSource(shader, 1, (const GLchar**)&source, &length);
    free(source);
    glExt::glCompileShader(shader);

    glExt::glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_ok);
    if (!shader_ok)
	{
        logError("Failed to compile %s:\n", filename);
        glExt::glDeleteShader(shader);
        return 1;
    }
    return shader;
}

GLuint OpenGLRenderDevice::createProgram(GLuint vertex_shader, GLuint fragment_shader)
{
    GLint program_ok;

    GLuint program = glExt::glCreateProgram();

    glExt::glAttachShader(program, vertex_shader);
	glExt::glAttachShader(program, fragment_shader);
    glExt::glLinkProgram(program);

    glExt::glGetProgramiv(program, GL_LINK_STATUS, &program_ok);
    if (!program_ok)
	{
        logError("Failed to link shader program:\n");
        glExt::glDeleteProgram(program);
        return 1;
    }
    return program;
}

GLuint OpenGLRenderDevice::createBuffer(GLenum target, const void *buffer_data, GLsizei buffer_size)
{
    GLuint buffer;
    glExt::glGenBuffers(1, &buffer);
    glExt::glBindBuffer(target, buffer);
    glExt::glBufferData(target, buffer_size, buffer_data, GL_STATIC_DRAW);
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

    attributes.position = glExt::glGetAttribLocation(m_program, "position");

    uniforms.texture = glExt::glGetUniformLocation(m_program, "texture");
	uniforms.offset = glExt::glGetUniformLocation(m_program, "offset");
	uniforms.texelOffset = glExt::glGetUniformLocation(m_program, "texelOffset");

    uniforms.normals = glExt::glGetUniformLocation(m_program, "normals");
    uniforms.light = glExt::glGetUniformLocation(m_program, "lightEnabled");

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
	glExt::glUseProgram(m_program);

    glExt::glUniform1i(uniforms.texture, 0);

	if (withLight)
	{
		glExt::glUniform1i(uniforms.light, 1);
		glExt::glUniform1i(uniforms.normals, 1);
	}
	else
	{
		glExt::glUniform1i(uniforms.light, 0);
	}


	glExt::glUniform4fv(uniforms.offset, 1, offset);
	glExt::glUniform4fv(uniforms.texelOffset, 1, texelOffset);

    glExt::glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glExt::glVertexAttribPointer(
        attributes.position,
        2, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat)*2, (void*)0
    );

    glExt::glEnableVertexAttribArray(attributes.position);

	glExt::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
    glDrawElements(
        GL_TRIANGLE_STRIP,  /* mode */
        4,                  /* count */
        GL_UNSIGNED_SHORT,  /* type */
        (void*)0            /* element array buffer offset */
    );

    glExt::glDisableVertexAttribArray(attributes.position);
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
