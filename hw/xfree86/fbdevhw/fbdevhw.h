
#ifndef _FBDEVHW_H_
#define _FBDEVHW_H_

#include "xf86str.h"
#include "colormapst.h"

#define FBDEVHW_PACKED_PIXELS		0       /* Packed Pixels        */
#define FBDEVHW_PLANES			1       /* Non interleaved planes */
#define FBDEVHW_INTERLEAVED_PLANES	2       /* Interleaved planes   */
#define FBDEVHW_TEXT			3       /* Text/attributes      */
#define FBDEVHW_VGA_PLANES		4       /* EGA/VGA planes       */

extern XORG_EXPORT Bool fbdevHWGetRec(ScrnInfoPtr pScrn);
extern XORG_EXPORT void fbdevHWFreeRec(ScrnInfoPtr pScrn);

extern XORG_EXPORT int fbdevHWGetFD(ScrnInfoPtr pScrn);

extern XORG_EXPORT Bool fbdevHWProbe(struct pci_device *pPci, char *device,
                                   char **namep);
extern XORG_EXPORT Bool fbdevHWInit(ScrnInfoPtr pScrn, struct pci_device *pPci,
                                  char *device);

extern XORG_EXPORT char *fbdevHWGetName(ScrnInfoPtr pScrn);
extern XORG_EXPORT int fbdevHWGetDepth(ScrnInfoPtr pScrn, int *fbbpp);
extern XORG_EXPORT int fbdevHWGetLineLength(ScrnInfoPtr pScrn);
extern XORG_EXPORT int fbdevHWGetType(ScrnInfoPtr pScrn);
extern XORG_EXPORT int fbdevHWGetVidmem(ScrnInfoPtr pScrn);

extern XORG_EXPORT void *fbdevHWMapVidmem(ScrnInfoPtr pScrn);
extern XORG_EXPORT int fbdevHWLinearOffset(ScrnInfoPtr pScrn);
extern XORG_EXPORT Bool fbdevHWUnmapVidmem(ScrnInfoPtr pScrn);
extern XORG_EXPORT void *fbdevHWMapMMIO(ScrnInfoPtr pScrn);
extern XORG_EXPORT Bool fbdevHWUnmapMMIO(ScrnInfoPtr pScrn);

extern XORG_EXPORT void fbdevHWSetVideoModes(ScrnInfoPtr pScrn);
extern XORG_EXPORT DisplayModePtr fbdevHWGetBuildinMode(ScrnInfoPtr pScrn);
extern XORG_EXPORT void fbdevHWUseBuildinMode(ScrnInfoPtr pScrn);
extern XORG_EXPORT Bool fbdevHWModeInit(ScrnInfoPtr pScrn, DisplayModePtr mode);
extern XORG_EXPORT void fbdevHWSave(ScrnInfoPtr pScrn);
extern XORG_EXPORT void fbdevHWRestore(ScrnInfoPtr pScrn);

extern XORG_EXPORT void fbdevHWLoadPalette(ScrnInfoPtr pScrn, int numColors,
                                         int *indices, LOCO * colors,
                                         VisualPtr pVisual);

extern XORG_EXPORT ModeStatus fbdevHWValidMode(ScrnInfoPtr pScrn, DisplayModePtr mode,
                                             Bool verbose, int flags);
extern XORG_EXPORT Bool fbdevHWSwitchMode(ScrnInfoPtr pScrn, DisplayModePtr mode);
extern XORG_EXPORT void fbdevHWAdjustFrame(ScrnInfoPtr pScrn, int x, int y);
extern XORG_EXPORT Bool fbdevHWEnterVT(ScrnInfoPtr pScrn);
extern XORG_EXPORT void fbdevHWLeaveVT(ScrnInfoPtr pScrn);
extern XORG_EXPORT void fbdevHWDPMSSet(ScrnInfoPtr pScrn, int mode, int flags);

extern XORG_EXPORT Bool fbdevHWSaveScreen(ScreenPtr pScreen, int mode);

extern XORG_EXPORT xf86SwitchModeProc *fbdevHWSwitchModeWeak(void);
extern XORG_EXPORT xf86AdjustFrameProc *fbdevHWAdjustFrameWeak(void);
extern XORG_EXPORT xf86EnterVTProc *fbdevHWEnterVTWeak(void);
extern XORG_EXPORT xf86LeaveVTProc *fbdevHWLeaveVTWeak(void);
extern XORG_EXPORT xf86ValidModeProc *fbdevHWValidModeWeak(void);
extern XORG_EXPORT xf86DPMSSetProc *fbdevHWDPMSSetWeak(void);
extern XORG_EXPORT xf86LoadPaletteProc *fbdevHWLoadPaletteWeak(void);
extern XORG_EXPORT SaveScreenProcPtr fbdevHWSaveScreenWeak(void);

#endif
