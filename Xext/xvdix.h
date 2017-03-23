/***********************************************************
Copyright 1991 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#ifndef XVDIX_H
#define XVDIX_H
/*
** File:
**
**   xvdix.h --- Xv device independent header file
**
** Author:
**
**   David Carver (Digital Workstation Engineering/Project Athena)
**
** Revisions:
**
**   29.08.91 Carver
**     - removed UnrealizeWindow wrapper unrealizing windows no longer
**       preempts video
**
**   11.06.91 Carver
**     - changed SetPortControl to SetPortAttribute
**     - changed GetPortControl to GetPortAttribute
**     - changed QueryBestSize
**
**   15.05.91 Carver
**     - version 2.0 upgrade
**
**   24.01.91 Carver
**     - version 1.4 upgrade
**
*/

#include "scrnintstr.h"
#include <X11/extensions/Xvproto.h>

extern XORG_EXPORT_VAR unsigned long XvExtensionGeneration;
extern XORG_EXPORT_VAR unsigned long XvScreenGeneration;
extern XORG_EXPORT_VAR unsigned long XvResourceGeneration;

extern XORG_EXPORT_VAR int XvReqCode;
extern XORG_EXPORT_VAR int XvEventBase;
extern XORG_EXPORT_VAR int XvErrorBase;

extern XORG_EXPORT_VAR RESTYPE XvRTPort;
extern XORG_EXPORT_VAR RESTYPE XvRTEncoding;
extern XORG_EXPORT_VAR RESTYPE XvRTGrab;
extern XORG_EXPORT_VAR RESTYPE XvRTVideoNotify;
extern XORG_EXPORT_VAR RESTYPE XvRTVideoNotifyList;
extern XORG_EXPORT_VAR RESTYPE XvRTPortNotify;

typedef struct {
    int numerator;
    int denominator;
} XvRationalRec, *XvRationalPtr;

typedef struct {
    char depth;
    unsigned long visual;
} XvFormatRec, *XvFormatPtr;

typedef struct {
    unsigned long id;
    ClientPtr client;
} XvGrabRec, *XvGrabPtr;

typedef struct _XvVideoNotifyRec {
    struct _XvVideoNotifyRec *next;
    ClientPtr client;
    unsigned long id;
    unsigned long mask;
} XvVideoNotifyRec, *XvVideoNotifyPtr;

typedef struct _XvPortNotifyRec {
    struct _XvPortNotifyRec *next;
    ClientPtr client;
    unsigned long id;
} XvPortNotifyRec, *XvPortNotifyPtr;

typedef struct {
    int id;
    ScreenPtr pScreen;
    char *name;
    unsigned short width, height;
    XvRationalRec rate;
} XvEncodingRec, *XvEncodingPtr;

typedef struct _XvAttributeRec {
    int flags;
    int min_value;
    int max_value;
    char *name;
} XvAttributeRec, *XvAttributePtr;

typedef struct {
    int id;
    int type;
    int byte_order;
    char guid[16];
    int bits_per_pixel;
    int format;
    int num_planes;

    /* for RGB formats only */
    int depth;
    unsigned int red_mask;
    unsigned int green_mask;
    unsigned int blue_mask;

    /* for YUV formats only */
    unsigned int y_sample_bits;
    unsigned int u_sample_bits;
    unsigned int v_sample_bits;
    unsigned int horz_y_period;
    unsigned int horz_u_period;
    unsigned int horz_v_period;
    unsigned int vert_y_period;
    unsigned int vert_u_period;
    unsigned int vert_v_period;
    char component_order[32];
    int scanline_order;
} XvImageRec, *XvImagePtr;

typedef struct {
    unsigned long base_id;
    unsigned char type;
    char *name;
    int nEncodings;
    XvEncodingPtr pEncodings;
    int nFormats;
    XvFormatPtr pFormats;
    int nAttributes;
    XvAttributePtr pAttributes;
    int nImages;
    XvImagePtr pImages;
    int nPorts;
    struct _XvPortRec *pPorts;
    ScreenPtr pScreen;
    int (*ddPutVideo) (DrawablePtr, struct _XvPortRec *, GCPtr,
                       INT16, INT16, CARD16, CARD16,
                       INT16, INT16, CARD16, CARD16);
    int (*ddPutStill) (DrawablePtr, struct _XvPortRec *, GCPtr,
                       INT16, INT16, CARD16, CARD16,
                       INT16, INT16, CARD16, CARD16);
    int (*ddGetVideo) (DrawablePtr, struct _XvPortRec *, GCPtr,
                       INT16, INT16, CARD16, CARD16,
                       INT16, INT16, CARD16, CARD16);
    int (*ddGetStill) (DrawablePtr, struct _XvPortRec *, GCPtr,
                       INT16, INT16, CARD16, CARD16,
                       INT16, INT16, CARD16, CARD16);
    int (*ddStopVideo) (struct _XvPortRec *, DrawablePtr);
    int (*ddSetPortAttribute) (struct _XvPortRec *, Atom, INT32);
    int (*ddGetPortAttribute) (struct _XvPortRec *, Atom, INT32 *);
    int (*ddQueryBestSize) (struct _XvPortRec *, CARD8,
                            CARD16, CARD16, CARD16, CARD16,
                            unsigned int *, unsigned int *);
    int (*ddPutImage) (DrawablePtr, struct _XvPortRec *, GCPtr,
                       INT16, INT16, CARD16, CARD16,
                       INT16, INT16, CARD16, CARD16,
                       XvImagePtr, unsigned char *, Bool, CARD16, CARD16);
    int (*ddQueryImageAttributes) (struct _XvPortRec *, XvImagePtr,
                                   CARD16 *, CARD16 *, int *, int *);
    DevUnion devPriv;
} XvAdaptorRec, *XvAdaptorPtr;

typedef struct _XvPortRec {
    unsigned long id;
    XvAdaptorPtr pAdaptor;
    XvPortNotifyPtr pNotify;
    DrawablePtr pDraw;
    ClientPtr client;
    XvGrabRec grab;
    TimeStamp time;
    DevUnion devPriv;
} XvPortRec, *XvPortPtr;

#define VALIDATE_XV_PORT(portID, pPort, mode)\
    {\
	int rc = dixLookupResourceByType((void **)&(pPort), portID,\
	                                 XvRTPort, client, mode);\
	if (rc != Success)\
	    return rc;\
    }

typedef struct {
    int version, revision;
    int nAdaptors;
    XvAdaptorPtr pAdaptors;
    DestroyWindowProcPtr DestroyWindow;
    DestroyPixmapProcPtr DestroyPixmap;
    CloseScreenProcPtr CloseScreen;
} XvScreenRec, *XvScreenPtr;

#define SCREEN_PROLOGUE(pScreen, field) ((pScreen)->field = ((XvScreenPtr) \
    dixLookupPrivate(&(pScreen)->devPrivates, XvScreenKey))->field)

#define SCREEN_EPILOGUE(pScreen, field, wrapper)\
    ((pScreen)->field = wrapper)

/* Errors */

#define _XvBadPort (XvBadPort+XvErrorBase)
#define _XvBadEncoding (XvBadEncoding+XvErrorBase)

extern XORG_EXPORT int ProcXvDispatch(ClientPtr);
extern XORG_EXPORT int SProcXvDispatch(ClientPtr);

extern XORG_EXPORT int XvScreenInit(ScreenPtr);
extern XORG_EXPORT DevPrivateKey XvGetScreenKey(void);
extern XORG_EXPORT unsigned long XvGetRTPort(void);
extern XORG_EXPORT void XvFreeAdaptor(XvAdaptorPtr pAdaptor);
extern void XORG_EXPORT XvFillColorKey(DrawablePtr pDraw, CARD32 key,
                                     RegionPtr region);
extern XORG_EXPORT int XvdiSendPortNotify(XvPortPtr, Atom, INT32);

extern XORG_EXPORT int XvdiPutVideo(ClientPtr, DrawablePtr, XvPortPtr, GCPtr,
                                  INT16, INT16, CARD16, CARD16,
                                  INT16, INT16, CARD16, CARD16);
extern XORG_EXPORT int XvdiPutStill(ClientPtr, DrawablePtr, XvPortPtr, GCPtr,
                                  INT16, INT16, CARD16, CARD16,
                                  INT16, INT16, CARD16, CARD16);
extern XORG_EXPORT int XvdiGetVideo(ClientPtr, DrawablePtr, XvPortPtr, GCPtr,
                                  INT16, INT16, CARD16, CARD16,
                                  INT16, INT16, CARD16, CARD16);
extern XORG_EXPORT int XvdiGetStill(ClientPtr, DrawablePtr, XvPortPtr, GCPtr,
                                  INT16, INT16, CARD16, CARD16,
                                  INT16, INT16, CARD16, CARD16);
extern XORG_EXPORT int XvdiPutImage(ClientPtr, DrawablePtr, XvPortPtr, GCPtr,
                                  INT16, INT16, CARD16, CARD16,
                                  INT16, INT16, CARD16, CARD16,
                                  XvImagePtr, unsigned char *, Bool,
                                  CARD16, CARD16);
extern XORG_EXPORT int XvdiSelectVideoNotify(ClientPtr, DrawablePtr, BOOL);
extern XORG_EXPORT int XvdiSelectPortNotify(ClientPtr, XvPortPtr, BOOL);
extern XORG_EXPORT int XvdiSetPortAttribute(ClientPtr, XvPortPtr, Atom, INT32);
extern XORG_EXPORT int XvdiGetPortAttribute(ClientPtr, XvPortPtr, Atom, INT32 *);
extern XORG_EXPORT int XvdiStopVideo(ClientPtr, XvPortPtr, DrawablePtr);
extern XORG_EXPORT int XvdiMatchPort(XvPortPtr, DrawablePtr);
extern XORG_EXPORT int XvdiGrabPort(ClientPtr, XvPortPtr, Time, int *);
extern XORG_EXPORT int XvdiUngrabPort(ClientPtr, XvPortPtr, Time);
#endif                          /* XVDIX_H */
