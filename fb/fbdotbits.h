/*
 * Copyright © 1998 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * This file defines functions for drawing some primitives using
 * underlying datatypes instead of masks
 */

#define isClipped(c,ul,lr)  (((c) | ((c) - (ul)) | ((lr) - (c))) & 0x80008000)

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#define STORE(b,x)  WRITE((b), (x))

#define RROP(b,a,x)	WRITE((b), FbDoRRop (READ(b), (a), (x)))

#define UNIT BITS

#ifdef DOTS
static void
DOTS(FbBits * dst,
     FbStride dstStride,
     int dstBpp,
     BoxPtr pBox,
     xPoint * ptsOrig,
     int npt, int xorg, int yorg, int xoff, int yoff, FbBits and, FbBits xor)
{
    INT32 *pts = (INT32 *) ptsOrig;
    UNIT *bits = (UNIT *) dst;
    UNIT *point;
    BITS bxor = (BITS) xor;
    BITS band = (BITS) and;
    FbStride bitsStride = dstStride * (sizeof(FbBits) / sizeof(UNIT));
    INT32 ul, lr;
    INT32 pt;

    ul = coordToInt(pBox->x1 - xorg, pBox->y1 - yorg);
    lr = coordToInt(pBox->x2 - xorg - 1, pBox->y2 - yorg - 1);

    bits += bitsStride * (yorg + yoff) + (xorg + xoff);

    if (and == 0) {
        while (npt--) {
            pt = *pts++;
            if (!isClipped(pt, ul, lr)) {
                point = bits + intToY(pt) * bitsStride + intToX(pt);
                STORE(point, bxor);
            }
        }
    }
    else {
        while (npt--) {
            pt = *pts++;
            if (!isClipped(pt, ul, lr)) {
                point = bits + intToY(pt) * bitsStride + intToX(pt);
                RROP(point, band, bxor);
            }
        }
    }
}
#endif

#undef STORE
#undef RROP
#undef UNIT

#undef isClipped
