
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#ifndef __MIOVERLAY_H
#define __MIOVERLAY_H

typedef void (*miOverlayTransFunc) (ScreenPtr, int, BoxPtr);
typedef Bool (*miOverlayInOverlayFunc) (WindowPtr);

extern XORG_EXPORT Bool

miInitOverlay(ScreenPtr pScreen,
              miOverlayInOverlayFunc inOverlay, miOverlayTransFunc trans);

extern XORG_EXPORT Bool

miOverlayGetPrivateClips(WindowPtr pWin,
                         RegionPtr *borderClip, RegionPtr *clipList);

extern XORG_EXPORT Bool miOverlayCollectUnderlayRegions(WindowPtr, RegionPtr *);
extern XORG_EXPORT void miOverlayComputeCompositeClip(GCPtr, WindowPtr);
extern XORG_EXPORT Bool miOverlayCopyUnderlay(ScreenPtr);
extern XORG_EXPORT void miOverlaySetTransFunction(ScreenPtr, miOverlayTransFunc);
extern XORG_EXPORT void miOverlaySetRootClip(ScreenPtr, Bool);

#endif                          /* __MIOVERLAY_H */
