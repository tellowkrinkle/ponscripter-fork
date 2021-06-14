/* -*- C++ -*-
 *
 *  Compositor.cpp - Ponscripter GPU compositor
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

#include "Compositor.h"
#include <vector>
#include <string>

/// An image with information for compositing
struct Compositor::CImage {
    Image* img;   ///< The actual image
    Layer layer;  ///< The image's position
    uint32_t seq; ///< Used to prevent ABA issues
    bool inUse;   ///< Does this CImage represent valid data, or is it just sitting around waiting to be reused?
    bool owned;   ///< Should `img` be freed on destruction?
    bool shown;   ///<
    bool updated; ///< Has the image been changed since it was last drawn?
    CImage(uint32_t seq, Image* img, Layer layer, bool owned)
        : img(img)
        , layer(layer)
        , seq(seq)
        , inUse(true)
        , owned(owned)
        , shown(false)
        , updated(false)
    {
    }
};

static void SDLFailedThrow(const char* operation) {
    throw std::runtime_error(std::string(operation) + ": " + SDL_GetError());
}
static void SDLFailedWarn(const char* operation) {
    fprintf(stderr, "Warning: %s: %s\n", operation, SDL_GetError());
}
static void SDLAutoWarn(int result, const char* operation) {
    if (result) { SDLFailedWarn(operation); }
}

typedef std::vector< std::pair<Compositor::CImageRef, uint32_t> > PresentationList;

struct Compositor::Impl {
    SDL_Renderer* renderer;

    std::vector<int> humanOrder;
    std::vector<CImage> cimages;
    PresentationList lastPresent;
    PresentationList nextPresent;

    uint32_t cimgSeqNext;

    uint16_t activeTypes;
    uint16_t activeRangeBegin;
    uint16_t activeRangeEnd;

    bool needsFullRedraw;

    Impl() {
        renderer = NULL;
        needsFullRedraw = false;
        cimgSeqNext = 0;
        activeTypes = 0xFFFF;
        activeRangeBegin = 0;
        activeRangeEnd = UINT16_MAX;
    }

    ~Impl() {
        for (std::vector<CImage>::iterator i = cimages.begin(); i < cimages.end(); i++) {
            if (i->inUse && i->owned) {
                deleteImage(i->img);
            }
        }
    }

    void initialize(SDL_Renderer* renderer) {
        this->renderer = renderer;
    }

    Image* newImage(const SDL_Rect& size, SDL_BlendMode blendMode = SDL_BLENDMODE_NONE, bool yuv = false) {
        SDL_Texture* tex = SDL_CreateTexture(renderer, yuv ? SDL_PIXELFORMAT_YV12 : SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, size.w, size.h);
        if (!tex) {
            SDLFailedThrow("Failed to create texture");
        }
        SDL_SetTextureBlendMode(tex, blendMode);
        return (Image*)tex;
    }
    Image* newImage(SDL_Surface* base, SDL_BlendMode blendMode) {
        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, base);
        if (!tex) {
            SDLFailedThrow("Failed to create texture");
        }
        SDL_SetTextureBlendMode(tex, blendMode);
        return (Image*)tex;
    }
    void updateImage(Image* dst, const SDL_Rect* rect, const void* pixels, int pitch) {
        SDLAutoWarn(SDL_UpdateTexture((SDL_Texture*)dst, rect, pixels, pitch), "Failed to update texture");
    }
    void deleteImage(Image* img) {
        SDL_DestroyTexture((SDL_Texture*)img);
    }

    CImage& cimg(CImageRef ref) {
        return cimages[ref.num];
    }

    CImageRef newCImage(Image* img, Layer layer, bool owning) {
        std::vector<CImage>::iterator i;
        CImage newImg = CImage(cimgSeqNext++, img, layer, owning);
        for (i = cimages.begin(); i < cimages.end(); i++) {
            if (!i->inUse) {
                *i = newImg;
                break;
            }
        }
        if (i == cimages.end()) {
            cimages.push_back(newImg);
            i = cimages.end() - 1;
        }
        CImageRef out;
        out.num = i - cimages.begin();
        return out;
    }
    void deleteCImage(CImageRef img) {
        CImage& c = cimages[img.num];
        if (c.owned) {
            deleteImage(c.img);
        }
        c.inUse = false;
        while (!cimages.empty() && !cimages.back().inUse) {
            cimages.pop_back();
        }
    }
};

void Compositor::setActiveTypes(Type types) { impl->activeTypes = types; }
void Compositor::addActiveTypes(Type types) { impl->activeTypes |= types; }
void Compositor::removeActiveTypes(Type types) { impl->activeTypes &= ~types; }
Compositor::Type Compositor::getActiveTypes() const { return (Type)impl->activeTypes; }

void Compositor::setActiveSpriteRange(uint16_t first, uint16_t last) {
    impl->activeRangeBegin = first;
    impl->activeRangeEnd = last;
}

std::pair<uint16_t, uint16_t> Compositor::getActiveSpriteRange() const {
    return std::make_pair(impl->activeRangeBegin, impl->activeRangeEnd);
}

void Compositor::setHumanOrder(const int* list, size_t length) {
    if (length == 0) {
        impl->humanOrder.clear();
    } else {
        impl->humanOrder.assign(list, list + length);
    }
}

std::pair<const int*, size_t> Compositor::getHumanOrder() const {
    return std::make_pair(impl->humanOrder.data(), impl->humanOrder.size());
}

Compositor::Image* Compositor::newImage(SDL_Surface* base, SDL_BlendMode blendMode) {
    return impl->newImage(base, blendMode);
}
void Compositor::updateImage(Image* dst, const SDL_Rect* rect, const void* pixels, int pitch) {
    impl->updateImage(dst, rect, pixels, pitch);
}
void Compositor::deleteImage(Image* img) {
    impl->deleteImage(img);
}

Compositor::CImageRef Compositor::newCImage(Image* img, Layer layer, bool owning) {
    return impl->newCImage(img, layer, owning);
}
Compositor::CImageRef Compositor::newCImage(const SDL_Rect& size, Layer layer, SDL_BlendMode blendMode, bool yuv) {
    return newCImage(impl->newImage(size, blendMode, yuv), layer, true);
}
Compositor::CImageRef Compositor::newCImage(SDL_Surface* img, Layer layer, SDL_BlendMode blendMode) {
    return newCImage(impl->newImage(img, blendMode), layer, true);
}
void Compositor::updateImage(CImageRef dst, const SDL_Rect* rect, const void* pixels, int pitch) {
    CImage& cimg = impl->cimg(dst);
    impl->updateImage(cimg.img, rect, pixels, pitch);
    cimg.updated = true;
}
Compositor::Layer Compositor::getLayer(CImageRef img) {
    return impl->cimg(img).layer;
}
void Compositor::deleteCImage(CImageRef img) {
    impl->deleteCImage(img);
}
void Compositor::show(CImageRef img, bool shown) {
    impl->cimg(img).shown = shown;
}
