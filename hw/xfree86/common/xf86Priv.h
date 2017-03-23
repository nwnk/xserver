/*
 * Copyright (c) 1997-2002 by The XFree86 Project, Inc.
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
 * This file contains declarations for private XFree86 functions and variables,
 * and definitions of private macros.
 *
 * "private" means not available to video drivers.
 */

#ifndef _XF86PRIV_H
#define _XF86PRIV_H

#include "xf86Privstr.h"
#include "propertyst.h"
#include "input.h"

/*
 * Parameters set ONLY from the command line options
 * The global state of these things is held in xf86InfoRec (when appropriate).
 */
extern XORG_EXPORT const char *xf86ConfigFile;
extern XORG_EXPORT const char *xf86ConfigDir;
extern XORG_EXPORT Bool xf86AllowMouseOpenFail;

#ifdef XF86VIDMODE
extern XORG_EXPORT Bool xf86VidModeDisabled;
extern XORG_EXPORT Bool xf86VidModeAllowNonLocal;
#endif
extern XORG_EXPORT Bool xf86fpFlag;
extern XORG_EXPORT Bool xf86sFlag;
extern XORG_EXPORT Bool xf86bsEnableFlag;
extern XORG_EXPORT Bool xf86bsDisableFlag;
extern XORG_EXPORT Bool xf86silkenMouseDisableFlag;
extern XORG_EXPORT Bool xf86xkbdirFlag;

#ifdef HAVE_ACPI
extern XORG_EXPORT Bool xf86acpiDisableFlag;
#endif
extern XORG_EXPORT char *xf86LayoutName;
extern XORG_EXPORT char *xf86ScreenName;
extern XORG_EXPORT char *xf86PointerName;
extern XORG_EXPORT char *xf86KeyboardName;
extern XORG_EXPORT int xf86FbBpp;
extern XORG_EXPORT int xf86Depth;
extern XORG_EXPORT rgb xf86Weight;
extern XORG_EXPORT Bool xf86FlipPixels;
extern XORG_EXPORT Gamma xf86Gamma;
extern XORG_EXPORT const char *xf86ServerName;

/* Other parameters */

extern XORG_EXPORT xf86InfoRec xf86Info;
extern XORG_EXPORT const char *xf86ModulePath;
extern XORG_EXPORT MessageType xf86ModPathFrom;
extern XORG_EXPORT const char *xf86LogFile;
extern XORG_EXPORT MessageType xf86LogFileFrom;
extern XORG_EXPORT Bool xf86LogFileWasOpened;
extern XORG_EXPORT serverLayoutRec xf86ConfigLayout;

extern XORG_EXPORT DriverPtr *xf86DriverList;
extern XORG_EXPORT int xf86NumDrivers;
extern XORG_EXPORT Bool xf86Resetting;
extern Bool xf86Initialising;
extern XORG_EXPORT int xf86NumScreens;
extern XORG_EXPORT const char *xf86VisualNames[];
extern XORG_EXPORT int xf86Verbose;       /* verbosity level */
extern XORG_EXPORT int xf86LogVerbose;    /* log file verbosity level */

extern ScrnInfoPtr *xf86GPUScreens;      /* List of pointers to ScrnInfoRecs */
extern int xf86NumGPUScreens;
#ifndef DEFAULT_VERBOSE
#define DEFAULT_VERBOSE		0
#endif
#ifndef DEFAULT_LOG_VERBOSE
#define DEFAULT_LOG_VERBOSE	3
#endif
#ifndef DEFAULT_DPI
#define DEFAULT_DPI		96
#endif

/* Function Prototypes */
#ifndef _NO_XF86_PROTOTYPES

/* xf86Bus.c */
extern XORG_EXPORT Bool xf86BusConfig(void);
extern XORG_EXPORT void xf86BusProbe(void);
extern XORG_EXPORT void xf86AccessEnter(void);
extern XORG_EXPORT void xf86AccessLeave(void);
extern XORG_EXPORT void xf86PostProbe(void);
extern XORG_EXPORT void xf86ClearEntityListForScreen(ScrnInfoPtr pScrn);
extern XORG_EXPORT void xf86AddDevToEntity(int entityIndex, GDevPtr dev);
extern XORG_EXPORT void xf86RemoveDevFromEntity(int entityIndex, GDevPtr dev);

/* xf86Config.c */

extern XORG_EXPORT Bool xf86PathIsSafe(const char *path);

/* xf86DefaultModes */

extern XORG_EXPORT const DisplayModeRec xf86DefaultModes[];
extern XORG_EXPORT const int xf86NumDefaultModes;

/* xf86Configure.c */
extern XORG_EXPORT void
DoConfigure(void)
    _X_NORETURN;
extern XORG_EXPORT void
DoShowOptions(void)
    _X_NORETURN;

/* xf86Events.c */

extern XORG_EXPORT void
xf86Wakeup(void *blockData, int err);
extern _X_HIDDEN int
xf86SigWrapper(int signo);
extern XORG_EXPORT void
xf86HandlePMEvents(int fd, void *data);
extern XORG_EXPORT int (*xf86PMGetEventFromOs) (int fd, pmEvent * events,
                                              int num);
extern XORG_EXPORT pmWait (*xf86PMConfirmEventToOs) (int fd, pmEvent event);

/* xf86Helper.c */
extern XORG_EXPORT void
xf86LogInit(void);
extern XORG_EXPORT void
xf86CloseLog(enum ExitCode error);

/* xf86Init.c */
extern XORG_EXPORT Bool
xf86LoadModules(const char **list, void **optlist);
extern XORG_EXPORT int
xf86SetVerbosity(int verb);
extern XORG_EXPORT int
xf86SetLogVerbosity(int verb);
extern XORG_EXPORT Bool
xf86CallDriverProbe(struct _DriverRec *drv, Bool detect_only);
extern XORG_EXPORT Bool
xf86PrivsElevated(void);

#endif                          /* _NO_XF86_PROTOTYPES */

#endif                          /* _XF86PRIV_H */
