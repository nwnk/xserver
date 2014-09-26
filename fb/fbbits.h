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

#ifdef GLYPH
#if BITMAP_BIT_ORDER == LSBFirst
#define WRITE_ADDR1(n)	    (n)
#define WRITE_ADDR2(n)	    (n)
#define WRITE_ADDR4(n)	    (n)
#else
#define WRITE_ADDR1(n)	    ((n) ^ 3)
#define WRITE_ADDR2(n)	    ((n) ^ 2)
#define WRITE_ADDR4(n)	    ((n))
#endif

#define WRITE1(d,n,fg)	    WRITE(d + WRITE_ADDR1(n), (BITS) (fg))

#ifdef BITS2
#define WRITE2(d,n,fg)	    WRITE((BITS2 *) &((d)[WRITE_ADDR2(n)]), (BITS2) (fg))
#else
#define WRITE2(d,n,fg)	    (WRITE1(d,n,fg), WRITE1(d,(n)+1,fg))
#endif

#ifdef BITS4
#define WRITE4(d,n,fg)	    WRITE((BITS4 *) &((d)[WRITE_ADDR4(n)]), (BITS4) (fg))
#else
#define WRITE4(d,n,fg)	    (WRITE2(d,n,fg), WRITE2(d,(n)+2,fg))
#endif

void
GLYPH(FbBits * dstBits,
      FbStride dstStride,
      int dstBpp, FbStip * stipple, FbBits fg, int x, int height)
{
    int lshift;
    FbStip bits;
    BITS *dstLine;
    BITS *dst;
    int n;
    int shift;

    dstLine = (BITS *) dstBits;
    dstLine += x & ~3;
    dstStride *= (sizeof(FbBits) / sizeof(BITS));
    shift = x & 3;
    lshift = 4 - shift;
    while (height--) {
        bits = *stipple++;
        dst = (BITS *) dstLine;
        n = lshift;
        while (bits) {
            switch (FbStipMoveLsb(FbLeftStipBits(bits, n), 4, n)) {
            case 0:
                break;
            case 1:
                WRITE1(dst, 0, fg);
                break;
            case 2:
                WRITE1(dst, 1, fg);
                break;
            case 3:
                WRITE2(dst, 0, fg);
                break;
            case 4:
                WRITE1(dst, 2, fg);
                break;
            case 5:
                WRITE1(dst, 0, fg);
                WRITE1(dst, 2, fg);
                break;
            case 6:
                WRITE1(dst, 1, fg);
                WRITE1(dst, 2, fg);
                break;
            case 7:
                WRITE2(dst, 0, fg);
                WRITE1(dst, 2, fg);
                break;
            case 8:
                WRITE1(dst, 3, fg);
                break;
            case 9:
                WRITE1(dst, 0, fg);
                WRITE1(dst, 3, fg);
                break;
            case 10:
                WRITE1(dst, 1, fg);
                WRITE1(dst, 3, fg);
                break;
            case 11:
                WRITE2(dst, 0, fg);
                WRITE1(dst, 3, fg);
                break;
            case 12:
                WRITE2(dst, 2, fg);
                break;
            case 13:
                WRITE1(dst, 0, fg);
                WRITE2(dst, 2, fg);
                break;
            case 14:
                WRITE1(dst, 1, fg);
                WRITE2(dst, 2, fg);
                break;
            case 15:
                WRITE4(dst, 0, fg);
                break;
            }
            bits = FbStipLeft(bits, n);
            n = 4;
            dst += 4;
        }
        dstLine += dstStride;
    }
}

#undef WRITE_ADDR1
#undef WRITE_ADDR2
#undef WRITE_ADDR4
#undef WRITE1
#undef WRITE2
#undef WRITE4

#endif

#ifdef POLYLINE
void
POLYLINE(DrawablePtr pDrawable,
         GCPtr pGC, int mode, int npt, DDXPointPtr ptsOrig)
{
    INT32 *pts = (INT32 *) ptsOrig;
    int xoff = pDrawable->x;
    int yoff = pDrawable->y;
    unsigned int bias = miGetZeroLineBias(pDrawable->pScreen);
    BoxPtr pBox = RegionExtents(fbGetCompositeClip(pGC));

    FbBits *dst;
    int dstStride;
    int dstBpp;
    int dstXoff, dstYoff;

    UNIT *bits, *bitsBase;
    FbStride bitsStride;
    BITS xor = fbGetGCPrivate(pGC)->xor;
    BITS and = fbGetGCPrivate(pGC)->and;
    int dashoffset = 0;

    INT32 ul, lr;
    INT32 pt1, pt2;

    int e, e1, e3, len;
    int stepmajor, stepminor;
    int octant;

    if (mode == CoordModePrevious)
        fbFixCoordModePrevious(npt, ptsOrig);

    fbGetDrawable(pDrawable, dst, dstStride, dstBpp, dstXoff, dstYoff);
    bitsStride = dstStride * (sizeof(FbBits) / sizeof(UNIT));
    bitsBase =
        ((UNIT *) dst) + (yoff + dstYoff) * bitsStride + (xoff + dstXoff);
    ul = coordToInt(pBox->x1 - xoff, pBox->y1 - yoff);
    lr = coordToInt(pBox->x2 - xoff - 1, pBox->y2 - yoff - 1);

    pt1 = *pts++;
    npt--;
    pt2 = *pts++;
    npt--;
    for (;;) {
        if (isClipped(pt1, ul, lr) | isClipped(pt2, ul, lr)) {
            fbSegment(pDrawable, pGC,
                      intToX(pt1) + xoff, intToY(pt1) + yoff,
                      intToX(pt2) + xoff, intToY(pt2) + yoff,
                      npt == 0 && pGC->capStyle != CapNotLast, &dashoffset);
            if (!npt) {
                fbFinishAccess(pDrawable);
                return;
            }
            pt1 = pt2;
            pt2 = *pts++;
            npt--;
        }
        else {
            bits = bitsBase + intToY(pt1) * bitsStride + intToX(pt1);
            for (;;) {
                CalcLineDeltas(intToX(pt1), intToY(pt1),
                               intToX(pt2), intToY(pt2),
                               len, e1, stepmajor, stepminor, 1, bitsStride,
                               octant);
                if (len < e1) {
                    e3 = len;
                    len = e1;
                    e1 = e3;

                    e3 = stepminor;
                    stepminor = stepmajor;
                    stepmajor = e3;
                    SetYMajorOctant(octant);
                }
                e = -len;
                e1 <<= 1;
                e3 = e << 1;
                FIXUP_ERROR(e, octant, bias);
                if (and == 0) {
                    while (len--) {
                        STORE(bits, xor);
                        bits += stepmajor;
                        e += e1;
                        if (e >= 0) {
                            bits += stepminor;
                            e += e3;
                        }
                    }
                }
                else {
                    while (len--) {
                        RROP(bits, and, xor);
                        bits += stepmajor;
                        e += e1;
                        if (e >= 0) {
                            bits += stepminor;
                            e += e3;
                        }
                    }
                }
                if (!npt) {
                    if (pGC->capStyle != CapNotLast &&
                        pt2 != *((INT32 *) ptsOrig)) {
                        RROP(bits, and, xor);
                    }
                    fbFinishAccess(pDrawable);
                    return;
                }
                pt1 = pt2;
                pt2 = *pts++;
                --npt;
                if (isClipped(pt2, ul, lr))
                    break;
            }
        }
    }

    fbFinishAccess(pDrawable);
}
#endif

#ifdef POLYSEGMENT
void
POLYSEGMENT(DrawablePtr pDrawable, GCPtr pGC, int nseg, xSegment * pseg)
{
    INT32 *pts = (INT32 *) pseg;
    int xoff = pDrawable->x;
    int yoff = pDrawable->y;
    unsigned int bias = miGetZeroLineBias(pDrawable->pScreen);
    BoxPtr pBox = RegionExtents(fbGetCompositeClip(pGC));

    FbBits *dst;
    int dstStride;
    int dstBpp;
    int dstXoff, dstYoff;

    UNIT *bits, *bitsBase;
    FbStride bitsStride;
    FbBits xorBits = fbGetGCPrivate(pGC)->xor;
    FbBits andBits = fbGetGCPrivate(pGC)->and;
    BITS xor = xorBits;
    BITS and = andBits;
    int dashoffset = 0;

    INT32 ul, lr;
    INT32 pt1, pt2;

    int e, e1, e3, len;
    int stepmajor, stepminor;
    int octant;
    Bool capNotLast;

    fbGetDrawable(pDrawable, dst, dstStride, dstBpp, dstXoff, dstYoff);
    bitsStride = dstStride * (sizeof(FbBits) / sizeof(UNIT));
    bitsBase =
        ((UNIT *) dst) + (yoff + dstYoff) * bitsStride + (xoff + dstXoff);
    ul = coordToInt(pBox->x1 - xoff, pBox->y1 - yoff);
    lr = coordToInt(pBox->x2 - xoff - 1, pBox->y2 - yoff - 1);

    capNotLast = pGC->capStyle == CapNotLast;

    while (nseg--) {
        pt1 = *pts++;
        pt2 = *pts++;
        if (isClipped(pt1, ul, lr) | isClipped(pt2, ul, lr)) {
            fbSegment(pDrawable, pGC,
                      intToX(pt1) + xoff, intToY(pt1) + yoff,
                      intToX(pt2) + xoff, intToY(pt2) + yoff,
                      !capNotLast, &dashoffset);
        }
        else {
            CalcLineDeltas(intToX(pt1), intToY(pt1),
                           intToX(pt2), intToY(pt2),
                           len, e1, stepmajor, stepminor, 1, bitsStride,
                           octant);
            if (e1 == 0 && len > 3) {
                int x1, x2;
                FbBits *dstLine;
                int dstX, width;
                FbBits startmask, endmask;
                int nmiddle;

                if (stepmajor < 0) {
                    x1 = intToX(pt2);
                    x2 = intToX(pt1) + 1;
                    if (capNotLast)
                        x1++;
                }
                else {
                    x1 = intToX(pt1);
                    x2 = intToX(pt2);
                    if (!capNotLast)
                        x2++;
                }
                dstX = (x1 + xoff + dstXoff) * (sizeof(UNIT) * 8);
                width = (x2 - x1) * (sizeof(UNIT) * 8);

                dstLine = dst + (intToY(pt1) + yoff + dstYoff) * dstStride;
                dstLine += dstX >> FB_SHIFT;
                dstX &= FB_MASK;
                FbMaskBits(dstX, width, startmask, nmiddle, endmask);
                if (startmask) {
                    WRITE(dstLine,
                          FbDoMaskRRop(READ(dstLine), andBits, xorBits,
                                       startmask));
                    dstLine++;
                }
                if (!andBits)
                    while (nmiddle--)
                        WRITE(dstLine++, xorBits);
                else
                    while (nmiddle--) {
                        WRITE(dstLine,
                              FbDoRRop(READ(dstLine), andBits, xorBits));
                        dstLine++;
                    }
                if (endmask)
                    WRITE(dstLine,
                          FbDoMaskRRop(READ(dstLine), andBits, xorBits,
                                       endmask));
            }
            else {
                bits = bitsBase + intToY(pt1) * bitsStride + intToX(pt1);
                if (len < e1) {
                    e3 = len;
                    len = e1;
                    e1 = e3;

                    e3 = stepminor;
                    stepminor = stepmajor;
                    stepmajor = e3;
                    SetYMajorOctant(octant);
                }
                e = -len;
                e1 <<= 1;
                e3 = e << 1;
                FIXUP_ERROR(e, octant, bias);
                if (!capNotLast)
                    len++;
                if (and == 0) {
                    while (len--) {
                        STORE(bits, xor);
                        bits += stepmajor;
                        e += e1;
                        if (e >= 0) {
                            bits += stepminor;
                            e += e3;
                        }
                    }
                }
                else {
                    while (len--) {
                        RROP(bits, and, xor);
                        bits += stepmajor;
                        e += e1;
                        if (e >= 0) {
                            bits += stepminor;
                            e += e3;
                        }
                    }
                }
            }
        }
    }

    fbFinishAccess(pDrawable);
}
#endif

#undef STORE
#undef RROP
#undef UNIT

#undef isClipped
