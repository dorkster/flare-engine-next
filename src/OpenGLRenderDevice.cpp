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

//#define FRAMEBUFFER

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
	, textureNumber(-1) {
}

OpenGLImage::~OpenGLImage() {
}

int OpenGLImage::getWidth() const {
	int width = 0;
	if (texture == -1) return width;

	glBindTexture(GL_TEXTURE_2D, texture);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
	return width;
}

int OpenGLImage::getHeight() const {
	int height = 0;
	if (texture == -1) return height;

	glBindTexture(GL_TEXTURE_2D, texture);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
	return height;
}

void OpenGLImage::fillWithColor(Uint32 color) {
	if (texture == -1) return;

	auto channels = 4;
	// TODO fill with color
	char* buffer = (char*)calloc(VIEW_W * VIEW_H * channels, sizeof(char));

	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, channels, VIEW_W, VIEW_H, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
 
	free(buffer);
}

/*
 * Set the pixel at (x, y) to the given value
 */
void OpenGLImage::drawPixel(int x, int y, Uint32 pixel) {
	if (texture == -1) return;
}

Uint32 OpenGLImage::MapRGB(Uint8 r, Uint8 g, Uint8 b) {
	if (texture == -1) return 0;
	return 0;
}

Uint32 OpenGLImage::MapRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	if (texture == -1) return 0;
	return 0;
}

/**
 * Resizes an image
 * Deletes the original image and returns a pointer to the resized version
 */
Image* OpenGLImage::resize(int width, int height) {
	if(texture == -1 || width <= 0 || height <= 0)
		return NULL;

	return NULL;
}

Uint32 OpenGLImage::readPixel(int x, int y) {
	if (texture == -1) return 0;

	return 0;
}

OpenGLRenderDevice::OpenGLRenderDevice()
	: screen(NULL)
	, renderer(NULL)
	, titlebar_icon(NULL)
	, textureCount(0) {
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

	offset[0] = 2.0f * (float)_dest.x/VIEW_W;
	offset[1] = 2.0f * (float)_dest.y/VIEW_H;

	offset[2] = (float)src.w/VIEW_W;
	offset[3] = (float)src.h/VIEW_H;

	auto height = static_cast<OpenGLImage *>(r.image)->getHeight();
	auto width = static_cast<OpenGLImage *>(r.image)->getWidth();

	texelOffset[0] = (float)width / src.w;
	texelOffset[1] = (float)src.x / width;

	texelOffset[2] = (float)height / src.h;
	texelOffset[3] = (float)src.y / height;

#ifdef FRAMEBUFFER
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, destTexture, 0);
#endif
 
    GLuint texture = static_cast<OpenGLImage *>(r.image)->texture;
	int textureNumber = static_cast<OpenGLImage *>(r.image)->textureNumber;

    if (texture == 0)
        return 1;

#ifdef FRAMEBUFFER
    glActiveTexture(GL_TEXTURE1);
#else
	glActiveTexture(GL_TEXTURE0 + textureNumber);
#endif
    glBindTexture(GL_TEXTURE_2D, texture);

	composeFrame(offset, texelOffset, textureNumber);

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

	return 0;
}

void OpenGLRenderDevice::clearBufferTexture()
{
	auto channels = 4;
	char* buffer = (char*)calloc(VIEW_W * VIEW_H * channels, sizeof(char));

	glGenTextures(1, &destTexture);
	glBindTexture(GL_TEXTURE_2D, destTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, channels, VIEW_W, VIEW_H, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

	free(buffer);
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
	uniforms.offset = glGetUniformLocation(program, "offset");
	uniforms.texelOffset = glGetUniformLocation(program, "texelOffset");

#ifdef FRAMEBUFFER
    destTexture = glGetUniformLocation(program, "destTexture");

	createFrameBuffer();
#else
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
#endif

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

	offset[0] = 2.0f * (float)dest.x/VIEW_W;
	offset[1] = 2.0f * (float)dest.y/VIEW_H;

	offset[2] = (float)src.w/VIEW_W;
	offset[3] = (float)src.h/VIEW_H;

	auto height = r->getGraphics()->getHeight();
	auto width = r->getGraphics()->getWidth();

	texelOffset[0] = (float)width / src.w;
	texelOffset[1] = (float)src.x / width;

	texelOffset[2] = (float)height / src.h;
	texelOffset[3] = (float)src.y / height;

#ifdef FRAMEBUFFER
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, destTexture, 0);
#endif
    GLuint texture = static_cast<OpenGLImage *>(r->getGraphics())->texture;
    int textureNumber = static_cast<OpenGLImage *>(r->getGraphics())->textureNumber;

    if (texture == 0)
        return 1;

#ifdef FRAMEBUFFER
    glActiveTexture(GL_TEXTURE1);
#else
	glActiveTexture(GL_TEXTURE0 + textureNumber);
#endif
    glBindTexture(GL_TEXTURE_2D, texture);

	composeFrame(offset, texelOffset, textureNumber);

	return 0;
}

void OpenGLRenderDevice::composeFrame(GLfloat* offset, GLfloat* texelOffset, int textureNumber)
{
	glUseProgram(program);

	// FIXME: incorrect textureNumber
    //glUniform1i(uniforms.texture, textureNumber);
    glUniform1i(uniforms.texture, 1);

#ifdef FRAMEBUFFER
    glUniform1i(destTexture, 0);
#endif

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
	if (!image) return NULL;

	SDL_Color _color = color;

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
#ifdef FRAMEBUFFER
	glDeleteTextures(1, &destTexture);
	clearBufferTexture();
#endif
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT);

	return;
}

void OpenGLRenderDevice::commitFrame() {
#ifdef FRAMEBUFFER
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, destTexture);
	GLfloat offset[4] = {0};
	composeFrame(offset);
#endif
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

	auto channels = 4;
	char* buffer = (char*)calloc(width * height * channels, sizeof(char));

	glGenTextures(1, &(image->texture));
	glBindTexture(GL_TEXTURE_2D, image->texture);
	image->textureNumber = textureCount;
	textureCount += 1;

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
		glBindTexture(GL_TEXTURE_2D, image->texture);
		image->textureNumber = textureCount;
		textureCount += 1;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, 4, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);

		SDL_FreeSurface(surface);
		SDL_FreeSurface(cleanup);
	}

	// store image to cache
	cacheStore(filename, image);
	return image;
}

void OpenGLRenderDevice::freeImage(Image *image) {
	if (!image) return;

	cacheRemove(image);

	if (static_cast<OpenGLImage *>(image)->texture != -1)
	{
		glDeleteTextures(1, &(static_cast<OpenGLImage *>(image)->texture));
		textureCount -= 1;
		static_cast<OpenGLImage *>(image)->textureNumber = -1;
	}
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
