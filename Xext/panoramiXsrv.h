
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#ifndef _PANORAMIXSRV_H_
#define _PANORAMIXSRV_H_

#include "panoramiX.h"

extern XORG_EXPORT_VAR int PanoramiXNumScreens;
extern XORG_EXPORT_VAR int PanoramiXPixWidth;
extern XORG_EXPORT_VAR int PanoramiXPixHeight;
extern XORG_EXPORT_VAR RegionRec PanoramiXScreenRegion;

extern XORG_EXPORT VisualID PanoramiXTranslateVisualID(int screen, VisualID orig);
extern XORG_EXPORT void PanoramiXConsolidate(void);
extern XORG_EXPORT Bool PanoramiXCreateConnectionBlock(void);
extern XORG_EXPORT PanoramiXRes *PanoramiXFindIDByScrnum(RESTYPE, XID, int);
extern XORG_EXPORT Bool
XineramaRegisterConnectionBlockCallback(void (*func) (void));
extern XORG_EXPORT int XineramaDeleteResource(void *, XID);

extern XORG_EXPORT void XineramaReinitData(void);

extern XORG_EXPORT_VAR RESTYPE XRC_DRAWABLE;
extern XORG_EXPORT_VAR RESTYPE XRT_WINDOW;
extern XORG_EXPORT_VAR RESTYPE XRT_PIXMAP;
extern XORG_EXPORT_VAR RESTYPE XRT_GC;
extern XORG_EXPORT_VAR RESTYPE XRT_COLORMAP;
extern XORG_EXPORT_VAR RESTYPE XRT_PICTURE;

/*
 * Drivers are allowed to wrap this function.  Each wrapper can decide that the
 * two visuals are unequal, but if they are deemed equal, the wrapper must call
 * down and return FALSE if the wrapped function does.  This ensures that all
 * layers agree that the visuals are equal.  The first visual is always from
 * screen 0.
 */
typedef Bool (*XineramaVisualsEqualProcPtr) (VisualPtr, ScreenPtr, VisualPtr);
extern XORG_EXPORT_VAR XineramaVisualsEqualProcPtr XineramaVisualsEqualPtr;

extern XORG_EXPORT void XineramaGetImageData(DrawablePtr *pDrawables,
                                           int left,
                                           int top,
                                           int width,
                                           int height,
                                           unsigned int format,
                                           unsigned long planemask,
                                           char *data, int pitch, Bool isRoot);

static inline void
panoramix_setup_ids(PanoramiXRes * resource, ClientPtr client, XID base_id)
{
    int j;

    resource->info[0].id = base_id;
    FOR_NSCREENS_FORWARD_SKIP(j) {
        resource->info[j].id = FakeClientID(client->index);
    }
}

#endif                          /* _PANORAMIXSRV_H_ */
