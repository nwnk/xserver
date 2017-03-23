/***********************************************************

Copyright 1987, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Digital not be
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

#ifndef DIX_H
#define DIX_H

#include "callback.h"
#include "gc.h"
#include "window.h"
#include "input.h"
#include "cursor.h"
#include "geext.h"
#include "events.h"
#include <X11/extensions/XI.h>

#define EARLIER -1
#define SAMETIME 0
#define LATER 1

#define NullClient ((ClientPtr) 0)
#define REQUEST(type) \
	type *stuff = (type *)client->requestBuffer

#define ARRAY_SIZE(a)  (sizeof((a)) / sizeof((a)[0]))

#define REQUEST_SIZE_MATCH(req)\
    if ((sizeof(req) >> 2) != client->req_len)\
         return(BadLength)

#define REQUEST_AT_LEAST_SIZE(req) \
    if ((sizeof(req) >> 2) > client->req_len )\
         return(BadLength)

#define REQUEST_AT_LEAST_EXTRA_SIZE(req, extra)  \
    if (((sizeof(req) + ((uint64_t) extra)) >> 2) > client->req_len ) \
         return(BadLength)

#define REQUEST_FIXED_SIZE(req, n)\
    if (((sizeof(req) >> 2) > client->req_len) || \
        (((n) >> 2) >= client->req_len) ||                              \
        ((((uint64_t) sizeof(req) + (n) + 3) >> 2) != (uint64_t) client->req_len))  \
         return(BadLength)

#define LEGAL_NEW_RESOURCE(id,client)\
    if (!LegalNewID(id,client)) \
    {\
	client->errorValue = id;\
        return BadIDChoice;\
    }

#define VALIDATE_DRAWABLE_AND_GC(drawID, pDraw, mode)\
    {\
	int tmprc = dixLookupDrawable(&(pDraw), drawID, client, M_ANY, mode);\
	if (tmprc != Success)\
	    return tmprc;\
	tmprc = dixLookupGC(&(pGC), stuff->gc, client, DixUseAccess);\
	if (tmprc != Success)\
	    return tmprc;\
	if ((pGC->depth != pDraw->depth) || (pGC->pScreen != pDraw->pScreen))\
	    return BadMatch;\
    }\
    if (pGC->serialNumber != pDraw->serialNumber)\
	ValidateGC(pDraw, pGC);

#define WriteReplyToClient(pClient, size, pReply) { \
   if ((pClient)->swapped) \
      (*ReplySwapVector[((xReq *)(pClient)->requestBuffer)->reqType]) \
           (pClient, (int)(size), pReply); \
   else WriteToClient(pClient, (int)(size), (pReply)); }

#define WriteSwappedDataToClient(pClient, size, pbuf) \
   if ((pClient)->swapped) \
      (*(pClient)->pSwapReplyFunc)(pClient, (int)(size), pbuf); \
   else WriteToClient(pClient, (int)(size), (pbuf));

typedef struct _TimeStamp *TimeStampPtr;

#ifndef _XTYPEDEF_CLIENTPTR
typedef struct _Client *ClientPtr;      /* also in misc.h */

#define _XTYPEDEF_CLIENTPTR
#endif

typedef struct _WorkQueue *WorkQueuePtr;

extern XORG_EXPORT ClientPtr clients[MAXCLIENTS];
extern XORG_EXPORT ClientPtr serverClient;
extern XORG_EXPORT int currentMaxClients;
extern XORG_EXPORT char dispatchExceptionAtReset;

typedef int HWEventQueueType;
typedef HWEventQueueType *HWEventQueuePtr;

extern XORG_EXPORT HWEventQueuePtr checkForInput[2];

static inline _X_NOTSAN Bool
InputCheckPending(void)
{
    return (*checkForInput[0] != *checkForInput[1]);
}

typedef struct _TimeStamp {
    CARD32 months;              /* really ~49.7 days */
    CARD32 milliseconds;
} TimeStamp;

/* dispatch.c */

extern XORG_EXPORT void SetInputCheck(HWEventQueuePtr /*c0 */ ,
                                    HWEventQueuePtr /*c1 */ );

extern XORG_EXPORT void CloseDownClient(ClientPtr /*client */ );

extern XORG_EXPORT void UpdateCurrentTime(void);

extern XORG_EXPORT void UpdateCurrentTimeIf(void);

extern XORG_EXPORT int dixDestroyPixmap(void *value,
                                      XID pid);

extern XORG_EXPORT void InitClient(ClientPtr client,
                                 int i,
                                 void *ospriv);

extern XORG_EXPORT ClientPtr NextAvailableClient(void *ospriv);

extern XORG_EXPORT void SendErrorToClient(ClientPtr /*client */ ,
                                        unsigned int /*majorCode */ ,
                                        unsigned int /*minorCode */ ,
                                        XID /*resId */ ,
                                        int /*errorCode */ );

extern XORG_EXPORT void MarkClientException(ClientPtr /*client */ );

extern _X_HIDDEN Bool CreateConnectionBlock(void);

/* dixutils.c */

extern XORG_EXPORT int CompareISOLatin1Lowered(const unsigned char * /*a */ ,
                                             int alen,
                                             const unsigned char * /*b */ ,
                                             int blen);

extern XORG_EXPORT int dixLookupWindow(WindowPtr *result,
                                     XID id,
                                     ClientPtr client, Mask access_mode);

extern XORG_EXPORT int dixLookupDrawable(DrawablePtr *result,
                                       XID id,
                                       ClientPtr client,
                                       Mask type_mask, Mask access_mode);

extern XORG_EXPORT int dixLookupGC(GCPtr *result,
                                 XID id, ClientPtr client, Mask access_mode);

extern XORG_EXPORT int dixLookupFontable(FontPtr *result,
                                       XID id,
                                       ClientPtr client, Mask access_mode);

extern XORG_EXPORT int dixLookupClient(ClientPtr *result,
                                     XID id,
                                     ClientPtr client, Mask access_mode);

extern XORG_EXPORT void NoopDDA(void);

extern XORG_EXPORT int AlterSaveSetForClient(ClientPtr /*client */ ,
                                           WindowPtr /*pWin */ ,
                                           unsigned /*mode */ ,
                                           Bool /*toRoot */ ,
                                           Bool /*map */ );

extern XORG_EXPORT void DeleteWindowFromAnySaveSet(WindowPtr /*pWin */ );

extern XORG_EXPORT void BlockHandler(void *timeout);

extern XORG_EXPORT void WakeupHandler(int result);

void
EnableLimitedSchedulingLatency(void);

void
DisableLimitedSchedulingLatency(void);

typedef void (*ServerBlockHandlerProcPtr) (void *blockData,
                                           void *timeout);

typedef void (*ServerWakeupHandlerProcPtr) (void *blockData,
                                            int result);

extern XORG_EXPORT Bool RegisterBlockAndWakeupHandlers(ServerBlockHandlerProcPtr blockHandler,
                                                     ServerWakeupHandlerProcPtr wakeupHandler,
                                                     void *blockData);

extern XORG_EXPORT void RemoveBlockAndWakeupHandlers(ServerBlockHandlerProcPtr blockHandler,
                                                   ServerWakeupHandlerProcPtr wakeupHandler,
                                                   void *blockData);

extern XORG_EXPORT void InitBlockAndWakeupHandlers(void);

extern XORG_EXPORT void ProcessWorkQueue(void);

extern XORG_EXPORT void ProcessWorkQueueZombies(void);

extern XORG_EXPORT Bool QueueWorkProc(Bool (*function)(ClientPtr clientUnused,
                                                     void *closure),
                                    ClientPtr client,
                                    void *closure);

typedef Bool (*ClientSleepProcPtr) (ClientPtr client,
                                    void *closure);

extern XORG_EXPORT Bool ClientSleep(ClientPtr client,
                                  ClientSleepProcPtr function,
                                  void *closure);

#ifndef ___CLIENTSIGNAL_DEFINED___
#define ___CLIENTSIGNAL_DEFINED___
extern XORG_EXPORT Bool ClientSignal(ClientPtr /*client */ );
#endif                          /* ___CLIENTSIGNAL_DEFINED___ */

#ifndef ___CLIENTSIGNALALL_DEFINED___
#define ___CLIENTSIGNALALL_DEFINED___
#define CLIENT_SIGNAL_ANY ((void *)-1)
extern XORG_EXPORT int ClientSignalAll(ClientPtr /*client*/,
                                     ClientSleepProcPtr /*function*/,
                                     void * /*closure*/);
#endif                          /* ___CLIENTSIGNALALL_DEFINED___ */

extern XORG_EXPORT void ClientWakeup(ClientPtr /*client */ );

extern XORG_EXPORT Bool ClientIsAsleep(ClientPtr /*client */ );

extern XORG_EXPORT void SendGraphicsExpose(ClientPtr /*client */ ,
                                         RegionPtr /*pRgn */ ,
                                         XID /*drawable */ ,
                                         int /*major */ ,
                                         int  /*minor */);

/* atom.c */

extern XORG_EXPORT Atom MakeAtom(const char * /*string */ ,
                               unsigned /*len */ ,
                               Bool /*makeit */ );

extern XORG_EXPORT Bool ValidAtom(Atom /*atom */ );

extern XORG_EXPORT const char *NameForAtom(Atom /*atom */ );

extern XORG_EXPORT void
AtomError(void)
    _X_NORETURN;

extern XORG_EXPORT void
FreeAllAtoms(void);

extern XORG_EXPORT void
InitAtoms(void);

/* main.c */

extern XORG_EXPORT void
SetVendorRelease(int release);

extern XORG_EXPORT void
SetVendorString(const char *string);

int
dix_main(int argc, char *argv[], char *envp[]);

/* events.c */

extern void
SetMaskForEvent(int /* deviceid */ ,
                Mask /* mask */ ,
                int /* event */ );

extern XORG_EXPORT void
ConfineToShape(DeviceIntPtr /* pDev */ ,
               RegionPtr /* shape */ ,
               int * /* px */ ,
               int * /* py */ );

extern XORG_EXPORT Bool
IsParent(WindowPtr /* maybeparent */ ,
         WindowPtr /* child */ );

extern XORG_EXPORT WindowPtr
GetCurrentRootWindow(DeviceIntPtr pDev);

extern XORG_EXPORT WindowPtr
GetSpriteWindow(DeviceIntPtr pDev);

extern XORG_EXPORT void
NoticeTime(const DeviceIntPtr dev,
           TimeStamp time);
extern XORG_EXPORT void
NoticeEventTime(InternalEvent *ev,
                DeviceIntPtr dev);
extern XORG_EXPORT TimeStamp
LastEventTime(int deviceid);
extern XORG_EXPORT Bool
LastEventTimeWasReset(int deviceid);
extern XORG_EXPORT void
LastEventTimeToggleResetFlag(int deviceid, Bool state);
extern XORG_EXPORT void
LastEventTimeToggleResetAll(Bool state);

extern void
EnqueueEvent(InternalEvent * /* ev */ ,
             DeviceIntPtr /* device */ );
extern void
PlayReleasedEvents(void);

extern void
ActivatePointerGrab(DeviceIntPtr /* mouse */ ,
                    GrabPtr /* grab */ ,
                    TimeStamp /* time */ ,
                    Bool /* autoGrab */ );

extern void
DeactivatePointerGrab(DeviceIntPtr /* mouse */ );

extern void
ActivateKeyboardGrab(DeviceIntPtr /* keybd */ ,
                     GrabPtr /* grab */ ,
                     TimeStamp /* time */ ,
                     Bool /* passive */ );

extern void
DeactivateKeyboardGrab(DeviceIntPtr /* keybd */ );

extern BOOL
ActivateFocusInGrab(DeviceIntPtr /* dev */ ,
                    WindowPtr /* old */ ,
                    WindowPtr /* win */ );

extern void
AllowSome(ClientPtr /* client */ ,
          TimeStamp /* time */ ,
          DeviceIntPtr /* thisDev */ ,
          int /* newState */ );

extern void
ReleaseActiveGrabs(ClientPtr client);

extern GrabPtr
CheckPassiveGrabsOnWindow(WindowPtr /* pWin */ ,
                          DeviceIntPtr /* device */ ,
                          InternalEvent * /* event */ ,
                          BOOL /* checkCore */ ,
                          BOOL /* activate */ );

extern XORG_EXPORT int
DeliverEventsToWindow(DeviceIntPtr /* pWin */ ,
                      WindowPtr /* pWin */ ,
                      xEventPtr /* pEvents */ ,
                      int /* count */ ,
                      Mask /* filter */ ,
                      GrabPtr /* grab */ );

extern XORG_EXPORT void
DeliverRawEvent(RawDeviceEvent * /* ev */ ,
                DeviceIntPtr    /* dev */
    );

extern int
DeliverDeviceEvents(WindowPtr /* pWin */ ,
                    InternalEvent * /* event */ ,
                    GrabPtr /* grab */ ,
                    WindowPtr /* stopAt */ ,
                    DeviceIntPtr /* dev */ );

extern int
DeliverOneGrabbedEvent(InternalEvent * /* event */ ,
                       DeviceIntPtr /* dev */ ,
                       enum InputLevel /* level */ );

extern void
DeliverTouchEvents(DeviceIntPtr /* dev */ ,
                   TouchPointInfoPtr /* ti */ ,
                   InternalEvent * /* ev */ ,
                   XID /* resource */ );

extern void
InitializeSprite(DeviceIntPtr /* pDev */ ,
                 WindowPtr /* pWin */ );
extern void
FreeSprite(DeviceIntPtr pDev);

extern void
UpdateSpriteForScreen(DeviceIntPtr /* pDev */ ,
                      ScreenPtr /* pScreen */ );

extern XORG_EXPORT void
WindowHasNewCursor(WindowPtr /* pWin */ );

extern Bool
CheckDeviceGrabs(DeviceIntPtr /* device */ ,
                 DeviceEvent * /* event */ ,
                 WindowPtr /* ancestor */ );

extern void
DeliverFocusedEvent(DeviceIntPtr /* keybd */ ,
                    InternalEvent * /* event */ ,
                    WindowPtr /* window */ );

extern int
DeliverGrabbedEvent(InternalEvent * /* event */ ,
                    DeviceIntPtr /* thisDev */ ,
                    Bool /* deactivateGrab */ );

extern void
FixKeyState(DeviceEvent * /* event */ ,
            DeviceIntPtr /* keybd */ );

extern void
RecalculateDeliverableEvents(WindowPtr /* pWin */ );

extern XORG_EXPORT int
OtherClientGone(void *value,
                XID id);

extern void
DoFocusEvents(DeviceIntPtr /* dev */ ,
              WindowPtr /* fromWin */ ,
              WindowPtr /* toWin */ ,
              int /* mode */ );

extern int
SetInputFocus(ClientPtr /* client */ ,
              DeviceIntPtr /* dev */ ,
              Window /* focusID */ ,
              CARD8 /* revertTo */ ,
              Time /* ctime */ ,
              Bool /* followOK */ );

extern int
GrabDevice(ClientPtr /* client */ ,
           DeviceIntPtr /* dev */ ,
           unsigned /* this_mode */ ,
           unsigned /* other_mode */ ,
           Window /* grabWindow */ ,
           unsigned /* ownerEvents */ ,
           Time /* ctime */ ,
           GrabMask * /* mask */ ,
           int /* grabtype */ ,
           Cursor /* curs */ ,
           Window /* confineToWin */ ,
           CARD8 * /* status */ );

extern void
InitEvents(void);

extern void
CloseDownEvents(void);

extern void
DeleteWindowFromAnyEvents(WindowPtr /* pWin */ ,
                          Bool /* freeResources */ );

extern Mask
EventMaskForClient(WindowPtr /* pWin */ ,
                   ClientPtr /* client */ );

extern XORG_EXPORT int
DeliverEvents(WindowPtr /*pWin */ ,
              xEventPtr /*xE */ ,
              int /*count */ ,
              WindowPtr /*otherParent */ );

extern Bool
CheckMotion(DeviceEvent * /* ev */ ,
            DeviceIntPtr /* pDev */ );

extern XORG_EXPORT void
WriteEventsToClient(ClientPtr /*pClient */ ,
                    int /*count */ ,
                    xEventPtr /*events */ );

extern XORG_EXPORT int
TryClientEvents(ClientPtr /*client */ ,
                DeviceIntPtr /* device */ ,
                xEventPtr /*pEvents */ ,
                int /*count */ ,
                Mask /*mask */ ,
                Mask /*filter */ ,
                GrabPtr /*grab */ );

extern XORG_EXPORT void
WindowsRestructured(void);

extern int
SetClientPointer(ClientPtr /* client */ ,
                 DeviceIntPtr /* device */ );

extern XORG_EXPORT DeviceIntPtr
PickPointer(ClientPtr /* client */ );

extern XORG_EXPORT DeviceIntPtr
PickKeyboard(ClientPtr /* client */ );

extern Bool
IsInterferingGrab(ClientPtr /* client */ ,
                  DeviceIntPtr /* dev */ ,
                  xEvent * /* events */ );

#ifdef PANORAMIX
extern XORG_EXPORT void
ReinitializeRootWindow(WindowPtr win, int xoff, int yoff);
#endif

#ifdef RANDR
extern XORG_EXPORT void
ScreenRestructured(ScreenPtr pScreen);
#endif

#ifndef HAVE_FFS
extern XORG_EXPORT int
ffs(int i);
#endif

/*
 *  ServerGrabCallback stuff
 */

extern XORG_EXPORT CallbackListPtr ServerGrabCallback;

typedef enum { SERVER_GRABBED, SERVER_UNGRABBED,
    CLIENT_PERVIOUS, CLIENT_IMPERVIOUS
} ServerGrabState;

typedef struct {
    ClientPtr client;
    ServerGrabState grabstate;
} ServerGrabInfoRec;

/*
 *  EventCallback stuff
 */

extern XORG_EXPORT CallbackListPtr EventCallback;

typedef struct {
    ClientPtr client;
    xEventPtr events;
    int count;
} EventInfoRec;

/*
 *  DeviceEventCallback stuff
 */

extern XORG_EXPORT CallbackListPtr DeviceEventCallback;

typedef struct {
    InternalEvent *event;
    DeviceIntPtr device;
} DeviceEventInfoRec;

extern XORG_EXPORT CallbackListPtr RootWindowFinalizeCallback;

extern int
XItoCoreType(int xi_type);
extern Bool
DevHasCursor(DeviceIntPtr pDev);
extern XORG_EXPORT Bool
IsPointerDevice(DeviceIntPtr dev);
extern XORG_EXPORT Bool
IsKeyboardDevice(DeviceIntPtr dev);
extern Bool
IsPointerEvent(InternalEvent *event);
extern Bool
IsTouchEvent(InternalEvent *event);
extern XORG_EXPORT Bool
IsMaster(DeviceIntPtr dev);
extern XORG_EXPORT Bool
IsFloating(DeviceIntPtr dev);

extern _X_HIDDEN void
CopyKeyClass(DeviceIntPtr device, DeviceIntPtr master);
extern _X_HIDDEN int
CorePointerProc(DeviceIntPtr dev, int what);
extern _X_HIDDEN int
CoreKeyboardProc(DeviceIntPtr dev, int what);

extern XORG_EXPORT void *lastGLContext;

#endif                          /* DIX_H */
