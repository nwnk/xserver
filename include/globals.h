
#ifndef _XSERV_GLOBAL_H_
#define _XSERV_GLOBAL_H_

#include "window.h"             /* for WindowPtr */
#include "extinit.h"

/* Global X server variables that are visible to mi, dix, os, and ddx */

extern XORG_EXPORT CARD32 defaultScreenSaverTime;
extern XORG_EXPORT CARD32 defaultScreenSaverInterval;
extern XORG_EXPORT CARD32 ScreenSaverTime;
extern XORG_EXPORT CARD32 ScreenSaverInterval;

#ifdef SCREENSAVER
extern XORG_EXPORT Bool screenSaverSuspended;
#endif

extern XORG_EXPORT const char *defaultFontPath;
extern XORG_EXPORT int monitorResolution;
extern XORG_EXPORT int defaultColorVisualClass;

extern XORG_EXPORT int GrabInProgress;
extern XORG_EXPORT Bool noTestExtensions;
extern XORG_EXPORT char *SeatId;
extern XORG_EXPORT char *ConnectionInfo;
extern XORG_EXPORT sig_atomic_t inSignalContext;

#ifdef DPMSExtension
extern XORG_EXPORT CARD32 DPMSStandbyTime;
extern XORG_EXPORT CARD32 DPMSSuspendTime;
extern XORG_EXPORT CARD32 DPMSOffTime;
extern XORG_EXPORT CARD16 DPMSPowerLevel;
extern XORG_EXPORT Bool DPMSEnabled;
extern XORG_EXPORT Bool DPMSDisabledSwitch;
extern XORG_EXPORT Bool DPMSCapableFlag;
#endif

#ifdef PANORAMIX
extern XORG_EXPORT Bool PanoramiXExtensionDisabledHack;
#endif

#ifdef XSELINUX
#define SELINUX_MODE_DEFAULT    0
#define SELINUX_MODE_DISABLED   1
#define SELINUX_MODE_PERMISSIVE 2
#define SELINUX_MODE_ENFORCING  3
extern XORG_EXPORT int selinuxEnforcingState;
#endif

#endif                          /* !_XSERV_GLOBAL_H_ */
