/*

Copyright 1987, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/

#ifndef OPAQUE_H
#define OPAQUE_H

#include <X11/Xmd.h>

#include "globals.h"

extern XORG_EXPORT const char *defaultTextFont;
extern XORG_EXPORT const char *defaultCursorFont;
extern XORG_EXPORT int MaxClients;
extern XORG_EXPORT int LimitClients;
extern XORG_EXPORT volatile char isItTimeToYield;
extern XORG_EXPORT volatile char dispatchException;

/* bit values for dispatchException */
#define DE_RESET     1
#define DE_TERMINATE 2
#define DE_PRIORITYCHANGE 4     /* set when a client's priority changes */

extern XORG_EXPORT CARD32 TimeOutValue;
extern XORG_EXPORT int ScreenSaverBlanking;
extern XORG_EXPORT int ScreenSaverAllowExposures;
extern XORG_EXPORT int defaultScreenSaverBlanking;
extern XORG_EXPORT int defaultScreenSaverAllowExposures;
extern XORG_EXPORT const char *display;
extern XORG_EXPORT int displayfd;
extern XORG_EXPORT Bool explicit_display;

extern XORG_EXPORT int defaultBackingStore;
extern XORG_EXPORT Bool disableBackingStore;
extern XORG_EXPORT Bool enableBackingStore;
extern XORG_EXPORT Bool enableIndirectGLX;
extern XORG_EXPORT Bool PartialNetwork;
extern XORG_EXPORT Bool RunFromSigStopParent;

#ifdef RLIMIT_DATA
extern XORG_EXPORT int limitDataSpace;
#endif
#ifdef RLIMIT_STACK
extern XORG_EXPORT int limitStackSpace;
#endif
#ifdef RLIMIT_NOFILE
extern XORG_EXPORT int limitNoFile;
#endif
extern XORG_EXPORT Bool defeatAccessControl;
extern XORG_EXPORT long maxBigRequestSize;
extern XORG_EXPORT Bool party_like_its_1989;
extern XORG_EXPORT Bool whiteRoot;
extern XORG_EXPORT Bool bgNoneRoot;

extern XORG_EXPORT Bool CoreDump;
extern XORG_EXPORT Bool NoListenAll;

#endif                          /* OPAQUE_H */
