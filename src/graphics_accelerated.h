/* -*- C++ -*-
 *
 *  graphics_accelerated.h - Accelerated graphics function chooser
 *
 *  Copyright (c) 2021 TellowKrinkle
 *
 *  tellowkrinkle@gmail.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, see <http://www.gnu.org/licenses/>
 *  or write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#pragma once

#include <SDL.h>

void imageFilterMean_Basic(unsigned char *src1, unsigned char *src2, unsigned char *dst, int length);
void imageFilterAddTo_Basic(unsigned char *dst, unsigned char *src, int length);
void imageFilterSubFrom_Basic(unsigned char *dst, unsigned char *src, int length);
void imageFilterBlend_Basic(Uint32 *dst_buffer, Uint32 *src_buffer, Uint8 *alphap, int alpha, int length);

class AcceleratedGraphicsFunctions {
    void (*_imageFilterMean)(unsigned char *src1, unsigned char *src2, unsigned char *dst, int length);
    void (*_imageFilterAddTo)(unsigned char *dst, unsigned char *src, int length);
    void (*_imageFilterSubFrom)(unsigned char *dst, unsigned char *src, int length);
    void (*_imageFilterBlend)(Uint32 *dst_buffer, Uint32 *src_buffer, Uint8 *alphap, int alpha, int length);

public:
    AcceleratedGraphicsFunctions() {
        _imageFilterMean = imageFilterMean_Basic;
        _imageFilterAddTo = imageFilterAddTo_Basic;
        _imageFilterSubFrom = imageFilterSubFrom_Basic;
        _imageFilterBlend = imageFilterBlend_Basic;
    }
    static AcceleratedGraphicsFunctions basic() { return AcceleratedGraphicsFunctions(); }
    static AcceleratedGraphicsFunctions accelerated();

    void imageFilterMean(unsigned char *src1, unsigned char *src2, unsigned char *dst, int length) {
        _imageFilterMean(src1, src2, dst, length);
    }

    void imageFilterAddTo(unsigned char *dst, unsigned char *src, int length) {
        _imageFilterAddTo(dst, src, length);
    }

    void imageFilterSubFrom(unsigned char *dst, unsigned char *src, int length) {
        _imageFilterSubFrom(dst, src, length);
    }

    void imageFilterBlend(Uint32 *dst_buffer, Uint32 *src_buffer, Uint8 *alphap, int alpha, int length) {
        _imageFilterBlend(dst_buffer, src_buffer, alphap, alpha, length);
    }
};
