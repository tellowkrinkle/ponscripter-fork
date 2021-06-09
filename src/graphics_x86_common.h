/* -*- C++ -*-
 *
 *  graphics_x86_common.h - shared code between SSE graphics backends
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

#ifdef USE_X86_GFX

#include <SDL.h>
#include <immintrin.h>

#include "graphics_common.h"

/// 0x0000gg?? -> 0x00gg00gg
static HELPER_FN __m128i extractFromGTo16L(__m128i v) {
#ifdef __SSSE3__
    __m128i mask = _mm_setr_epi8(1, 0x80, 1, 0x80, 5, 0x80, 5, 0x80, 9, 0x80, 9, 0x80, 13, 0x80, 13, 0x80);
    return _mm_shuffle_epi8(v, mask);
#else
    __m128i shifted = _mm_srli_epi32(v, 8); // 0x000000gg
    return _mm_or_si128(shifted, _mm_slli_epi32(shifted, 16));
#endif
}

/// 0x????gg?? -> 0x000000gg
static HELPER_FN __m128i extractG(__m128i v) {
#ifdef __SSSE3__
    __m128i mask = _mm_setr_epi8(1, 0x80, 0x80, 0x80, 5, 0x80, 0x80, 0x80, 9, 0x80, 0x80, 0x80, 13, 0x80, 0x80, 0x80);
    return _mm_shuffle_epi8(v, mask);
#else
    __m128i shifted = _mm_srli_epi32(v, 8);
    return _mm_and_si128(shifted, _mm_set1_epi32(0xFF));
#endif
}

static HELPER_FN void imageFilterBlend_SSE_Common(Uint32 *dst_buffer, Uint32 *src_buffer, Uint8 *alphap, int alpha, int length) {
    int n = length;

    // Compute first few values so we're on a 16-byte boundary in dst_buffer
    while( (((long)dst_buffer & 0xF) > 0) && (n > 0) ) {
        BLEND_PIXEL();
        --n; ++dst_buffer; ++src_buffer;
    }

    // Do bulk of processing using SSE2 (process 4 32bit (BGRA) pixels)
    // create basic bitmasks 0x00FF00FF, 0x000000FF
    __m128i bmask2 = _mm_set1_epi32(0x00FF00FF);
    while (n >= 4) {
        // alpha1 = ((src_argb >> 24) * alpha) >> 8
        __m128i a = _mm_set1_epi32(alpha);
        __m128i buf = _mm_loadu_si128((__m128i*)src_buffer);
        __m128i tmp = _mm_srli_epi32(buf, 24);
        a = _mm_mullo_epi16(a, tmp);
        // double-up alpha1 (0x0000vvxx -> 0x00vv00vv)
        a = extractFromGTo16L(a);
        // rb = (src_argb & bmask2) * alpha1
        tmp = _mm_and_si128(buf, bmask2);
        __m128i rb = _mm_mullo_epi16(a, tmp);
        // g = ((src_argb >> 8) & bmask) * alpha1
        tmp = extractG(buf);
        __m128i g = _mm_mullo_epi16(a, tmp);
        // alpha2 = alpha1 ^ bmask2
        a = _mm_xor_si128(a, bmask2);
        buf = _mm_load_si128((__m128i*)dst_buffer);
        // rb += (dst_argb & bmask2) * alpha2
        tmp = _mm_and_si128(buf, bmask2);
        tmp = _mm_mullo_epi16(a, tmp);
        rb = _mm_add_epi16(rb, tmp);
        // rb = 0xbbxxrrxx -> 0x00bb00rr
        rb = _mm_srli_epi16(rb, 8);
        // g += ((dst_argb >> 8) & bmask) * alpha2
        tmp = extractG(buf);
        tmp = _mm_mullo_epi16(a, tmp);
        g = _mm_add_epi32(g, tmp);
        // g = g & ~bmask2
        g = _mm_andnot_si128(bmask2, g);
        // dst_argb = rb | g
        tmp = _mm_or_si128(rb, g);
        _mm_store_si128((__m128i*)dst_buffer, tmp);

        n -= 4; src_buffer += 4; dst_buffer += 4; alphap += 16;
    }

    // If any pixels are left over, deal with them individually
    ++n;
    BASIC_BLEND();
}

#endif
