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

#ifndef OS_H
#define OS_H

#include "misc.h"
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#ifdef MONOTONIC_CLOCK
#include <time.h>
#endif

#define SCREEN_SAVER_ON   0
#define SCREEN_SAVER_OFF  1
#define SCREEN_SAVER_FORCER 2
#define SCREEN_SAVER_CYCLE  3

#ifndef MAX_REQUEST_SIZE
#define MAX_REQUEST_SIZE 65535
#endif
#ifndef MAX_BIG_REQUEST_SIZE
#define MAX_BIG_REQUEST_SIZE 4194303
#endif

typedef struct _FontPathRec *FontPathPtr;
typedef struct _NewClientRec *NewClientPtr;

#ifndef xnfalloc
#define xnfalloc(size) XNFalloc((unsigned long)(size))
#define xnfcalloc(_num, _size) XNFcallocarray((_num), (_size))
#define xnfrealloc(ptr, size) XNFrealloc((void *)(ptr), (unsigned long)(size))

#define xstrdup(s) Xstrdup(s)
#define xnfstrdup(s) XNFstrdup(s)

#define xallocarray(num, size) reallocarray(NULL, (num), (size))
#define xnfallocarray(num, size) XNFreallocarray(NULL, (num), (size))
#define xnfreallocarray(ptr, num, size) XNFreallocarray((ptr), (num), (size))
#endif

#include <stdio.h>
#include <stdarg.h>

#ifdef DDXBEFORERESET
extern void ddxBeforeReset(void);
#endif

#ifdef DDXOSVERRORF
extern XORG_EXPORT void (*OsVendorVErrorFProc) (const char *,
                                              va_list args)
_X_ATTRIBUTE_PRINTF(1, 0);
#endif

extern XORG_EXPORT Bool WaitForSomething(Bool clients_are_ready);

extern XORG_EXPORT int ReadRequestFromClient(ClientPtr /*client */ );

#if XTRANS_SEND_FDS
extern XORG_EXPORT int ReadFdFromClient(ClientPtr client);

extern XORG_EXPORT int WriteFdToClient(ClientPtr client, int fd, Bool do_close);
#endif

extern XORG_EXPORT Bool InsertFakeRequest(ClientPtr /*client */ ,
                                        char * /*data */ ,
                                        int /*count */ );

extern XORG_EXPORT void ResetCurrentRequest(ClientPtr /*client */ );

extern XORG_EXPORT void FlushAllOutput(void);

extern XORG_EXPORT void FlushIfCriticalOutputPending(void);

extern XORG_EXPORT void SetCriticalOutputPending(void);

extern XORG_EXPORT int WriteToClient(ClientPtr /*who */ , int /*count */ ,
                                   const void * /*buf */ );

extern XORG_EXPORT void ResetOsBuffers(void);

extern XORG_EXPORT void InitConnectionLimits(void);

extern XORG_EXPORT void NotifyParentProcess(void);

extern XORG_EXPORT void CreateWellKnownSockets(void);

extern XORG_EXPORT void ResetWellKnownSockets(void);

extern XORG_EXPORT void CloseWellKnownConnections(void);

extern XORG_EXPORT XID AuthorizationIDOfClient(ClientPtr /*client */ );

extern XORG_EXPORT const char *ClientAuthorized(ClientPtr /*client */ ,
                                              unsigned int /*proto_n */ ,
                                              char * /*auth_proto */ ,
                                              unsigned int /*string_n */ ,
                                              char * /*auth_string */ );

extern XORG_EXPORT void CloseDownConnection(ClientPtr /*client */ );

typedef void (*NotifyFdProcPtr)(int fd, int ready, void *data);

#define X_NOTIFY_NONE   0x0
#define X_NOTIFY_READ   0x1
#define X_NOTIFY_WRITE  0x2
#define X_NOTIFY_ERROR  0x4     /* don't need to select for, always reported */

extern XORG_EXPORT Bool SetNotifyFd(int fd, NotifyFdProcPtr notify_fd, int mask, void *data);

static inline void RemoveNotifyFd(int fd)
{
    (void) SetNotifyFd(fd, NULL, X_NOTIFY_NONE, NULL);
}

extern XORG_EXPORT int OnlyListenToOneClient(ClientPtr /*client */ );

extern XORG_EXPORT void ListenToAllClients(void);

extern XORG_EXPORT void IgnoreClient(ClientPtr /*client */ );

extern XORG_EXPORT void AttendClient(ClientPtr /*client */ );

extern XORG_EXPORT void MakeClientGrabImpervious(ClientPtr /*client */ );

extern XORG_EXPORT void MakeClientGrabPervious(ClientPtr /*client */ );

extern XORG_EXPORT void ListenOnOpenFD(int /* fd */ , int /* noxauth */ );

extern XORG_EXPORT Bool AddClientOnOpenFD(int /* fd */ );

#ifdef MONOTONIC_CLOCK
extern void ForceClockId(clockid_t /* forced_clockid */);
#endif

extern XORG_EXPORT CARD32 GetTimeInMillis(void);
extern XORG_EXPORT CARD64 GetTimeInMicros(void);

extern XORG_EXPORT void AdjustWaitForDelay(void *waitTime, int newdelay);

typedef struct _OsTimerRec *OsTimerPtr;

typedef CARD32 (*OsTimerCallback) (OsTimerPtr timer,
                                   CARD32 time,
                                   void *arg);

extern XORG_EXPORT void TimerInit(void);

extern XORG_EXPORT Bool TimerForce(OsTimerPtr /* timer */ );

#define TimerAbsolute (1<<0)
#define TimerForceOld (1<<1)

extern XORG_EXPORT OsTimerPtr TimerSet(OsTimerPtr timer,
                                     int flags,
                                     CARD32 millis,
                                     OsTimerCallback func,
                                     void *arg);

extern XORG_EXPORT void TimerCheck(void);
extern XORG_EXPORT void TimerCancel(OsTimerPtr /* pTimer */ );
extern XORG_EXPORT void TimerFree(OsTimerPtr /* pTimer */ );

extern XORG_EXPORT void SetScreenSaverTimer(void);
extern XORG_EXPORT void FreeScreenSaverTimer(void);

extern XORG_EXPORT void AutoResetServer(int /*sig */ );

extern XORG_EXPORT void GiveUp(int /*sig */ );

extern XORG_EXPORT void UseMsg(void);

extern XORG_EXPORT void ProcessCommandLine(int /*argc */ , char * /*argv */ []);

extern XORG_EXPORT int set_font_authorizations(char **authorizations,
                                             int *authlen,
                                             void *client);

/*
 * This function malloc(3)s buffer, terminating the server if there is not
 * enough memory.
 */
extern XORG_EXPORT void *
XNFalloc(unsigned long /*amount */ );

/*
 * This function calloc(3)s buffer, terminating the server if there is not
 * enough memory.
 */
extern XORG_EXPORT void *
XNFcalloc(unsigned long /*amount */ ) _X_DEPRECATED;

/*
 * This function calloc(3)s buffer, terminating the server if there is not
 * enough memory or the arguments overflow when multiplied
 */
extern XORG_EXPORT void *
XNFcallocarray(size_t nmemb, size_t size);

/*
 * This function realloc(3)s passed buffer, terminating the server if there is
 * not enough memory.
 */
extern XORG_EXPORT void *
XNFrealloc(void * /*ptr */ , unsigned long /*amount */ );

/*
 * This function reallocarray(3)s passed buffer, terminating the server if
 * there is not enough memory or the arguments overflow when multiplied.
 */
extern XORG_EXPORT void *
XNFreallocarray(void *ptr, size_t nmemb, size_t size);

/*
 * This function strdup(3)s passed string. The only difference from the library
 * function that it is safe to pass NULL, as NULL will be returned.
 */
extern XORG_EXPORT char *
Xstrdup(const char *s);

/*
 * This function strdup(3)s passed string, terminating the server if there is
 * not enough memory. If NULL is passed to this function, NULL is returned.
 */
extern XORG_EXPORT char *
XNFstrdup(const char *s);

/* Include new X*asprintf API */
#include "Xprintf.h"

/* Older api deprecated in favor of the asprintf versions */
extern XORG_EXPORT char *
Xprintf(const char *fmt, ...)
_X_ATTRIBUTE_PRINTF(1, 2)
    _X_DEPRECATED;
extern XORG_EXPORT char *
Xvprintf(const char *fmt, va_list va)
_X_ATTRIBUTE_PRINTF(1, 0)
    _X_DEPRECATED;
extern XORG_EXPORT char *
XNFprintf(const char *fmt, ...)
_X_ATTRIBUTE_PRINTF(1, 2)
    _X_DEPRECATED;
extern XORG_EXPORT char *
XNFvprintf(const char *fmt, va_list va)
_X_ATTRIBUTE_PRINTF(1, 0)
    _X_DEPRECATED;

typedef void (*OsSigHandlerPtr) (int /* sig */ );
typedef int (*OsSigWrapperPtr) (int /* sig */ );

extern XORG_EXPORT OsSigHandlerPtr
OsSignal(int /* sig */ , OsSigHandlerPtr /* handler */ );
extern XORG_EXPORT OsSigWrapperPtr
OsRegisterSigWrapper(OsSigWrapperPtr newWrap);

extern XORG_EXPORT int auditTrailLevel;

extern XORG_EXPORT void
LockServer(void);
extern XORG_EXPORT void
UnlockServer(void);

extern XORG_EXPORT int
OsLookupColor(int /*screen */ ,
              char * /*name */ ,
              unsigned /*len */ ,
              unsigned short * /*pred */ ,
              unsigned short * /*pgreen */ ,
              unsigned short * /*pblue */ );

extern XORG_EXPORT void
OsInit(void);

extern XORG_EXPORT void
OsCleanup(Bool);

extern XORG_EXPORT void
OsVendorFatalError(const char *f, va_list args)
_X_ATTRIBUTE_PRINTF(1, 0);

extern XORG_EXPORT void
OsVendorInit(void);

extern XORG_EXPORT void
OsBlockSignals(void);

extern XORG_EXPORT void
OsReleaseSignals(void);

extern void
OsResetSignals(void);

extern XORG_EXPORT void
OsAbort(void)
    _X_NORETURN;

#if !defined(WIN32)
extern XORG_EXPORT int
System(const char *);
extern XORG_EXPORT void *
Popen(const char *, const char *);
extern XORG_EXPORT int
Pclose(void *);
extern XORG_EXPORT void *
Fopen(const char *, const char *);
extern XORG_EXPORT int
Fclose(void *);
#else

extern const char *
Win32TempDir(void);

extern int
System(const char *cmdline);

#define Fopen(a,b) fopen(a,b)
#define Fclose(a) fclose(a)
#endif

extern XORG_EXPORT void
CheckUserParameters(int argc, char **argv, char **envp);
extern XORG_EXPORT void
CheckUserAuthorization(void);

extern XORG_EXPORT int
AddHost(ClientPtr /*client */ ,
        int /*family */ ,
        unsigned /*length */ ,
        const void * /*pAddr */ );

extern XORG_EXPORT Bool
ForEachHostInFamily(int family,
                    Bool (*func)(
                                           unsigned char *addr,
                                           short len,
                                           void *closure),
                    void *closure);

extern XORG_EXPORT int
RemoveHost(ClientPtr client,
           int family,
           unsigned length,
           void *pAddr);

extern XORG_EXPORT int
GetHosts(void ** /*data */ ,
         int * /*pnHosts */ ,
         int * /*pLen */ ,
         BOOL * /*pEnabled */ );

typedef struct sockaddr *sockaddrPtr;

extern XORG_EXPORT int
InvalidHost(sockaddrPtr /*saddr */ , int /*len */ , ClientPtr client);

#define LCC_UID_SET	(1 << 0)
#define LCC_GID_SET	(1 << 1)
#define LCC_PID_SET	(1 << 2)
#define LCC_ZID_SET	(1 << 3)

typedef struct {
    int fieldsSet;              /* Bit mask of fields set */
    int euid;                   /* Effective uid */
    int egid;                   /* Primary effective group id */
    int nSuppGids;              /* Number of supplementary group ids */
    int *pSuppGids;             /* Array of supplementary group ids */
    int pid;                    /* Process id */
    int zoneid;                 /* Only set on Solaris 10 & later */
} LocalClientCredRec;

extern XORG_EXPORT int
GetLocalClientCreds(ClientPtr, LocalClientCredRec **);

extern XORG_EXPORT void
FreeLocalClientCreds(LocalClientCredRec *);

extern XORG_EXPORT int
ChangeAccessControl(ClientPtr /*client */ , int /*fEnabled */ );

extern XORG_EXPORT int
GetClientFd(ClientPtr);

extern XORG_EXPORT Bool
ClientIsLocal(ClientPtr client);

extern XORG_EXPORT int
GetAccessControl(void);

extern XORG_EXPORT void
AddLocalHosts(void);

extern XORG_EXPORT void
ResetHosts(const char *display);

extern XORG_EXPORT void
EnableLocalAccess(void);

extern XORG_EXPORT void
DisableLocalAccess(void);

extern XORG_EXPORT void
EnableLocalHost(void);

extern XORG_EXPORT void
DisableLocalHost(void);

#ifndef NO_LOCAL_CLIENT_CRED
extern XORG_EXPORT void
EnableLocalUser(void);

extern XORG_EXPORT void
DisableLocalUser(void);

extern XORG_EXPORT void
LocalAccessScopeUser(void);
#endif

extern XORG_EXPORT void
AccessUsingXdmcp(void);

extern XORG_EXPORT void
DefineSelf(int /*fd */ );

#if XDMCP
extern XORG_EXPORT void
AugmentSelf(void *from, int len);

extern XORG_EXPORT void
RegisterAuthorizations(void);
#endif

extern XORG_EXPORT void
InitAuthorization(const char * /*filename */ );

/* extern int LoadAuthorization(void); */

extern XORG_EXPORT int
AuthorizationFromID(XID id,
                    unsigned short *name_lenp,
                    const char **namep,
                    unsigned short *data_lenp, char **datap);

extern XORG_EXPORT XID
CheckAuthorization(unsigned int /*namelength */ ,
                   const char * /*name */ ,
                   unsigned int /*datalength */ ,
                   const char * /*data */ ,
                   ClientPtr /*client */ ,
                   const char **        /*reason */
    );

extern XORG_EXPORT void
ResetAuthorization(void);

extern XORG_EXPORT int
RemoveAuthorization(unsigned short name_length,
                    const char *name,
                    unsigned short data_length, const char *data);

extern XORG_EXPORT int
AddAuthorization(unsigned int /*name_length */ ,
                 const char * /*name */ ,
                 unsigned int /*data_length */ ,
                 char * /*data */ );

#ifdef XCSECURITY
extern XORG_EXPORT XID
GenerateAuthorization(unsigned int /* name_length */ ,
                      const char * /* name */ ,
                      unsigned int /* data_length */ ,
                      const char * /* data */ ,
                      unsigned int * /* data_length_return */ ,
                      char ** /* data_return */ );
#endif

extern XORG_EXPORT int
ddxProcessArgument(int /*argc */ , char * /*argv */ [], int /*i */ );

extern XORG_EXPORT void
ddxUseMsg(void);

/* stuff for ReplyCallback */
extern XORG_EXPORT CallbackListPtr ReplyCallback;
typedef struct {
    ClientPtr client;
    const void *replyData;
    unsigned long dataLenBytes; /* actual bytes from replyData + pad bytes */
    unsigned long bytesRemaining;
    Bool startOfReply;
    unsigned long padBytes;     /* pad bytes from zeroed array */
} ReplyInfoRec;

/* stuff for FlushCallback */
extern XORG_EXPORT CallbackListPtr FlushCallback;

enum ExitCode {
    EXIT_NO_ERROR = 0,
    EXIT_ERR_ABORT = 1,
    EXIT_ERR_CONFIGURE = 2,
    EXIT_ERR_DRIVERS = 3,
};

extern XORG_EXPORT void
AbortDDX(enum ExitCode error);
extern XORG_EXPORT void
ddxGiveUp(enum ExitCode error);
extern XORG_EXPORT int
TimeSinceLastInputEvent(void);

/* Function fallbacks provided by AC_REPLACE_FUNCS in configure.ac */

#ifndef HAVE_REALLOCARRAY
#define reallocarray xreallocarray
extern XORG_EXPORT void *
reallocarray(void *optr, size_t nmemb, size_t size);
#endif

#ifndef HAVE_STRCASECMP
#define strcasecmp xstrcasecmp
extern XORG_EXPORT int
xstrcasecmp(const char *s1, const char *s2);
#endif

#ifndef HAVE_STRNCASECMP
#define strncasecmp xstrncasecmp
extern XORG_EXPORT int
xstrncasecmp(const char *s1, const char *s2, size_t n);
#endif

#ifndef HAVE_STRCASESTR
#define strcasestr xstrcasestr
extern XORG_EXPORT char *
xstrcasestr(const char *s, const char *find);
#endif

#ifndef HAVE_STRLCPY
extern XORG_EXPORT size_t
strlcpy(char *dst, const char *src, size_t siz);
extern XORG_EXPORT size_t
strlcat(char *dst, const char *src, size_t siz);
#endif

#ifndef HAVE_STRNDUP
extern XORG_EXPORT char *
strndup(const char *str, size_t n);
#endif

#ifndef HAVE_TIMINGSAFE_MEMCMP
extern XORG_EXPORT int
timingsafe_memcmp(const void *b1, const void *b2, size_t len);
#endif

/* Logging. */
typedef enum _LogParameter {
    XLOG_FLUSH,
    XLOG_SYNC,
    XLOG_VERBOSITY,
    XLOG_FILE_VERBOSITY
} LogParameter;

/* Flags for log messages. */
typedef enum {
    X_PROBED,                   /* Value was probed */
    X_CONFIG,                   /* Value was given in the config file */
    X_DEFAULT,                  /* Value is a default */
    X_CMDLINE,                  /* Value was given on the command line */
    X_NOTICE,                   /* Notice */
    X_ERROR,                    /* Error message */
    X_WARNING,                  /* Warning message */
    X_INFO,                     /* Informational message */
    X_NONE,                     /* No prefix */
    X_NOT_IMPLEMENTED,          /* Not implemented */
    X_DEBUG,                    /* Debug message */
    X_UNKNOWN = -1              /* unknown -- this must always be last */
} MessageType;

extern XORG_EXPORT const char *
LogInit(const char *fname, const char *backup);
extern void
LogSetDisplay(void);
extern XORG_EXPORT void
LogClose(enum ExitCode error);
extern XORG_EXPORT Bool
LogSetParameter(LogParameter param, int value);
extern XORG_EXPORT void
LogVWrite(int verb, const char *f, va_list args)
_X_ATTRIBUTE_PRINTF(2, 0);
extern XORG_EXPORT void
LogWrite(int verb, const char *f, ...)
_X_ATTRIBUTE_PRINTF(2, 3);
extern XORG_EXPORT void
LogVMessageVerb(MessageType type, int verb, const char *format, va_list args)
_X_ATTRIBUTE_PRINTF(3, 0);
extern XORG_EXPORT void
LogMessageVerb(MessageType type, int verb, const char *format, ...)
_X_ATTRIBUTE_PRINTF(3, 4);
extern XORG_EXPORT void
LogMessage(MessageType type, const char *format, ...)
_X_ATTRIBUTE_PRINTF(2, 3);
extern XORG_EXPORT void
LogMessageVerbSigSafe(MessageType type, int verb, const char *format, ...)
_X_ATTRIBUTE_PRINTF(3, 4);
extern XORG_EXPORT void
LogVMessageVerbSigSafe(MessageType type, int verb, const char *format, va_list args)
_X_ATTRIBUTE_PRINTF(3, 0);

extern XORG_EXPORT void
LogVHdrMessageVerb(MessageType type, int verb,
                   const char *msg_format, va_list msg_args,
                   const char *hdr_format, va_list hdr_args)
_X_ATTRIBUTE_PRINTF(3, 0)
_X_ATTRIBUTE_PRINTF(5, 0);
extern XORG_EXPORT void
LogHdrMessageVerb(MessageType type, int verb,
                  const char *msg_format, va_list msg_args,
                  const char *hdr_format, ...)
_X_ATTRIBUTE_PRINTF(3, 0)
_X_ATTRIBUTE_PRINTF(5, 6);
extern XORG_EXPORT void
LogHdrMessage(MessageType type, const char *msg_format,
              va_list msg_args, const char *hdr_format, ...)
_X_ATTRIBUTE_PRINTF(2, 0)
_X_ATTRIBUTE_PRINTF(4, 5);

extern XORG_EXPORT void
FreeAuditTimer(void);
extern XORG_EXPORT void
AuditF(const char *f, ...)
_X_ATTRIBUTE_PRINTF(1, 2);
extern XORG_EXPORT void
VAuditF(const char *f, va_list args)
_X_ATTRIBUTE_PRINTF(1, 0);
extern XORG_EXPORT void
FatalError(const char *f, ...)
_X_ATTRIBUTE_PRINTF(1, 2)
    _X_NORETURN;

#ifdef DEBUG
#define DebugF ErrorF
#else
#define DebugF(...)             /* */
#endif

extern XORG_EXPORT void
VErrorF(const char *f, va_list args)
_X_ATTRIBUTE_PRINTF(1, 0);
extern XORG_EXPORT void
ErrorF(const char *f, ...)
_X_ATTRIBUTE_PRINTF(1, 2);
extern XORG_EXPORT void
VErrorFSigSafe(const char *f, va_list args)
_X_ATTRIBUTE_PRINTF(1, 0);
extern XORG_EXPORT void
ErrorFSigSafe(const char *f, ...)
_X_ATTRIBUTE_PRINTF(1, 2);
extern XORG_EXPORT void
LogPrintMarkers(void);

extern XORG_EXPORT void
xorg_backtrace(void);

extern XORG_EXPORT int
os_move_fd(int fd);

#include <signal.h>

extern XORG_EXPORT int
xthread_sigmask(int how, const sigset_t *set, sigset_t *oldest);

#endif                          /* OS_H */
