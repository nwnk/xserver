/*
 * Copyright (c) 1997-2003 by The XFree86 Project, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the copyright holder(s)
 * and author(s) shall not be used in advertising or otherwise to promote
 * the sale, use or other dealings in this Software without prior written
 * authorization from the copyright holder(s) and author(s).
 */

/*
 * This file contains declarations for public XFree86 functions and variables,
 * and definitions of public macros.
 *
 * "public" means available to video drivers.
 */

#ifndef _XF86_H
#define _XF86_H

#if HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#elif HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "xf86str.h"
#include "xf86Opt.h"
#include <X11/Xfuncproto.h>
#include <stdarg.h>
#ifdef RANDR
#include <X11/extensions/randr.h>
#endif

#include "propertyst.h"

/* General parameters */
extern XORG_EXPORT_VAR int xf86DoConfigure;
extern XORG_EXPORT_VAR int xf86DoShowOptions;
extern XORG_EXPORT_VAR Bool xf86DoConfigurePass1;
extern XORG_EXPORT_VAR Bool xf86ProbeIgnorePrimary;
extern XORG_EXPORT_VAR Bool xorgHWAccess;

extern XORG_EXPORT_VAR DevPrivateKeyRec xf86ScreenKeyRec;

#define xf86ScreenKey (&xf86ScreenKeyRec)

extern XORG_EXPORT_VAR ScrnInfoPtr *xf86Screens;      /* List of pointers to ScrnInfoRecs */
extern XORG_EXPORT const unsigned char byte_reversed[256];
extern XORG_EXPORT_VAR Bool fbSlotClaimed;

#if (defined(__sparc__) || defined(__sparc)) && !defined(__OpenBSD__)
extern XORG_EXPORT_VAR Bool sbusSlotClaimed;
#endif

#if defined(XSERVER_PLATFORM_BUS)
extern XORG_EXPORT_VAR int platformSlotClaimed;
#endif

extern XORG_EXPORT_VAR confDRIRec xf86ConfigDRI;
extern XORG_EXPORT_VAR Bool xf86DRI2Enabled(void);

extern XORG_EXPORT_VAR Bool VTSwitchEnabled;  /* kbd driver */

#define XF86SCRNINFO(p) xf86ScreenToScrn(p)

#define XF86FLIP_PIXELS() \
	do { \
	    if (xf86GetFlipPixels()) { \
		pScreen->whitePixel = (pScreen->whitePixel) ? 0 : 1; \
		pScreen->blackPixel = (pScreen->blackPixel) ? 0 : 1; \
	   } \
	while (0)

#define BOOLTOSTRING(b) ((b) ? "TRUE" : "FALSE")

/* Compatibility functions for pre-input-thread drivers */
static inline _X_DEPRECATED int xf86BlockSIGIO(void) { input_lock(); return 0; }
static inline _X_DEPRECATED void xf86UnblockSIGIO(int wasset) { input_unlock(); }

/* Function Prototypes */
#ifndef _NO_XF86_PROTOTYPES

/* PCI related */
#ifdef XSERVER_LIBPCIACCESS
#include <pciaccess.h>
extern XORG_EXPORT_VAR int pciSlotClaimed;

extern XORG_EXPORT Bool xf86CheckPciSlot(const struct pci_device *);
extern XORG_EXPORT int xf86ClaimPciSlot(struct pci_device *, DriverPtr drvp,
                                      int chipset, GDevPtr dev, Bool active);
extern XORG_EXPORT void xf86UnclaimPciSlot(struct pci_device *, GDevPtr dev);
extern XORG_EXPORT Bool xf86ParsePciBusString(const char *busID, int *bus,
                                            int *device, int *func);
extern XORG_EXPORT Bool xf86ComparePciBusString(const char *busID, int bus,
                                              int device, int func);
extern XORG_EXPORT Bool xf86IsPrimaryPci(struct pci_device *pPci);
extern XORG_EXPORT Bool xf86CheckPciMemBase(struct pci_device *pPci,
                                          memType base);
extern XORG_EXPORT struct pci_device *xf86GetPciInfoForEntity(int entityIndex);
extern XORG_EXPORT int xf86MatchPciInstances(const char *driverName,
                                           int vendorID, SymTabPtr chipsets,
                                           PciChipsets * PCIchipsets,
                                           GDevPtr * devList, int numDevs,
                                           DriverPtr drvp, int **foundEntities);
extern XORG_EXPORT ScrnInfoPtr xf86ConfigPciEntity(ScrnInfoPtr pScrn,
                                                 int scrnFlag, int entityIndex,
                                                 PciChipsets * p_chip,
                                                 void *dummy, EntityProc init,
                                                 EntityProc enter,
                                                 EntityProc leave,
                                                 void *private);
#else
#define xf86VGAarbiterInit() do {} while (0)
#define xf86VGAarbiterFini() do {} while (0)
#define xf86VGAarbiterLock(x) do {} while (0)
#define xf86VGAarbiterUnlock(x) do {} while (0)
#define xf86VGAarbiterScrnInit(x) do {} while (0)
#define xf86VGAarbiterDeviceDecodes() do {} while (0)
#define xf86VGAarbiterWrapFunctions() do {} while (0)
#endif

/* xf86Bus.c */

extern XORG_EXPORT int xf86GetFbInfoForScreen(int scrnIndex);
extern XORG_EXPORT int xf86ClaimFbSlot(DriverPtr drvp, int chipset, GDevPtr dev,
                                     Bool active);
extern XORG_EXPORT int xf86ClaimNoSlot(DriverPtr drvp, int chipset, GDevPtr dev,
                                     Bool active);
extern XORG_EXPORT Bool xf86DriverHasEntities(DriverPtr drvp);
extern XORG_EXPORT void xf86AddEntityToScreen(ScrnInfoPtr pScrn, int entityIndex);
extern XORG_EXPORT void xf86SetEntityInstanceForScreen(ScrnInfoPtr pScrn,
                                                     int entityIndex,
                                                     int instance);
extern XORG_EXPORT int xf86GetNumEntityInstances(int entityIndex);
extern XORG_EXPORT GDevPtr xf86GetDevFromEntity(int entityIndex, int instance);
extern XORG_EXPORT void xf86RemoveEntityFromScreen(ScrnInfoPtr pScrn,
                                                 int entityIndex);
extern XORG_EXPORT EntityInfoPtr xf86GetEntityInfo(int entityIndex);
extern XORG_EXPORT Bool xf86SetEntityFuncs(int entityIndex, EntityProc init,
                                         EntityProc enter, EntityProc leave,
                                         void *);
extern XORG_EXPORT Bool xf86IsEntityPrimary(int entityIndex);
extern XORG_EXPORT ScrnInfoPtr xf86FindScreenForEntity(int entityIndex);

extern XORG_EXPORT int xf86GetLastScrnFlag(int entityIndex);
extern XORG_EXPORT void xf86SetLastScrnFlag(int entityIndex, int scrnIndex);
extern XORG_EXPORT Bool xf86IsEntityShared(int entityIndex);
extern XORG_EXPORT void xf86SetEntityShared(int entityIndex);
extern XORG_EXPORT Bool xf86IsEntitySharable(int entityIndex);
extern XORG_EXPORT void xf86SetEntitySharable(int entityIndex);
extern XORG_EXPORT Bool xf86IsPrimInitDone(int entityIndex);
extern XORG_EXPORT void xf86SetPrimInitDone(int entityIndex);
extern XORG_EXPORT void xf86ClearPrimInitDone(int entityIndex);
extern XORG_EXPORT int xf86AllocateEntityPrivateIndex(void);
extern XORG_EXPORT DevUnion *xf86GetEntityPrivate(int entityIndex, int privIndex);

/* xf86Configure.c */
extern XORG_EXPORT GDevPtr xf86AddBusDeviceToConfigure(const char *driver,
                                                     BusType bus, void *busData,
                                                     int chipset);

/* xf86Cursor.c */

extern XORG_EXPORT void xf86LockZoom(ScreenPtr pScreen, int lock);
extern XORG_EXPORT void xf86InitViewport(ScrnInfoPtr pScr);
extern XORG_EXPORT void xf86SetViewport(ScreenPtr pScreen, int x, int y);
extern XORG_EXPORT void xf86ZoomViewport(ScreenPtr pScreen, int zoom);
extern XORG_EXPORT Bool xf86SwitchMode(ScreenPtr pScreen, DisplayModePtr mode);
extern XORG_EXPORT void *xf86GetPointerScreenFuncs(void);
extern XORG_EXPORT void xf86InitOrigins(void);
extern XORG_EXPORT void xf86ReconfigureLayout(void);

/* xf86DPMS.c */

extern XORG_EXPORT Bool xf86DPMSInit(ScreenPtr pScreen, DPMSSetProcPtr set,
                                   int flags);

/* xf86DGA.c */

#ifdef XFreeXDGA
extern XORG_EXPORT Bool DGAInit(ScreenPtr pScreen, DGAFunctionPtr funcs,
                              DGAModePtr modes, int num);
extern XORG_EXPORT Bool DGAReInitModes(ScreenPtr pScreen, DGAModePtr modes,
                                     int num);
extern XORG_EXPORT_VAR xf86SetDGAModeProc xf86SetDGAMode;
#endif

/* xf86Events.c */

typedef struct _InputInfoRec *InputInfoPtr;

extern XORG_EXPORT void SetTimeSinceLastInputEvent(void);
extern XORG_EXPORT void *xf86AddInputHandler(int fd, InputHandlerProc proc,
                                             void *data);
extern XORG_EXPORT int xf86RemoveInputHandler(void *handler);
extern XORG_EXPORT void xf86DisableInputHandler(void *handler);
extern XORG_EXPORT void xf86EnableInputHandler(void *handler);
extern XORG_EXPORT void *xf86AddGeneralHandler(int fd, InputHandlerProc proc,
                                               void *data);
extern XORG_EXPORT int xf86RemoveGeneralHandler(void *handler);
extern XORG_EXPORT void xf86DisableGeneralHandler(void *handler);
extern XORG_EXPORT void xf86EnableGeneralHandler(void *handler);
extern XORG_EXPORT InputHandlerProc xf86SetConsoleHandler(InputHandlerProc
                                                        handler, void *data);
extern XORG_EXPORT void xf86InterceptSignals(int *signo);
extern XORG_EXPORT void xf86InterceptSigIll(void (*sigillhandler) (void));
extern XORG_EXPORT Bool xf86EnableVTSwitch(Bool new);
extern XORG_EXPORT void xf86ProcessActionEvent(ActionEvent action, void *arg);
extern XORG_EXPORT void xf86PrintBacktrace(void);
extern XORG_EXPORT Bool xf86VTOwner(void);
extern XORG_EXPORT void xf86VTLeave(void);
extern XORG_EXPORT void xf86VTEnter(void);
extern XORG_EXPORT void xf86EnableInputDeviceForVTSwitch(InputInfoPtr pInfo);
extern XORG_EXPORT void xf86DisableInputDeviceForVTSwitch(InputInfoPtr pInfo);

/* xf86Helper.c */

extern XORG_EXPORT void xf86AddDriver(DriverPtr driver, void *module,
                                    int flags);
extern XORG_EXPORT void xf86DeleteDriver(int drvIndex);
extern XORG_EXPORT ScrnInfoPtr xf86AllocateScreen(DriverPtr drv, int flags);
extern XORG_EXPORT void xf86DeleteScreen(ScrnInfoPtr pScrn);
extern XORG_EXPORT int xf86AllocateScrnInfoPrivateIndex(void);
extern XORG_EXPORT Bool xf86AddPixFormat(ScrnInfoPtr pScrn, int depth, int bpp,
                                       int pad);
extern XORG_EXPORT Bool xf86SetDepthBpp(ScrnInfoPtr scrp, int depth, int bpp,
                                      int fbbpp, int depth24flags);
extern XORG_EXPORT void xf86PrintDepthBpp(ScrnInfoPtr scrp);
extern XORG_EXPORT Bool xf86SetWeight(ScrnInfoPtr scrp, rgb weight, rgb mask);
extern XORG_EXPORT Bool xf86SetDefaultVisual(ScrnInfoPtr scrp, int visual);
extern XORG_EXPORT Bool xf86SetGamma(ScrnInfoPtr scrp, Gamma newGamma);
extern XORG_EXPORT void xf86SetDpi(ScrnInfoPtr pScrn, int x, int y);
extern XORG_EXPORT void xf86SetBlackWhitePixels(ScreenPtr pScreen);
extern XORG_EXPORT void xf86EnableDisableFBAccess(ScrnInfoPtr pScrn, Bool enable);
extern XORG_EXPORT void
xf86VDrvMsgVerb(int scrnIndex, MessageType type, int verb,
                const char *format, va_list args)
_X_ATTRIBUTE_PRINTF(4, 0);
extern XORG_EXPORT void
xf86DrvMsgVerb(int scrnIndex, MessageType type, int verb,
               const char *format, ...)
_X_ATTRIBUTE_PRINTF(4, 5);
extern XORG_EXPORT void
xf86DrvMsg(int scrnIndex, MessageType type, const char *format, ...)
_X_ATTRIBUTE_PRINTF(3, 4);
extern XORG_EXPORT void
xf86MsgVerb(MessageType type, int verb, const char *format, ...)
_X_ATTRIBUTE_PRINTF(3, 4);
extern XORG_EXPORT void
xf86Msg(MessageType type, const char *format, ...)
_X_ATTRIBUTE_PRINTF(2, 3);
extern XORG_EXPORT void
xf86ErrorFVerb(int verb, const char *format, ...)
_X_ATTRIBUTE_PRINTF(2, 3);
extern XORG_EXPORT void
xf86ErrorF(const char *format, ...)
_X_ATTRIBUTE_PRINTF(1, 2);
extern XORG_EXPORT const char *
xf86TokenToString(SymTabPtr table, int token);
extern XORG_EXPORT int
xf86StringToToken(SymTabPtr table, const char *string);
extern XORG_EXPORT void
xf86ShowClocks(ScrnInfoPtr scrp, MessageType from);
extern XORG_EXPORT void
xf86PrintChipsets(const char *drvname, const char *drvmsg, SymTabPtr chips);
extern XORG_EXPORT int
xf86MatchDevice(const char *drivername, GDevPtr ** driversectlist);
extern XORG_EXPORT const char *
xf86GetVisualName(int visual);
extern XORG_EXPORT int
xf86GetVerbosity(void);
extern XORG_EXPORT int
xf86GetDepth(void);
extern XORG_EXPORT rgb
xf86GetWeight(void);
extern XORG_EXPORT Gamma
xf86GetGamma(void);
extern XORG_EXPORT Bool
xf86GetFlipPixels(void);
extern XORG_EXPORT const char *
xf86GetServerName(void);
extern XORG_EXPORT Bool
xf86ServerIsExiting(void);
extern XORG_EXPORT Bool
xf86ServerIsResetting(void);
extern XORG_EXPORT Bool
xf86ServerIsOnlyDetecting(void);
extern XORG_EXPORT Bool
xf86CaughtSignal(void);
extern XORG_EXPORT Bool
xf86GetVidModeAllowNonLocal(void);
extern XORG_EXPORT Bool
xf86GetVidModeEnabled(void);
extern XORG_EXPORT Bool
xf86GetModInDevAllowNonLocal(void);
extern XORG_EXPORT Bool
xf86GetModInDevEnabled(void);
extern XORG_EXPORT Bool
xf86GetAllowMouseOpenFail(void);
extern XORG_EXPORT void
xf86DisableRandR(void);
extern XORG_EXPORT CARD32
xorgGetVersion(void);
extern XORG_EXPORT CARD32
xf86GetModuleVersion(void *module);
extern XORG_EXPORT void *
xf86LoadDrvSubModule(DriverPtr drv, const char *name);
extern XORG_EXPORT void *
xf86LoadSubModule(ScrnInfoPtr pScrn, const char *name);
extern XORG_EXPORT void *
xf86LoadOneModule(const char *name, void *optlist);
extern XORG_EXPORT void
xf86UnloadSubModule(void *mod);
extern XORG_EXPORT Bool
xf86LoaderCheckSymbol(const char *name);
extern XORG_EXPORT void
xf86SetBackingStore(ScreenPtr pScreen);
extern XORG_EXPORT void
xf86SetSilkenMouse(ScreenPtr pScreen);
extern XORG_EXPORT void *
xf86FindXvOptions(ScrnInfoPtr pScrn, int adapt_index, const char *port_name,
                  const char **adaptor_name, void **adaptor_options);
extern XORG_EXPORT ScrnInfoPtr
xf86ConfigFbEntity(ScrnInfoPtr pScrn, int scrnFlag,
                   int entityIndex, EntityProc init,
                   EntityProc enter, EntityProc leave, void *private);

extern XORG_EXPORT Bool
xf86IsScreenPrimary(ScrnInfoPtr pScrn);
extern XORG_EXPORT Bool
xf86IsUnblank(int mode);

/* xf86Init.c */

extern XORG_EXPORT PixmapFormatPtr
xf86GetPixFormat(ScrnInfoPtr pScrn, int depth);
extern XORG_EXPORT int
xf86GetBppFromDepth(ScrnInfoPtr pScrn, int depth);

/* xf86Mode.c */

extern XORG_EXPORT const char *
xf86ModeStatusToString(ModeStatus status);
extern XORG_EXPORT ModeStatus
xf86CheckModeForMonitor(DisplayModePtr mode, MonPtr monitor);
extern XORG_EXPORT ModeStatus
xf86CheckModeForDriver(ScrnInfoPtr scrp, DisplayModePtr mode, int flags);
extern XORG_EXPORT int
xf86ValidateModes(ScrnInfoPtr scrp, DisplayModePtr availModes,
                  const char **modeNames, ClockRangePtr clockRanges,
                  int *linePitches, int minPitch, int maxPitch,
                  int minHeight, int maxHeight, int pitchInc,
                  int virtualX, int virtualY, int apertureSize,
                  LookupModeFlags strategy);
extern XORG_EXPORT void
xf86DeleteMode(DisplayModePtr * modeList, DisplayModePtr mode);
extern XORG_EXPORT void
xf86PruneDriverModes(ScrnInfoPtr scrp);
extern XORG_EXPORT void
xf86SetCrtcForModes(ScrnInfoPtr scrp, int adjustFlags);
extern XORG_EXPORT void
xf86PrintModes(ScrnInfoPtr scrp);
extern XORG_EXPORT void
xf86ShowClockRanges(ScrnInfoPtr scrp, ClockRangePtr clockRanges);

/* xf86Option.c */

extern XORG_EXPORT void
xf86CollectOptions(ScrnInfoPtr pScrn, XF86OptionPtr extraOpts);

/* xf86RandR.c */
#ifdef RANDR
extern XORG_EXPORT Bool
xf86RandRInit(ScreenPtr pScreen);
extern XORG_EXPORT Rotation
xf86GetRotation(ScreenPtr pScreen);
extern XORG_EXPORT Bool
xf86RandRSetNewVirtualAndDimensions(ScreenPtr pScreen,
                                    int newvirtX, int newvirtY,
                                    int newmmWidth, int newmmHeight,
                                    Bool resetMode);
#endif

/* xf86Extensions.c */
extern void xf86ExtensionInit(void);

/* convert ScreenPtr to ScrnInfoPtr */
extern XORG_EXPORT ScrnInfoPtr xf86ScreenToScrn(ScreenPtr pScreen);
/* convert ScrnInfoPtr to ScreenPtr */
extern XORG_EXPORT ScreenPtr xf86ScrnToScreen(ScrnInfoPtr pScrn);

#endif                          /* _NO_XF86_PROTOTYPES */

#define XF86_HAS_SCRN_CONV 1 /* define for drivers to use in api compat */

#define XF86_SCRN_INTERFACE 1 /* define for drivers to use in api compat */

/* flags passed to xf86 allocate screen */
#define XF86_ALLOCATE_GPU_SCREEN 1

/* Update the internal total dimensions of all ScreenRecs together */
extern XORG_EXPORT void
xf86UpdateDesktopDimensions(void);

#endif                          /* _XF86_H */
