/* -*- C++ -*-
 *
 *  Compositor.h - Ponscripter GPU compositor
 *
 *  Copyright (c) 2021 TellowKrinkle
 *
 *  tellowkrinkle@gmail.com
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307 USA
 */

#include <SDL.h>
#include <utility>

class Compositor {
    struct Impl;
    Impl* impl;
    Compositor(const Compositor&); // = delete;
    Compositor& operator=(const Compositor&); // = delete;
public:
    Compositor();
    ~Compositor();

    /// Initialize the compositor
    void initialize(SDL_Renderer* renderer);

    enum Type {
        TYPE_BG     = 1 << 0, ///< BG image, usually on the bottom
        TYPE_SPRITE = 1 << 1, ///< Sprite
        TYPE_HUMAN  = 1 << 2, ///< Human / 立ち絵
        TYPE_TEXT   = 1 << 3, ///< Text
    };

    /// Position of an object in compositing
    class Layer {
        uint16_t _type;
        uint16_t _num;
    public:
        Layer(Type type, uint16_t num) { _type = type; _num = num; }
        Type type() const { return (Type)_type; }
        uint16_t num() const { return _num; }
    };

    struct Image;
    struct CImage;

    /// Reference to a composited image
    class CImageRef {
        friend class Compositor;
        int32_t num;
    };

    /// Set which types are active (or them together)
    void setActiveTypes(Type types);
    /// Add the given types to the set of active types
    void addActiveTypes(Type types);
    /// Remove the given types from the set of active types
    void removeActiveTypes(Type types);
    /// Clear the current active types
    Type getActiveTypes() const;

    /// Only sprites greater than or equal to first, and less than or equal to last will be displayed
    void setActiveSpriteRange(uint16_t first = 0, uint16_t last = UINT16_MAX);
    std::pair<uint16_t, uint16_t> getActiveSpriteRange() const;

    /// Easy reordering of humans.  Set an empty list to disable reordering
    void setHumanOrder(const int* list, size_t length);
    std::pair<const int*, size_t> getHumanOrder() const;

    /// Create a new image of the given size
    Image* newImage(const SDL_Rect& size, SDL_BlendMode blendMode = SDL_BLENDMODE_NONE, bool yuv = false);
    /// Create a new image with the content of the given surface
    Image* newImage(SDL_Surface* base, SDL_BlendMode blendMode = SDL_BLENDMODE_NONE);
    /// Update the content of `dst` in `rect` with pixels specified by `pixels` and `pitch`
    /// @warning Only use this function to update images that are not contained within CImages.  Use the version that takes a CImageRef instead for those
    void updateImage(Image* dst, const SDL_Rect* rect, const void* pixels, int pitch);
    /// Update the content of `dst` in `dstRect` with content of `src` in `srcRect`.  Specify NULL for whole images
    /// @warning Only use this function to update images that are not contained within CImages.  Use the version that takes a CImageRef instead for those
    void updateImage(SDL_Surface* src, const SDL_Rect* srcRect, Image* dst, const SDL_Rect* dstRect) {
        const char* pixels = (const char*)src->pixels;
        if (srcRect) {
            pixels += srcRect->y * src->pitch;
            pixels += srcRect->x * src->format->BytesPerPixel;
        }
        updateImage(dst, dstRect, pixels, src->pitch);
    }
    /// Destroy the given image
    void deleteImage(Image* img);

    /// Create a new composited image
    CImageRef newCImage(Image* img, Layer layer, bool owning);
    CImageRef newCImage(const SDL_Rect& size, Layer layer, SDL_BlendMode blendMode = SDL_BLENDMODE_NONE, bool yuv = false);
    CImageRef newCImage(SDL_Surface* img, Layer layer, SDL_BlendMode blendMode = SDL_BLENDMODE_NONE);
    /// Update the content of `dst` in `rect` with pixels specified by `pixels` and `pitch`
    void updateImage(CImageRef dst, const SDL_Rect* rect, const void* pixels, int pitch);
    /// Update the content of `dst` in `dstRect` with content of `src` in `srcRect`.  Specify NULL for whole images
    void updateImage(SDL_Surface* src, const SDL_Rect* srcRect, CImageRef dst, const SDL_Rect* dstRect) {
        const char* pixels = (const char*)src->pixels;
        if (srcRect) {
            pixels += srcRect->y * src->pitch;
            pixels += srcRect->x * src->format->BytesPerPixel;
        }
        updateImage(dst, dstRect, pixels, src->pitch);
    }
    Layer getLayer(CImageRef img);
    void deleteCImage(CImageRef img);
    void show(CImageRef img, bool shown = true);
};
