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

#define isClipped(c,ul,lr)  (((c) | ((c) - (ul)) | ((lr) - (c))) & 0x80008000)

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#define STORE(b,x)  WRITE((b), (x))

#define RROP(b,a,x)	WRITE((b), FbDoRRop (READ(b), (a), (x)))

#define UNIT BITS

#ifdef BRESSOLID
static void
BRESSOLID(DrawablePtr pDrawable,
          GCPtr pGC,
          int dashOffset,
          int signdx,
          int signdy, int axis, int x1, int y1, int e, int e1, int e3, int len)
{
    FbBits *dst;
    FbStride dstStride;
    int dstBpp;
    int dstXoff, dstYoff;
    FbGCPrivPtr pPriv = fbGetGCPrivate(pGC);
    UNIT *bits;
    FbStride bitsStride;
    FbStride majorStep, minorStep;
    BITS xor = (BITS) pPriv->xor;

    fbGetDrawable(pDrawable, dst, dstStride, dstBpp, dstXoff, dstYoff);
    bits = ((UNIT *) (dst + ((y1 + dstYoff) * dstStride))) + (x1 + dstXoff);
    bitsStride = dstStride * (sizeof(FbBits) / sizeof(UNIT));
    if (signdy < 0)
        bitsStride = -bitsStride;
    if (axis == X_AXIS) {
        majorStep = signdx;
        minorStep = bitsStride;
    }
    else {
        majorStep = bitsStride;
        minorStep = signdx;
    }
    while (len--) {
        STORE(bits, xor);
        bits += majorStep;
        e += e1;
        if (e >= 0) {
            bits += minorStep;
            e += e3;
        }
    }

    fbFinishAccess(pDrawable);
}
#endif

#ifdef BRESDASH
static void
BRESDASH(DrawablePtr pDrawable,
         GCPtr pGC,
         int dashOffset,
         int signdx,
         int signdy, int axis, int x1, int y1, int e, int e1, int e3, int len)
{
    FbBits *dst;
    FbStride dstStride;
    int dstBpp;
    int dstXoff, dstYoff;
    FbGCPrivPtr pPriv = fbGetGCPrivate(pGC);
    UNIT *bits;
    FbStride bitsStride;
    FbStride majorStep, minorStep;
    BITS xorfg, xorbg;

    FbDashDeclare;
    int dashlen;
    Bool even;
    Bool doOdd;

    fbGetDrawable(pDrawable, dst, dstStride, dstBpp, dstXoff, dstYoff);
    doOdd = pGC->lineStyle == LineDoubleDash;
    xorfg = (BITS) pPriv->xor;
    xorbg = (BITS) pPriv->bgxor;

    FbDashInit(pGC, pPriv, dashOffset, dashlen, even);

    bits = ((UNIT *) (dst + ((y1 + dstYoff) * dstStride))) + (x1 + dstXoff);
    bitsStride = dstStride * (sizeof(FbBits) / sizeof(UNIT));
    if (signdy < 0)
        bitsStride = -bitsStride;
    if (axis == X_AXIS) {
        majorStep = signdx;
        minorStep = bitsStride;
    }
    else {
        majorStep = bitsStride;
        minorStep = signdx;
    }
    if (dashlen >= len)
        dashlen = len;
    if (doOdd) {
        if (!even)
            goto doubleOdd;
        for (;;) {
            len -= dashlen;
            while (dashlen--) {
                STORE(bits, xorfg);
                bits += majorStep;
                if ((e += e1) >= 0) {
                    e += e3;
                    bits += minorStep;
                }
            }
            if (!len)
                break;

            FbDashNextEven(dashlen);

            if (dashlen >= len)
                dashlen = len;
 doubleOdd:
            len -= dashlen;
            while (dashlen--) {
                STORE(bits, xorbg);
                bits += majorStep;
                if ((e += e1) >= 0) {
                    e += e3;
                    bits += minorStep;
                }
            }
            if (!len)
                break;

            FbDashNextOdd(dashlen);

            if (dashlen >= len)
                dashlen = len;
        }
    }
    else {
        if (!even)
            goto onOffOdd;
        for (;;) {
            len -= dashlen;
            while (dashlen--) {
                STORE(bits, xorfg);
                bits += majorStep;
                if ((e += e1) >= 0) {
                    e += e3;
                    bits += minorStep;
                }
            }
            if (!len)
                break;

            FbDashNextEven(dashlen);

            if (dashlen >= len)
                dashlen = len;
 onOffOdd:
            len -= dashlen;
            while (dashlen--) {
                bits += majorStep;
                if ((e += e1) >= 0) {
                    e += e3;
                    bits += minorStep;
                }
            }
            if (!len)
                break;

            FbDashNextOdd(dashlen);

            if (dashlen >= len)
                dashlen = len;
        }
    }

    fbFinishAccess(pDrawable);
}
#endif

#undef STORE
#undef RROP
#undef UNIT

#undef isClipped
