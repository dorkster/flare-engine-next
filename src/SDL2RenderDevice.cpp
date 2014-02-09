/*
Copyright © 2013 Igor Paliychuk

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

#include "SDL2RenderDevice.h"

using namespace std;

Sprite::Sprite(const Sprite& other) {
	local_frame = other.local_frame;
	sprite = other.sprite;
	src = other.src;
	offset = other.offset;
	dest = other.dest;
}

Sprite& Sprite::operator=(const Sprite& other) {
	local_frame = other.local_frame;
	sprite = other.sprite;
	src = other.src;
	offset = other.offset;
	dest = other.dest;

	return *this;
}

Sprite::~Sprite() {
}

/**
 * Set the graphics context of a Sprite.
 * Initialize graphics resources. That is the SLD_surface buffer
 *
 * It is important that, if the client owns the graphics resources,
 * clearGraphics() method is called first in case this Sprite holds the
 * last references to avoid resource leaks.
 */
void Sprite::setGraphics(Image s, bool setClipToFull) {

	sprite = s;

	if (setClipToFull && sprite.surface != NULL) {
		src.x = 0;
		src.y = 0;
		src.w = sprite.getWidth();
		src.h = sprite.getHeight();
	}

}

Image* Sprite::getGraphics() {

	return &sprite;
}

bool Sprite::graphicsIsNull() {

	return (sprite.surface == NULL);
}

/**
 * Clear the graphics context of a Sprite.
 * Release graphics resources. That is the SLD_surface buffer
 *
 * It is important that this method is only called by clients who own the
 * graphics resources.
 */
void Sprite::clearGraphics() {
	if (sprite.surface != NULL) {
		SDL_DestroyTexture(sprite.surface);
		textures_count-=1;
		sprite.surface = NULL;
	}
}

void Sprite::setOffset(const Point& _offset) {
	this->offset = _offset;
}

void Sprite::setOffset(const int x, const int y) {
	this->offset.x = x;
	this->offset.y = y;
}

Point Sprite::getOffset() {

	return offset;
}
/**
 * Set the clipping rectangle for the sprite
 */
void Sprite::setClip(const Rect& clip) {
	src = clip;
}

/**
 * Set the clipping rectangle for the sprite
 */
void Sprite::setClip(const int x, const int y, const int w, const int h) {
	src.x = x;
	src.y = y;
	src.w = w;
	src.h = h;
}

void Sprite::setClipX(const int x) {
	src.x = x;
}

void Sprite::setClipY(const int y) {
	src.y = y;
}

void Sprite::setClipW(const int w) {
	src.w = w;
}

void Sprite::setClipH(const int h) {
	src.h = h;
}


Rect Sprite::getClip() {
	return src;
}
void Sprite::setDest(const Rect& _dest) {
	dest.x = (float)_dest.x;
	dest.y = (float)_dest.y;
}

void Sprite::setDest(const Point& _dest) {
	dest.x = (float)_dest.x;
	dest.y = (float)_dest.y;
}

void Sprite::setDest(int x, int y) {
	dest.x = (float)x;
	dest.y = (float)y;
}

void Sprite::setDestX(int x) {
	dest.x = (float)x;
}

void Sprite::setDestY(int y) {
	dest.y = (float)y;
}

FPoint Sprite::getDest() {
	return dest;
}

int Sprite::getGraphicsWidth() {
	return (sprite.surface ? sprite.getWidth() : 0);
}

int Sprite::getGraphicsHeight() {
	return (sprite.surface ? sprite.getHeight() : 0);
}

SDL2RenderDevice::SDL2RenderDevice()
	: screen(NULL)
	, titlebar_icon(NULL) {
	cout << "Using Render Device: SDL2RenderDevice" << endl;
	textures_count = 0;
}

int SDL2RenderDevice::createContext(int width, int height) {
	int set_fullscreen = 0;

	if (is_initialized) {
		if (textures_count != 0) {
			cout << "Trying to change video mode. Number of not freed textures:" << textures_count << endl;
			cout << "This can cause graphical issues or even game exit" << endl;
		}
		SDL_DestroyRenderer(renderer);
		Uint32 flags = 0;

		if (FULLSCREEN) flags = SDL_WINDOW_FULLSCREEN;
		else flags = SDL_WINDOW_SHOWN;

		SDL_DestroyWindow(screen);
		screen = SDL_CreateWindow(msg->get(WINDOW_TITLE).c_str(),
									SDL_WINDOWPOS_CENTERED,
									SDL_WINDOWPOS_CENTERED,
									width, height,
									flags);

		if (HWSURFACE) flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE;
		else flags = SDL_RENDERER_SOFTWARE | SDL_RENDERER_TARGETTEXTURE;

		if (DOUBLEBUF) flags = flags | SDL_RENDERER_PRESENTVSYNC;

		renderer = SDL_CreateRenderer(screen, -1, flags);

	}
	else {
		Uint32 flags = 0;

		if (FULLSCREEN) flags = SDL_WINDOW_FULLSCREEN;
		else flags = SDL_WINDOW_SHOWN;

		screen = SDL_CreateWindow(msg->get(WINDOW_TITLE).c_str(),
									SDL_WINDOWPOS_CENTERED,
									SDL_WINDOWPOS_CENTERED,
									width, height,
									flags);

		if (HWSURFACE) flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE;
		else flags = SDL_RENDERER_SOFTWARE | SDL_RENDERER_TARGETTEXTURE;

		if (DOUBLEBUF) flags = flags | SDL_RENDERER_PRESENTVSYNC;

		if (screen != NULL) renderer = SDL_CreateRenderer(screen, -1, flags);
	}

	if (screen != NULL && renderer != NULL) {
		is_initialized = true;

		// Add Window Titlebar Icon
		if (titlebar_icon == NULL) {
			titlebar_icon = IMG_Load(mods->locate("images/logo/icon.png").c_str());
			SDL_SetWindowIcon(screen, titlebar_icon);
		}

		return (set_fullscreen == 0 ? 0 : -1);
	}
	else {
		fprintf(stderr, "createContext() failed: %s\n", SDL_GetError());
		SDL_Quit();
		exit(1);
	}
}

Rect SDL2RenderDevice::getContextSize() {
	Rect size;
	size.x = size.y = 0;
	SDL_GetWindowSize(screen, &size.w, &size.h);

	return size;
}

int SDL2RenderDevice::render(Renderable& r, Rect dest) {
	dest.w = r.src.w;
	dest.h = r.src.h;
    SDL_Rect src = r.src;
    SDL_Rect _dest = dest;
	return SDL_RenderCopy(renderer, r.sprite.surface, &src, &_dest);
}

int SDL2RenderDevice::render(ISprite& r) {
	if (r.graphicsIsNull()) {
		return -1;
	}
	if ( !local_to_global(r) ) {
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
	return SDL_RenderCopy(renderer, r.getGraphics()->surface, &src, &dest);
}

int SDL2RenderDevice::renderImage(Image* image, Rect& src) {
	if (!image) return -1;

    SDL_Rect _src = src;
	SDL_Rect dest;
	dest.x = 0;
	dest.y = 0;

	// negative x and y clip causes weird stretching
	// adjust for that here
	if (src.x < 0) {
		src.w -= abs(src.x);
		dest.x += abs(src.x);
		src.x = 0;
	}
	if (src.y < 0) {
		src.h -= abs(src.y);
		dest.y += abs(src.y);
		src.y = 0;
	}

	dest.w = src.w;
	dest.h = src.h;

	return SDL_RenderCopy(renderer, image->surface, &_src, &dest);
}

int SDL2RenderDevice::renderToImage(Image* src_image, Rect& src, Image* dest_image, Rect& dest, bool dest_is_transparent) {
	if (!src_image || !dest_image) return -1;
	if (SDL_SetRenderTarget(renderer, dest_image->surface) != 0) return -1;
	SDL_SetTextureBlendMode(dest_image->surface, SDL_BLENDMODE_BLEND);
	dest.w = src.w;
	dest.h = src.h;
    SDL_Rect _src = src;
    SDL_Rect _dest = dest;
	SDL_RenderCopy(renderer, src_image->surface, &_src, &_dest);
	SDL_SetRenderTarget(renderer, NULL);
	return 0;
}

int SDL2RenderDevice::renderText(
	TTF_Font *ttf_font,
	const std::string& text,
	Color color,
	Rect& dest
) {
	int ret = 0;
	Image ttf;
	SDL_Surface *cleanup = TTF_RenderUTF8_Blended(ttf_font, text.c_str(), color);
	if (cleanup) {
		ttf.surface = SDL_CreateTextureFromSurface(renderer,cleanup);
		textures_count+=1;
		SDL_FreeSurface(cleanup);
	}
	else {
		return -1;
	}
	m_ttf_renderable.setGraphics(ttf);
	if (!m_ttf_renderable.graphicsIsNull()) {
		SDL_Rect clip = m_ttf_renderable.getClip();
		dest.w = clip.w;
		dest.h = clip.h;
        SDL_Rect _dest = dest;
		ret = SDL_RenderCopy(
				  renderer,
				  m_ttf_renderable.getGraphics()->surface,
				  &clip,
				  &_dest
			  );
		SDL_DestroyTexture(m_ttf_renderable.getGraphics()->surface);
		textures_count-=1;
		ttf.surface = NULL;
		m_ttf_renderable.setGraphics(ttf);
	}
	else {
		ret = -1;
	}

	return ret;
}

void SDL2RenderDevice::renderTextToImage(Image* image, TTF_Font* ttf_font, const std::string& text, Color color, bool blended) {
	if (!image) return;

	SDL_Surface *cleanup;

	if (blended) {
		cleanup = TTF_RenderUTF8_Blended(ttf_font, text.c_str(), color);
	}
	else {
		cleanup = TTF_RenderUTF8_Solid(ttf_font, text.c_str(), color);
	}

	if (cleanup) {
		image->surface = SDL_CreateTextureFromSurface(renderer, cleanup);
		textures_count+=1;
		SDL_FreeSurface(cleanup);
	}
}

void SDL2RenderDevice::drawPixel(
	int x,
	int y,
	Uint32 color
) {
	Uint32 u_format = SDL_GetWindowPixelFormat(screen);
	SDL_PixelFormat* format = SDL_AllocFormat(u_format);

	if (!format) return;

	SDL_Color rgba;
	SDL_GetRGBA(color, format, &rgba.r, &rgba.g, &rgba.b, &rgba.a);
	SDL_FreeFormat(format);

	SDL_SetRenderDrawColor(renderer, rgba.r, rgba.g, rgba.b, rgba.a);
	SDL_RenderDrawPoint(renderer, x, y);
}

/*
 * Set the pixel at (x, y) to the given value
 */
void SDL2RenderDevice::drawPixel(Image *image, int x, int y, Uint32 pixel) {
	if (!image || !image->surface) return;

	Uint32 u_format;
	SDL_QueryTexture(image->surface, &u_format, NULL, NULL, NULL);
	SDL_PixelFormat* format = SDL_AllocFormat(u_format);

	if (!format) return;

	SDL_Color rgba;
	SDL_GetRGBA(pixel, format, &rgba.r, &rgba.g, &rgba.b, &rgba.a);
	SDL_FreeFormat(format);

	SDL_SetRenderTarget(renderer, image->surface);
	SDL_SetTextureBlendMode(image->surface, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, rgba.r, rgba.g, rgba.b, rgba.a);
	SDL_RenderDrawPoint(renderer, x, y);
	SDL_SetRenderTarget(renderer, NULL);
}

void SDL2RenderDevice::drawLine(
	int x0,
	int y0,
	int x1,
	int y1,
	Uint32 color
) {
	Uint32 u_format = SDL_GetWindowPixelFormat(screen);
	SDL_PixelFormat* format = SDL_AllocFormat(u_format);

	if (!format) return;

	SDL_Color rgba;
	SDL_GetRGBA(color, format, &rgba.r, &rgba.g, &rgba.b, &rgba.a);
	SDL_FreeFormat(format);

	SDL_SetRenderDrawColor(renderer, rgba.r, rgba.g, rgba.b, rgba.a);
	SDL_RenderDrawLine(renderer, x0, y0, x1, y1);
}

void SDL2RenderDevice::drawLine(
	const Point& p0,
	const Point& p1,
	Uint32 color
) {
	drawLine(p0.x, p0.y, p1.x, p1.y, color);
}

/**
 * draw line to the screen
 */
void SDL2RenderDevice::drawLine(Image *image, int x0, int y0, int x1, int y1, Uint32 color) {
	if (!image || !image->surface) return;

	Uint32 u_format;
	SDL_QueryTexture(image->surface, &u_format, NULL, NULL, NULL);
	SDL_PixelFormat* format = SDL_AllocFormat(u_format);

	if (!format) return;

	SDL_Color rgba;
	SDL_GetRGBA(color, format, &rgba.r, &rgba.g, &rgba.b, &rgba.a);
	SDL_FreeFormat(format);

	SDL_SetRenderTarget(renderer, image->surface);
	SDL_SetTextureBlendMode(image->surface, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, rgba.r, rgba.g, rgba.b, rgba.a);
	SDL_RenderDrawLine(renderer, x0, y0, x1, y1);
	SDL_SetRenderTarget(renderer, NULL);
}

void SDL2RenderDevice::drawLine(Image *image, Point pos0, Point pos1, Uint32 color) {
	drawLine(image, pos0.x, pos0.y, pos1.x, pos1.y, color);
}

void SDL2RenderDevice::drawRectangle(Image *image, Point pos0, Point pos1, Uint32 color) {
	drawLine(image, pos0.x, pos0.y, pos1.x, pos0.y, color);
	drawLine(image, pos1.x, pos0.y, pos1.x, pos1.y, color);
	drawLine(image, pos0.x, pos0.y, pos0.x, pos1.y, color);
	drawLine(image, pos0.x, pos1.y, pos1.x, pos1.y, color);
}

void SDL2RenderDevice::drawRectangle(
	const Point& p0,
	const Point& p1,
	Uint32 color
) {
	drawLine(p0.x, p0.y, p1.x, p0.y, color);
	drawLine(p1.x, p0.y, p1.x, p1.y, color);
	drawLine(p0.x, p0.y, p0.x, p1.y, color);
	drawLine(p0.x, p1.y, p1.x, p1.y, color);
}

void SDL2RenderDevice::blankScreen() {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);
	return;
}

void SDL2RenderDevice::commitFrame() {
	SDL_RenderPresent(renderer);
	return;
}

void SDL2RenderDevice::destroyContext() {
	m_ttf_renderable.clearGraphics();
	SDL_FreeSurface(titlebar_icon);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(screen);

	return;
}

void SDL2RenderDevice::fillImageWithColor(Image *dst, Rect *dstrect, Uint32 color) {
	if (!dst) return;

	Uint32 u_format;
	SDL_QueryTexture(dst->surface, &u_format, NULL, NULL, NULL);
	SDL_PixelFormat* format = SDL_AllocFormat(u_format);

	if (!format) return;

	SDL_Color rgba;
	SDL_GetRGBA(color, format, &rgba.r, &rgba.g, &rgba.b, &rgba.a);
	SDL_FreeFormat(format);

	SDL_SetRenderTarget(renderer, dst->surface);
	SDL_SetTextureBlendMode(dst->surface, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, rgba.r, rgba.g , rgba.b, rgba.a);
	SDL_RenderClear(renderer);
	SDL_SetRenderTarget(renderer, NULL);
}

Uint32 SDL2RenderDevice::MapRGB(Image *src, Uint8 r, Uint8 g, Uint8 b) {
	if (!src || !src->surface) return 0;

	Uint32 u_format;
	SDL_QueryTexture(src->surface, &u_format, NULL, NULL, NULL);
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

Uint32 SDL2RenderDevice::MapRGB(Uint8 r, Uint8 g, Uint8 b) {
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

Uint32 SDL2RenderDevice::MapRGBA(Image *src, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	if (!src || !src->surface) return 0;

	Uint32 u_format;
	SDL_QueryTexture(src->surface, &u_format, NULL, NULL, NULL);
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

Uint32 SDL2RenderDevice::MapRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
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

bool SDL2RenderDevice::local_to_global(ISprite& r) {
	m_clip = r.getClip();

	int left = r.getDest().x - r.getOffset().x;
	int right = left + r.getClip().w;
	int up = r.getDest().y - r.getOffset().y;
	int down = up + r.getClip().h;

	// Check whether we need to render.
	// If so, compute the correct clipping.
	if (r.local_frame.w) {
		if (left > r.local_frame.w) {
			return false;
		}
		if (right < 0) {
			return false;
		}
		if (left < 0) {
			m_clip.x = r.getClip().x - left;
			left = 0;
		};
		right = (right < r.local_frame.w ? right : r.local_frame.w);
		m_clip.w = right - left;
	}
	if (r.local_frame.h) {
		if (up > r.local_frame.h) {
			return false;
		}
		if (down < 0) {
			return false;
		}
		if (up < 0) {
			m_clip.y = r.getClip().y - up;
			up = 0;
		};
		down = (down < r.local_frame.h ? down : r.local_frame.h);
		m_clip.h = down - up;
	}

	m_dest.x = left + r.local_frame.x;
	m_dest.y = up + r.local_frame.y;

	return true;
}

/**
 * create blank surface
 */
Image SDL2RenderDevice::createAlphaSurface(int width, int height) {

	Image image;

	if (width > 0 && height > 0) {
		image.surface = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, width, height);
		textures_count+=1;
		if(image.surface == NULL) {
			fprintf(stderr, "CreateRGBSurface failed: %s\n", SDL_GetError());
		}
		else {
				SDL_SetRenderTarget(renderer, image.surface);
				SDL_SetTextureBlendMode(image.surface, SDL_BLENDMODE_BLEND);
				SDL_SetRenderDrawColor(renderer, 0,0,0,0);
				SDL_RenderClear(renderer);
				SDL_SetRenderTarget(renderer, NULL);
		}
	}

	return image;
}

Image SDL2RenderDevice::createSurface(int width, int height) {
	return createAlphaSurface(width, height);
}

void SDL2RenderDevice::setGamma(float g) {
	Uint16 ramp[256];
	SDL_CalculateGammaRamp(g, ramp);
	SDL_SetWindowGammaRamp(screen, ramp, ramp, ramp);
}

void SDL2RenderDevice::listModes(std::vector<Rect> &modes) {
	Rect** detect_modes;
	std::vector<Rect> vec_detect_modes;
	Rect detect_mode;
	SDL_DisplayMode mode;
	/* SDL_compat.c */
	for (int i = 0; i < SDL_GetNumDisplayModes(SDL_GetWindowDisplayIndex(screen)); ++i) {
		SDL_GetDisplayMode(SDL_GetWindowDisplayIndex(screen), i, &mode);

		if (!vec_detect_modes.empty())
		{
			if (vec_detect_modes.back().w == mode.w && vec_detect_modes.back().h == mode.h) {
			continue;
			}
		}

		detect_mode.x = 0;
		detect_mode.y = 0;
		detect_mode.w = mode.w;
		detect_mode.h = mode.h;
		vec_detect_modes.push_back(detect_mode);

	}
	detect_modes = (Rect**)calloc(vec_detect_modes.size(),sizeof(Rect));
	for (unsigned i = 0; i < vec_detect_modes.size(); ++i) {
		detect_modes[i] = &vec_detect_modes[i];
	}
	vec_detect_modes.clear();
	// Check if there are any modes available
	if (detect_modes == (Rect**)0) {
		fprintf(stderr, "No modes available!\n");
		return;
	}

	// Check if our resolution is restricted
	if (detect_modes == (Rect**)-1) {
		fprintf(stderr, "All resolutions available.\n");
	}

	for (unsigned i=0; detect_modes[i]; ++i) {
		modes.push_back(*detect_modes[i]);
		if (detect_modes[i]->w < MIN_VIEW_W || detect_modes[i]->h < MIN_VIEW_H) {
			// make sure the resolution fits in the constraints of MIN_VIEW_W and MIN_VIEW_H
			modes.pop_back();
		}
		else {
			// check previous resolutions for duplicates. If one is found, drop the one we just added
			for (unsigned j=0; j<modes.size()-1; ++j) {
				if (modes[j].w == detect_modes[i]->w && modes[j].h == detect_modes[i]->h) {
					modes.pop_back();
					break;
				}
			}
		}
	}
	if (detect_modes)
		free(detect_modes);
}

Image SDL2RenderDevice::loadGraphicSurface(std::string filename, std::string errormessage, bool IfNotFoundExit, bool HavePinkColorKey) {
	Image image;

	if (HavePinkColorKey) {
		// SDL_Textures don't support colorkeying
		// so we instead create an SDL_Surface, key it, and convert to a texture
		SDL_Surface* cleanup = IMG_Load(mods->locate(filename).c_str());
		if (cleanup) {
			SDL_SetColorKey(cleanup, true, SDL_MapRGB(cleanup->format, 255, 0, 255));
			image.surface = SDL_CreateTextureFromSurface(renderer, cleanup);
			textures_count+=1;
			SDL_FreeSurface(cleanup);
		}
	}
	else {
		image.surface = IMG_LoadTexture(renderer, mods->locate(filename).c_str());
		textures_count+=1;
	}

	if(image.graphicIsNull()) {
		if (!errormessage.empty())
			fprintf(stderr, "%s: %s\n", errormessage.c_str(), IMG_GetError());
		if (IfNotFoundExit) {
			SDL_Quit();
			exit(1);
		}
	}

	return image;
}

void SDL2RenderDevice::scaleSurface(Image *source, int width, int height) {
	if (!source || !source->surface) return;

	Image dest = createAlphaSurface(width, height);
	if (dest.surface) {
		// copy the source texture to the new texture, stretching it in the process
		SDL_SetRenderTarget(renderer, dest.surface);
		SDL_RenderCopyEx(renderer, source->surface, NULL, NULL, 0, NULL, SDL_FLIP_NONE);
		SDL_SetRenderTarget(renderer, NULL);

		// Remove the old surface
		SDL_DestroyTexture(source->surface);
		textures_count-=1;
		source->surface = dest.surface;
	}
}

Uint32 SDL2RenderDevice::readPixel(Image *image, int x, int y) {
	//Unimplemented
	return 0;
}

/*
 * Returns false if a pixel at Point px is transparent
 */
bool SDL2RenderDevice::checkPixel(Point px, Image *image) {
	//Unimplemented
	return true;
}

void SDL2RenderDevice::freeImage(Image *image) {
	if (image && image->surface)
	{
		SDL_DestroyTexture(image->surface);
		textures_count-=1;
	}
}

void setSDL_RGBA(Uint32 *rmask, Uint32 *gmask, Uint32 *bmask, Uint32 *amask) {
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

