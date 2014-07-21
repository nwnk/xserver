/************************************************************
 Copyright (c) 1994 by Silicon Graphics Computer Systems, Inc.

 Permission to use, copy, modify, and distribute this
 software and its documentation for any purpose and without
 fee is hereby granted, provided that the above copyright
 notice appear in all copies and that both that copyright
 notice and this permission notice appear in supporting
 documentation, and that the name of Silicon Graphics not be
 used in advertising or publicity pertaining to distribution
 of the software without specific prior written permission.
 Silicon Graphics makes no representation about the suitability
 of this software for any purpose. It is provided "as is"
 without any express or implied warranty.

 SILICON GRAPHICS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SILICON
 GRAPHICS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
 THE USE OR PERFORMANCE OF THIS SOFTWARE.

 ********************************************************/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <X11/Xfuncs.h>

#include <X11/X.h>
#include <X11/keysym.h>
#include <X11/Xproto.h>
#include <X11/extensions/XKMformat.h>
#include "misc.h"
#include "inputstr.h"
#include "dix.h"
#include "xkbstr.h"
#define XKBSRV_NEED_FILE_FUNCS	1
#include <xkbsrv.h>

#include "xkbgeom.h"
#include "xkbfile.h"

#define	BUFFER_SIZE	512

static char textBuffer[BUFFER_SIZE];
static int tbNext = 0;

static const char *modNames[XkbNumModifiers] = {
    "Shift", "Lock", "Control", "Mod1", "Mod2", "Mod3", "Mod4", "Mod5"
};

static char *
tbGetBuffer(unsigned size)
{
    char *rtrn;

    if (size >= BUFFER_SIZE)
        return NULL;
    if ((BUFFER_SIZE - tbNext) <= size)
        tbNext = 0;
    rtrn = &textBuffer[tbNext];
    tbNext += size;
    return rtrn;
}

static char *
XkbStringText(char *str)
{
    char *buf;
    register char *in, *out;
    int len;
    Bool ok;

    if (str == NULL) {
        buf = tbGetBuffer(2);
        buf[0] = '\0';
        return buf;
    }
    for (ok = TRUE, len = 0, in = str; *in != '\0'; in++, len++) {
        if (!isprint(*in)) {
            ok = FALSE;
            switch (*in) {
            case '\n':
            case '\t':
            case '\v':
            case '\b':
            case '\r':
            case '\f':
                len++;
                break;
            default:
                len += 4;
                break;
            }
        }
    }
    if (ok)
        return str;
    buf = tbGetBuffer(len + 1);
    for (in = str, out = buf; *in != '\0'; in++) {
        if (isprint(*in))
            *out++ = *in;
        else {
            *out++ = '\\';
            if (*in == '\n')
                *out++ = 'n';
            else if (*in == '\t')
                *out++ = 't';
            else if (*in == '\v')
                *out++ = 'v';
            else if (*in == '\b')
                *out++ = 'b';
            else if (*in == '\r')
                *out++ = 'r';
            else if (*in == '\f')
                *out++ = 'f';
            else {
                *out++ = '0';
                sprintf(out, "%o", *in);
                while (*out != '\0')
                    out++;
            }
        }
    }
    *out++ = '\0';
    return buf;
}

static char *
XkbAtomText(Atom atm)
{
    const char *atmstr;
    char *rtrn;

    atmstr = NameForAtom(atm);
    if (atmstr != NULL) {
        int len;

        len = strlen(atmstr) + 1;
        if (len > BUFFER_SIZE)
            len = BUFFER_SIZE - 2;
        rtrn = tbGetBuffer(len);
        strlcpy(rtrn, atmstr, len);
    }
    else {
        rtrn = tbGetBuffer(1);
        rtrn[0] = '\0';
    }
    return XkbStringText(rtrn);
}

static char *
XkbVModIndexText(XkbDescPtr xkb, unsigned ndx)
{
    register int len;
    register Atom *vmodNames;
    char *rtrn;
    const char *tmp;
    char numBuf[20];

    if (xkb && xkb->names)
        vmodNames = xkb->names->vmods;
    else
        vmodNames = NULL;

    tmp = NULL;
    if (ndx >= XkbNumVirtualMods)
        tmp = "illegal";
    else if (vmodNames && (vmodNames[ndx] != None))
        tmp = NameForAtom(vmodNames[ndx]);
    if (tmp == NULL) {
        snprintf(numBuf, sizeof(numBuf), "%d", ndx);
        tmp = numBuf;
    }

    len = strlen(tmp) + 1;
    if (len >= BUFFER_SIZE)
        len = BUFFER_SIZE - 1;
    rtrn = tbGetBuffer(len);
    strncpy(rtrn, tmp, len);
    return rtrn;
}

static char *
XkbModMaskText(unsigned mask)
{
    register int i, bit;
    char buf[64], *rtrn;

    if ((mask & 0xff) == 0xff) {
        strcpy(buf, "all");
    }
    else if ((mask & 0xff) == 0) {
        strcpy(buf, "none");
    }
    else {
        char *str = buf;

        buf[0] = '\0';
        for (i = 0, bit = 1; i < XkbNumModifiers; i++, bit <<= 1) {
            if (mask & bit) {
                if (str != buf) {
                    *str++ = '+';
                }
                strcpy(str, modNames[i]);
                str = &str[strlen(str)];
            }
        }
    }
    rtrn = tbGetBuffer(strlen(buf) + 1);
    strcpy(rtrn, buf);
    return rtrn;
}

static char *
XkbVModMaskText(XkbDescPtr xkb, unsigned modMask, unsigned mask)
{
    register int i, bit;
    int len;
    char *mm, *rtrn;
    char *str, buf[BUFFER_SIZE];

    if ((modMask == 0) && (mask == 0)) {
        rtrn = tbGetBuffer(5);
        sprintf(rtrn, "none");
        return rtrn;
    }
    if (modMask != 0)
        mm = XkbModMaskText(modMask);
    else
        mm = NULL;

    str = buf;
    buf[0] = '\0';
    if (mask) {
        char *tmp;

        for (i = 0, bit = 1; i < XkbNumVirtualMods; i++, bit <<= 1) {
            if (mask & bit) {
                tmp = XkbVModIndexText(xkb, i);
                len = strlen(tmp) + 1 + (str == buf ? 0 : 1);
                if ((str - (buf + len)) <= BUFFER_SIZE) {
                    if (str != buf) {
                        *str++ = '+';
                        len--;
                    }
                }
                strcpy(str, tmp);
                str = &str[len - 1];
            }
        }
        str = buf;
    }
    else
        str = NULL;
    if (mm)
        len = strlen(mm);
    else
        len = 0;
    if (str)
        len += strlen(str) + (mm == NULL ? 0 : 1);
    if (len >= BUFFER_SIZE)
        len = BUFFER_SIZE - 1;
    rtrn = tbGetBuffer(len + 1);
    rtrn[0] = '\0';

    if (mm != NULL) {
        i = strlen(mm);
        if (i > len)
            i = len;
        strcpy(rtrn, mm);
    }
    else {
        i = 0;
    }
    if (str != NULL) {
        if (mm != NULL) {
            strcat(rtrn, "+");
        }
        strncat(rtrn, str, len - i);
    }
    rtrn[len] = '\0';
    return rtrn;
}

static char *
XkbModIndexText(unsigned ndx)
{
    char *rtrn;
    char buf[100];

    if (ndx < XkbNumModifiers)
        strcpy(buf, modNames[ndx]);
    else if (ndx == XkbNoModifier)
        strcpy(buf, "none");
    else
        snprintf(buf, sizeof(buf), "ILLEGAL_%02x", ndx);
    rtrn = tbGetBuffer(strlen(buf) + 1);
    strcpy(rtrn, buf);
    return rtrn;
}

static char *
XkbKeysymText(KeySym sym)
{
    static char buf[32];

    if (sym == NoSymbol)
        strcpy(buf, "NoSymbol");
    else
        snprintf(buf, sizeof(buf), "0x%lx", (long) sym);
    return buf;
}

static char *
XkbKeyNameText(char *name)
{
    char *buf;
    int len;

    buf = tbGetBuffer(7);
    buf[0] = '<';
    memcpy(&buf[1], name, 4);
    buf[5] = '\0';
    len = strlen(buf);
    buf[len++] = '>';
    buf[len] = '\0';

    return buf;
}

static const char *siMatchText[5] = {
    "NoneOf", "AnyOfOrNone", "AnyOf", "AllOf", "Exactly"
};

static const char *
XkbSIMatchText(unsigned type)
{
    static char buf[40];
    const char *rtrn;

    switch (type & XkbSI_OpMask) {
    case XkbSI_NoneOf:
        rtrn = siMatchText[0];
        break;
    case XkbSI_AnyOfOrNone:
        rtrn = siMatchText[1];
        break;
    case XkbSI_AnyOf:
        rtrn = siMatchText[2];
        break;
    case XkbSI_AllOf:
        rtrn = siMatchText[3];
        break;
    case XkbSI_Exactly:
        rtrn = siMatchText[4];
        break;
    default:
        snprintf(buf, sizeof(buf), "0x%x", type & XkbSI_OpMask);
        return buf;
    }
    return rtrn;
}

static const char *imWhichNames[] = {
    "base",
    "latched",
    "locked",
    "effective",
    "compat"
};

static char *
XkbIMWhichStateMaskText(unsigned use_which)
{
    int len;
    unsigned i, bit, tmp;
    char *buf;

    if (use_which == 0) {
        buf = tbGetBuffer(2);
        strcpy(buf, "0");
        return buf;
    }
    tmp = use_which & XkbIM_UseAnyMods;
    for (len = i = 0, bit = 1; tmp != 0; i++, bit <<= 1) {
        if (tmp & bit) {
            tmp &= ~bit;
            len += strlen(imWhichNames[i]) + 1;
        }
    }
    buf = tbGetBuffer(len + 1);
    tmp = use_which & XkbIM_UseAnyMods;
    for (len = i = 0, bit = 1; tmp != 0; i++, bit <<= 1) {
        if (tmp & bit) {
            tmp &= ~bit;
            if (len != 0)
                buf[len++] = '+';
            sprintf(&buf[len], "%s", imWhichNames[i]);
            len += strlen(&buf[len]);
        }
    }
    return buf;
}

static const char *ctrlNames[] = {
    "repeatKeys",
    "slowKeys",
    "bounceKeys",
    "stickyKeys",
    "mouseKeys",
    "mouseKeysAccel",
    "accessXKeys",
    "accessXTimeout",
    "accessXFeedback",
    "audibleBell",
    "overlay1",
    "overlay2",
    "ignoreGroupLock"
};

static char *
XkbControlsMaskText(unsigned ctrls)
{
    int len;
    unsigned i, bit, tmp;
    char *buf;

    if (ctrls == 0) {
        buf = tbGetBuffer(5);
        strcpy(buf, "none");
        return buf;
    }
    tmp = ctrls & XkbAllBooleanCtrlsMask;
    for (len = i = 0, bit = 1; tmp != 0; i++, bit <<= 1) {
        if (tmp & bit) {
            tmp &= ~bit;
            len += strlen(ctrlNames[i]) + 1;
        }
    }
    buf = tbGetBuffer(len + 1);
    tmp = ctrls & XkbAllBooleanCtrlsMask;
    for (len = i = 0, bit = 1; tmp != 0; i++, bit <<= 1) {
        if (tmp & bit) {
            tmp &= ~bit;
            if (len != 0)
                buf[len++] = '+';
            sprintf(&buf[len], "%s", ctrlNames[i]);
            len += strlen(&buf[len]);
        }
    }
    return buf;
}

static char *
XkbGeomFPText(int val)
{
    int whole, frac;
    char *buf;

    buf = tbGetBuffer(12);
    whole = val / XkbGeomPtsPerMM;
    frac = val % XkbGeomPtsPerMM;
    if (frac != 0)
        sprintf(buf, "%d.%d", whole, frac);
    else
        sprintf(buf, "%d", whole);
    return buf;
}

static char *
XkbDoodadTypeText(unsigned type)
{
    char *buf = tbGetBuffer(12);

    if (type == XkbOutlineDoodad)
        strcpy(buf, "outline");
    else if (type == XkbSolidDoodad)
        strcpy(buf, "solid");
    else if (type == XkbTextDoodad)
        strcpy(buf, "text");
    else if (type == XkbIndicatorDoodad)
        strcpy(buf, "indicator");
    else if (type == XkbLogoDoodad)
        strcpy(buf, "logo");
    else
        sprintf(buf, "unknown%d", type);

    return buf;
}

static const char *actionTypeNames[XkbSA_NumActions] = {
    "NoAction",
    "SetMods", "LatchMods", "LockMods",
    "SetGroup", "LatchGroup", "LockGroup",
    "MovePtr",
    "PtrBtn", "LockPtrBtn",
    "SetPtrDflt",
    "ISOLock",
    "Terminate", "SwitchScreen",
    "SetControls", "LockControls",
    "ActionMessage",
    "RedirectKey",
    "DeviceBtn", "LockDeviceBtn"
};

static const char *
XkbActionTypeText(unsigned type)
{
    static char buf[32];
    const char *rtrn;

    if (type <= XkbSA_LastAction) {
        rtrn = actionTypeNames[type];
        return rtrn;
    }
    snprintf(buf, sizeof(buf), "Private");
    return buf;
}

static int
TryCopyStr(char *to, const char *from, int *pLeft)
{
    register int len;

    if (*pLeft > 0) {
        len = strlen(from);
        if (len < ((*pLeft) - 3)) {
            strcat(to, from);
            *pLeft -= len;
            return TRUE;
        }
    }
    *pLeft = -1;
    return FALSE;
}

static Bool
CopyNoActionArgs(XkbDescPtr xkb, XkbAction *action, char *buf, int *sz)
{
    return TRUE;
}

static Bool
CopyModActionArgs(XkbDescPtr xkb, XkbAction *action, char *buf, int *sz)
{
    XkbModAction *act;
    unsigned tmp;

    act = &action->mods;
    tmp = XkbModActionVMods(act);
    TryCopyStr(buf, "modifiers=", sz);
    if (act->flags & XkbSA_UseModMapMods)
        TryCopyStr(buf, "modMapMods", sz);
    else if (act->real_mods || tmp) {
        TryCopyStr(buf, XkbVModMaskText(xkb, act->real_mods, tmp), sz);
    }
    else
        TryCopyStr(buf, "none", sz);
    if (act->type == XkbSA_LockMods)
        return TRUE;
    if (act->flags & XkbSA_ClearLocks)
        TryCopyStr(buf, ",clearLocks", sz);
    if (act->flags & XkbSA_LatchToLock)
        TryCopyStr(buf, ",latchToLock", sz);
    return TRUE;
}

static Bool
CopyGroupActionArgs(XkbDescPtr xkb, XkbAction *action, char *buf, int *sz)
{
    XkbGroupAction *act;
    char tbuf[32];

    act = &action->group;
    TryCopyStr(buf, "group=", sz);
    if (act->flags & XkbSA_GroupAbsolute)
        snprintf(tbuf, sizeof(tbuf), "%d", XkbSAGroup(act) + 1);
    else if (XkbSAGroup(act) < 0)
        snprintf(tbuf, sizeof(tbuf), "%d", XkbSAGroup(act));
    else
        snprintf(tbuf, sizeof(tbuf), "+%d", XkbSAGroup(act));
    TryCopyStr(buf, tbuf, sz);
    if (act->type == XkbSA_LockGroup)
        return TRUE;
    if (act->flags & XkbSA_ClearLocks)
        TryCopyStr(buf, ",clearLocks", sz);
    if (act->flags & XkbSA_LatchToLock)
        TryCopyStr(buf, ",latchToLock", sz);
    return TRUE;
}

static Bool
CopyMovePtrArgs(XkbDescPtr xkb, XkbAction *action, char *buf, int *sz)
{
    XkbPtrAction *act;
    int x, y;
    char tbuf[32];

    act = &action->ptr;
    x = XkbPtrActionX(act);
    y = XkbPtrActionY(act);
    if ((act->flags & XkbSA_MoveAbsoluteX) || (x < 0))
        snprintf(tbuf, sizeof(tbuf), "x=%d", x);
    else
        snprintf(tbuf, sizeof(tbuf), "x=+%d", x);
    TryCopyStr(buf, tbuf, sz);

    if ((act->flags & XkbSA_MoveAbsoluteY) || (y < 0))
        snprintf(tbuf, sizeof(tbuf), ",y=%d", y);
    else
        snprintf(tbuf, sizeof(tbuf), ",y=+%d", y);
    TryCopyStr(buf, tbuf, sz);
    if (act->flags & XkbSA_NoAcceleration)
        TryCopyStr(buf, ",!accel", sz);
    return TRUE;
}

static Bool
CopyPtrBtnArgs(XkbDescPtr xkb, XkbAction *action, char *buf, int *sz)
{
    XkbPtrBtnAction *act;
    char tbuf[32];

    act = &action->btn;
    TryCopyStr(buf, "button=", sz);
    if ((act->button > 0) && (act->button < 6)) {
        snprintf(tbuf, sizeof(tbuf), "%d", act->button);
        TryCopyStr(buf, tbuf, sz);
    }
    else
        TryCopyStr(buf, "default", sz);
    if (act->count > 0) {
        snprintf(tbuf, sizeof(tbuf), ",count=%d", act->count);
        TryCopyStr(buf, tbuf, sz);
    }
    if (action->type == XkbSA_LockPtrBtn) {
        switch (act->flags & (XkbSA_LockNoUnlock | XkbSA_LockNoLock)) {
        case XkbSA_LockNoLock:
            TryCopyStr(buf, ",affect=unlock", sz);
            break;
        case XkbSA_LockNoUnlock:
            TryCopyStr(buf, ",affect=lock", sz);
            break;
        case XkbSA_LockNoUnlock | XkbSA_LockNoLock:
            TryCopyStr(buf, ",affect=neither", sz);
            break;
        default:
            TryCopyStr(buf, ",affect=both", sz);
            break;
        }
    }
    return TRUE;
}

static Bool
CopySetPtrDfltArgs(XkbDescPtr xkb, XkbAction *action, char *buf, int *sz)
{
    XkbPtrDfltAction *act;
    char tbuf[32];

    act = &action->dflt;
    if (act->affect == XkbSA_AffectDfltBtn) {
        TryCopyStr(buf, "affect=button,button=", sz);
        if ((act->flags & XkbSA_DfltBtnAbsolute) ||
            (XkbSAPtrDfltValue(act) < 0))
            snprintf(tbuf, sizeof(tbuf), "%d", XkbSAPtrDfltValue(act));
        else
            snprintf(tbuf, sizeof(tbuf), "+%d", XkbSAPtrDfltValue(act));
        TryCopyStr(buf, tbuf, sz);
    }
    return TRUE;
}

static Bool
CopyISOLockArgs(XkbDescPtr xkb, XkbAction *action, char *buf, int *sz)
{
    XkbISOAction *act;
    char tbuf[64];

    act = &action->iso;
    if (act->flags & XkbSA_ISODfltIsGroup) {
        TryCopyStr(tbuf, "group=", sz);
        if (act->flags & XkbSA_GroupAbsolute)
            snprintf(tbuf, sizeof(tbuf), "%d", XkbSAGroup(act) + 1);
        else if (XkbSAGroup(act) < 0)
            snprintf(tbuf, sizeof(tbuf), "%d", XkbSAGroup(act));
        else
            snprintf(tbuf, sizeof(tbuf), "+%d", XkbSAGroup(act));
        TryCopyStr(buf, tbuf, sz);
    }
    else {
        unsigned tmp;

        tmp = XkbModActionVMods(act);
        TryCopyStr(buf, "modifiers=", sz);
        if (act->flags & XkbSA_UseModMapMods)
            TryCopyStr(buf, "modMapMods", sz);
        else if (act->real_mods || tmp) {
            if (act->real_mods) {
                TryCopyStr(buf, XkbModMaskText(act->real_mods), sz);
                if (tmp)
                    TryCopyStr(buf, "+", sz);
            }
            if (tmp)
                TryCopyStr(buf, XkbVModMaskText(xkb, 0, tmp), sz);
        }
        else
            TryCopyStr(buf, "none", sz);
    }
    TryCopyStr(buf, ",affect=", sz);
    if ((act->affect & XkbSA_ISOAffectMask) == 0)
        TryCopyStr(buf, "all", sz);
    else {
        int nOut = 0;

        if ((act->affect & XkbSA_ISONoAffectMods) == 0) {
            TryCopyStr(buf, "mods", sz);
            nOut++;
        }
        if ((act->affect & XkbSA_ISONoAffectGroup) == 0) {
            snprintf(tbuf, sizeof(tbuf), "%sgroups", (nOut > 0 ? "+" : ""));
            TryCopyStr(buf, tbuf, sz);
            nOut++;
        }
        if ((act->affect & XkbSA_ISONoAffectPtr) == 0) {
            snprintf(tbuf, sizeof(tbuf), "%spointer", (nOut > 0 ? "+" : ""));
            TryCopyStr(buf, tbuf, sz);
            nOut++;
        }
        if ((act->affect & XkbSA_ISONoAffectCtrls) == 0) {
            snprintf(tbuf, sizeof(tbuf), "%scontrols", (nOut > 0 ? "+" : ""));
            TryCopyStr(buf, tbuf, sz);
            nOut++;
        }
    }
    return TRUE;
}

static Bool
CopySwitchScreenArgs(XkbDescPtr xkb, XkbAction *action, char *buf, int *sz)
{
    XkbSwitchScreenAction *act;
    char tbuf[32];

    act = &action->screen;
    if ((act->flags & XkbSA_SwitchAbsolute) || (XkbSAScreen(act) < 0))
        snprintf(tbuf, sizeof(tbuf), "screen=%d", XkbSAScreen(act));
    else
        snprintf(tbuf, sizeof(tbuf), "screen=+%d", XkbSAScreen(act));
    TryCopyStr(buf, tbuf, sz);
    if (act->flags & XkbSA_SwitchApplication)
        TryCopyStr(buf, ",!same", sz);
    else
        TryCopyStr(buf, ",same", sz);
    return TRUE;
}

static Bool
CopySetLockControlsArgs(XkbDescPtr xkb, XkbAction *action, char *buf, int *sz)
{
    XkbCtrlsAction *act;
    unsigned tmp;
    char tbuf[32];

    act = &action->ctrls;
    tmp = XkbActionCtrls(act);
    TryCopyStr(buf, "controls=", sz);
    if (tmp == 0)
        TryCopyStr(buf, "none", sz);
    else if ((tmp & XkbAllBooleanCtrlsMask) == XkbAllBooleanCtrlsMask)
        TryCopyStr(buf, "all", sz);
    else {
        int nOut = 0;

        if (tmp & XkbRepeatKeysMask) {
            snprintf(tbuf, sizeof(tbuf), "%sRepeatKeys", (nOut > 0 ? "+" : ""));
            TryCopyStr(buf, tbuf, sz);
            nOut++;
        }
        if (tmp & XkbSlowKeysMask) {
            snprintf(tbuf, sizeof(tbuf), "%sSlowKeys", (nOut > 0 ? "+" : ""));
            TryCopyStr(buf, tbuf, sz);
            nOut++;
        }
        if (tmp & XkbBounceKeysMask) {
            snprintf(tbuf, sizeof(tbuf), "%sBounceKeys", (nOut > 0 ? "+" : ""));
            TryCopyStr(buf, tbuf, sz);
            nOut++;
        }
        if (tmp & XkbStickyKeysMask) {
            snprintf(tbuf, sizeof(tbuf), "%sStickyKeys", (nOut > 0 ? "+" : ""));
            TryCopyStr(buf, tbuf, sz);
            nOut++;
        }
        if (tmp & XkbMouseKeysMask) {
            snprintf(tbuf, sizeof(tbuf), "%sMouseKeys", (nOut > 0 ? "+" : ""));
            TryCopyStr(buf, tbuf, sz);
            nOut++;
        }
        if (tmp & XkbMouseKeysAccelMask) {
            snprintf(tbuf, sizeof(tbuf), "%sMouseKeysAccel",
                     (nOut > 0 ? "+" : ""));
            TryCopyStr(buf, tbuf, sz);
            nOut++;
        }
        if (tmp & XkbAccessXKeysMask) {
            snprintf(tbuf, sizeof(tbuf), "%sAccessXKeys",
                     (nOut > 0 ? "+" : ""));
            TryCopyStr(buf, tbuf, sz);
            nOut++;
        }
        if (tmp & XkbAccessXTimeoutMask) {
            snprintf(tbuf, sizeof(tbuf), "%sAccessXTimeout",
                     (nOut > 0 ? "+" : ""));
            TryCopyStr(buf, tbuf, sz);
            nOut++;
        }
        if (tmp & XkbAccessXFeedbackMask) {
            snprintf(tbuf, sizeof(tbuf), "%sAccessXFeedback",
                     (nOut > 0 ? "+" : ""));
            TryCopyStr(buf, tbuf, sz);
            nOut++;
        }
        if (tmp & XkbAudibleBellMask) {
            snprintf(tbuf, sizeof(tbuf), "%sAudibleBell",
                     (nOut > 0 ? "+" : ""));
            TryCopyStr(buf, tbuf, sz);
            nOut++;
        }
        if (tmp & XkbOverlay1Mask) {
            snprintf(tbuf, sizeof(tbuf), "%sOverlay1", (nOut > 0 ? "+" : ""));
            TryCopyStr(buf, tbuf, sz);
            nOut++;
        }
        if (tmp & XkbOverlay2Mask) {
            snprintf(tbuf, sizeof(tbuf), "%sOverlay2", (nOut > 0 ? "+" : ""));
            TryCopyStr(buf, tbuf, sz);
            nOut++;
        }
        if (tmp & XkbIgnoreGroupLockMask) {
            snprintf(tbuf, sizeof(tbuf), "%sIgnoreGroupLock",
                     (nOut > 0 ? "+" : ""));
            TryCopyStr(buf, tbuf, sz);
            nOut++;
        }
    }
    return TRUE;
}

static Bool
CopyActionMessageArgs(XkbDescPtr xkb, XkbAction *action, char *buf, int *sz)
{
    XkbMessageAction *act;
    unsigned all;
    char tbuf[32];

    act = &action->msg;
    all = XkbSA_MessageOnPress | XkbSA_MessageOnRelease;
    TryCopyStr(buf, "report=", sz);
    if ((act->flags & all) == 0)
        TryCopyStr(buf, "none", sz);
    else if ((act->flags & all) == all)
        TryCopyStr(buf, "all", sz);
    else if (act->flags & XkbSA_MessageOnPress)
        TryCopyStr(buf, "KeyPress", sz);
    else
        TryCopyStr(buf, "KeyRelease", sz);
    snprintf(tbuf, sizeof(tbuf), ",data[0]=0x%02x", act->message[0]);
    TryCopyStr(buf, tbuf, sz);
    snprintf(tbuf, sizeof(tbuf), ",data[1]=0x%02x", act->message[1]);
    TryCopyStr(buf, tbuf, sz);
    snprintf(tbuf, sizeof(tbuf), ",data[2]=0x%02x", act->message[2]);
    TryCopyStr(buf, tbuf, sz);
    snprintf(tbuf, sizeof(tbuf), ",data[3]=0x%02x", act->message[3]);
    TryCopyStr(buf, tbuf, sz);
    snprintf(tbuf, sizeof(tbuf), ",data[4]=0x%02x", act->message[4]);
    TryCopyStr(buf, tbuf, sz);
    snprintf(tbuf, sizeof(tbuf), ",data[5]=0x%02x", act->message[5]);
    TryCopyStr(buf, tbuf, sz);
    return TRUE;
}

static Bool
CopyRedirectKeyArgs(XkbDescPtr xkb, XkbAction *action, char *buf, int *sz)
{
    XkbRedirectKeyAction *act;
    char tbuf[32], *tmp;
    unsigned kc;
    unsigned vmods, vmods_mask;

    act = &action->redirect;
    kc = act->new_key;
    vmods = XkbSARedirectVMods(act);
    vmods_mask = XkbSARedirectVModsMask(act);
    if (xkb && xkb->names && xkb->names->keys && (kc <= xkb->max_key_code) &&
        (xkb->names->keys[kc].name[0] != '\0')) {
        char *kn;

        kn = XkbKeyNameText(xkb->names->keys[kc].name);
        snprintf(tbuf, sizeof(tbuf), "key=%s", kn);
    }
    else
        snprintf(tbuf, sizeof(tbuf), "key=%d", kc);
    TryCopyStr(buf, tbuf, sz);
    if ((act->mods_mask == 0) && (vmods_mask == 0))
        return TRUE;
    if ((act->mods_mask == XkbAllModifiersMask) &&
        (vmods_mask == XkbAllVirtualModsMask)) {
        tmp = XkbVModMaskText(xkb, act->mods, vmods);
        TryCopyStr(buf, ",mods=", sz);
        TryCopyStr(buf, tmp, sz);
    }
    else {
        if ((act->mods_mask & act->mods) || (vmods_mask & vmods)) {
            tmp = XkbVModMaskText(xkb, act->mods_mask & act->mods,
                                  vmods_mask & vmods);
            TryCopyStr(buf, ",mods= ", sz);
            TryCopyStr(buf, tmp, sz);
        }
        if ((act->mods_mask & (~act->mods)) || (vmods_mask & (~vmods))) {
            tmp = XkbVModMaskText(xkb, act->mods_mask & (~act->mods),
                                  vmods_mask & (~vmods));
            TryCopyStr(buf, ",clearMods= ", sz);
            TryCopyStr(buf, tmp, sz);
        }
    }
    return TRUE;
}

static Bool
CopyDeviceBtnArgs(XkbDescPtr xkb, XkbAction *action, char *buf, int *sz)
{
    XkbDeviceBtnAction *act;
    char tbuf[32];

    act = &action->devbtn;
    snprintf(tbuf, sizeof(tbuf), "device= %d", act->device);
    TryCopyStr(buf, tbuf, sz);
    TryCopyStr(buf, ",button=", sz);
    snprintf(tbuf, sizeof(tbuf), "%d", act->button);
    TryCopyStr(buf, tbuf, sz);
    if (act->count > 0) {
        snprintf(tbuf, sizeof(tbuf), ",count=%d", act->count);
        TryCopyStr(buf, tbuf, sz);
    }
    if (action->type == XkbSA_LockDeviceBtn) {
        switch (act->flags & (XkbSA_LockNoUnlock | XkbSA_LockNoLock)) {
        case XkbSA_LockNoLock:
            TryCopyStr(buf, ",affect=unlock", sz);
            break;
        case XkbSA_LockNoUnlock:
            TryCopyStr(buf, ",affect=lock", sz);
            break;
        case XkbSA_LockNoUnlock | XkbSA_LockNoLock:
            TryCopyStr(buf, ",affect=neither", sz);
            break;
        default:
            TryCopyStr(buf, ",affect=both", sz);
            break;
        }
    }
    return TRUE;
}

static Bool
CopyOtherArgs(XkbDescPtr xkb, XkbAction *action, char *buf, int *sz)
{
    XkbAnyAction *act;
    char tbuf[32];

    act = &action->any;
    snprintf(tbuf, sizeof(tbuf), "type=0x%02x", act->type);
    TryCopyStr(buf, tbuf, sz);
    snprintf(tbuf, sizeof(tbuf), ",data[0]=0x%02x", act->data[0]);
    TryCopyStr(buf, tbuf, sz);
    snprintf(tbuf, sizeof(tbuf), ",data[1]=0x%02x", act->data[1]);
    TryCopyStr(buf, tbuf, sz);
    snprintf(tbuf, sizeof(tbuf), ",data[2]=0x%02x", act->data[2]);
    TryCopyStr(buf, tbuf, sz);
    snprintf(tbuf, sizeof(tbuf), ",data[3]=0x%02x", act->data[3]);
    TryCopyStr(buf, tbuf, sz);
    snprintf(tbuf, sizeof(tbuf), ",data[4]=0x%02x", act->data[4]);
    TryCopyStr(buf, tbuf, sz);
    snprintf(tbuf, sizeof(tbuf), ",data[5]=0x%02x", act->data[5]);
    TryCopyStr(buf, tbuf, sz);
    snprintf(tbuf, sizeof(tbuf), ",data[6]=0x%02x", act->data[6]);
    TryCopyStr(buf, tbuf, sz);
    return TRUE;
}

typedef Bool (*actionCopy) (XkbDescPtr /* xkb */ ,
                            XkbAction * /* action */ ,
                            char * /* buf */ ,
                            int *       /* sz */
    );

static actionCopy copyActionArgs[XkbSA_NumActions] = {
    CopyNoActionArgs /* NoAction     */ ,
    CopyModActionArgs /* SetMods      */ ,
    CopyModActionArgs /* LatchMods    */ ,
    CopyModActionArgs /* LockMods     */ ,
    CopyGroupActionArgs /* SetGroup     */ ,
    CopyGroupActionArgs /* LatchGroup   */ ,
    CopyGroupActionArgs /* LockGroup    */ ,
    CopyMovePtrArgs /* MovePtr      */ ,
    CopyPtrBtnArgs /* PtrBtn       */ ,
    CopyPtrBtnArgs /* LockPtrBtn   */ ,
    CopySetPtrDfltArgs /* SetPtrDflt   */ ,
    CopyISOLockArgs /* ISOLock      */ ,
    CopyNoActionArgs /* Terminate    */ ,
    CopySwitchScreenArgs /* SwitchScreen */ ,
    CopySetLockControlsArgs /* SetControls  */ ,
    CopySetLockControlsArgs /* LockControls */ ,
    CopyActionMessageArgs /* ActionMessage */ ,
    CopyRedirectKeyArgs /* RedirectKey  */ ,
    CopyDeviceBtnArgs /* DeviceBtn    */ ,
    CopyDeviceBtnArgs           /* LockDeviceBtn */
};

#define	ACTION_SZ	256

static char *
XkbActionText(XkbDescPtr xkb, XkbAction *action)
{
    char buf[ACTION_SZ], *tmp;
    int sz;

    snprintf(buf, sizeof(buf), "%s(", XkbActionTypeText(action->type));
    sz = ACTION_SZ - strlen(buf) + 2;       /* room for close paren and NULL */
    if (action->type < (unsigned) XkbSA_NumActions)
        (*copyActionArgs[action->type]) (xkb, action, buf, &sz);
    else
        CopyOtherArgs(xkb, action, buf, &sz);
    TryCopyStr(buf, ")", &sz);
    tmp = tbGetBuffer(strlen(buf) + 1);
    if (tmp != NULL)
        strcpy(tmp, buf);
    return tmp;
}

static char *
XkbBehaviorText(XkbDescPtr xkb, XkbBehavior * behavior)
{
    char buf[256], *tmp;
    unsigned type, permanent;

    type = behavior->type & XkbKB_OpMask;
    permanent = ((behavior->type & XkbKB_Permanent) != 0);

    if (type == XkbKB_Lock) {
        snprintf(buf, sizeof(buf), "lock= %s",
                 (permanent ? "Permanent" : "TRUE"));
    }
    else if (type == XkbKB_RadioGroup) {
        int g;

        g = ((behavior->data) & (~XkbKB_RGAllowNone)) + 1;
        if (XkbKB_RGAllowNone & behavior->data) {
            snprintf(buf, sizeof(buf), "allowNone,");
            tmp = &buf[strlen(buf)];
        }
        else
            tmp = buf;
        if (permanent)
            sprintf(tmp, "permanentRadioGroup= %d", g);
        else
            sprintf(tmp, "radioGroup= %d", g);
    }
    else if ((type == XkbKB_Overlay1) || (type == XkbKB_Overlay2)) {
        int ndx, kc;
        char *kn;

        ndx = ((type == XkbKB_Overlay1) ? 1 : 2);
        kc = behavior->data;
        if ((xkb) && (xkb->names) && (xkb->names->keys))
            kn = XkbKeyNameText(xkb->names->keys[kc].name);
        else {
            static char tbuf[8];

            snprintf(tbuf, sizeof(tbuf), "%d", kc);
            kn = tbuf;
        }
        if (permanent)
            snprintf(buf, sizeof(buf), "permanentOverlay%d= %s", ndx, kn);
        else
            snprintf(buf, sizeof(buf), "overlay%d= %s", ndx, kn);
    }

    tmp = tbGetBuffer(strlen(buf) + 1);
    if (tmp != NULL)
        strcpy(tmp, buf);
    return tmp;
}

static char *
XkbIndentText(unsigned size)
{
    static char buf[32];
    register int i;

    if (size > 31)
        size = 31;

    for (i = 0; i < size; i++) {
        buf[i] = ' ';
    }
    buf[size] = '\0';
    return buf;
}

#define	VMOD_HIDE_VALUE	0
#define	VMOD_SHOW_VALUE	1
#define	VMOD_COMMENT_VALUE 2

static Bool
WriteXKBVModDecl(FILE * file, XkbDescPtr xkb, int showValue)
{
    register int i, nMods;
    Atom *vmodNames;

    if (xkb == NULL)
        return FALSE;
    if (xkb->names != NULL)
        vmodNames = xkb->names->vmods;
    else
        vmodNames = NULL;

    for (i = nMods = 0; i < XkbNumVirtualMods; i++) {
        if ((vmodNames != NULL) && (vmodNames[i] != None)) {
            if (nMods == 0)
                fprintf(file, "    virtual_modifiers ");
            else
                fprintf(file, ",");
            fprintf(file, "%s", XkbAtomText(vmodNames[i]));
            if ((showValue != VMOD_HIDE_VALUE) &&
                (xkb->server) && (xkb->server->vmods[i] != XkbNoModifierMask)) {
                if (showValue == VMOD_COMMENT_VALUE) {
                    fprintf(file, "/* = %s */",
                            XkbModMaskText(xkb->server->vmods[i]));
                }
                else {
                    fprintf(file, "= %s",
                            XkbModMaskText(xkb->server->vmods[i]));
                }
            }
            nMods++;
        }
    }
    if (nMods > 0)
        fprintf(file, ";\n\n");
    return TRUE;
}

/***====================================================================***/

static Bool
WriteXKBAction(FILE * file, XkbDescPtr xkb, XkbAnyAction * action)
{
    fprintf(file, "%s", XkbActionText(xkb, (XkbAction *) action));
    return TRUE;
}

/***====================================================================***/

Bool
XkbWriteXKBKeycodes(FILE * file,
                    XkbDescPtr xkb,
                    Bool topLevel,
                    Bool showImplicit, XkbFileAddOnFunc addOn, void *priv)
{
    Atom kcName;
    register unsigned i;
    const char *alternate;

    if ((!xkb) || (!xkb->names) || (!xkb->names->keys)) {
        return FALSE;
    }
    kcName = xkb->names->keycodes;
    if (kcName != None)
        fprintf(file, "xkb_keycodes \"%s\" {\n", XkbAtomText(kcName));
    else
        fprintf(file, "xkb_keycodes {\n");
    fprintf(file, "    minimum = %d;\n", xkb->min_key_code);
    fprintf(file, "    maximum = %d;\n", xkb->max_key_code);
    for (i = xkb->min_key_code; i <= xkb->max_key_code; i++) {
        if (xkb->names->keys[i].name[0] != '\0') {
            if (XkbFindKeycodeByName(xkb, xkb->names->keys[i].name, TRUE) != i)
                alternate = "alternate ";
            else
                alternate = "";
            fprintf(file, "    %s%6s = %d;\n", alternate,
                    XkbKeyNameText(xkb->names->keys[i].name), i);
        }
    }
    if (xkb->indicators != NULL) {
        for (i = 0; i < XkbNumIndicators; i++) {
            const char *type;

            if (xkb->indicators->phys_indicators & (1 << i))
                type = "    ";
            else
                type = "    virtual ";
            if (xkb->names->indicators[i] != None) {
                fprintf(file, "%sindicator %d = \"%s\";\n", type, i + 1,
                        XkbAtomText(xkb->names->indicators[i]));
            }
        }
    }
    if (xkb->names->key_aliases != NULL) {
        XkbKeyAliasPtr pAl;

        pAl = xkb->names->key_aliases;
        for (i = 0; i < xkb->names->num_key_aliases; i++, pAl++) {
            fprintf(file, "    alias %6s = %6s;\n", XkbKeyNameText(pAl->alias),
                    XkbKeyNameText(pAl->real));
        }
    }
    if (addOn)
        (*addOn) (file, xkb, topLevel, showImplicit, XkmKeyNamesIndex, priv);
    fprintf(file, "};\n\n");
    return TRUE;
}

Bool
XkbWriteXKBKeyTypes(FILE * file,
                    XkbDescPtr xkb,
                    Bool topLevel,
                    Bool showImplicit, XkbFileAddOnFunc addOn, void *priv)
{
    register unsigned i, n;
    XkbKeyTypePtr type;
    XkbKTMapEntryPtr entry;

    if ((!xkb) || (!xkb->map) || (!xkb->map->types)) {
        return FALSE;
    }
    if (xkb->map->num_types < XkbNumRequiredTypes) {
        return 0;
    }
    if ((xkb->names == NULL) || (xkb->names->types == None))
        fprintf(file, "xkb_types {\n\n");
    else
        fprintf(file, "xkb_types \"%s\" {\n\n", XkbAtomText(xkb->names->types));
    WriteXKBVModDecl(file, xkb,
                     (showImplicit ? VMOD_COMMENT_VALUE : VMOD_HIDE_VALUE));

    type = xkb->map->types;
    for (i = 0; i < xkb->map->num_types; i++, type++) {
        fprintf(file, "    type \"%s\" {\n",
                XkbAtomText(type->name));
        fprintf(file, "        modifiers= %s;\n",
                XkbVModMaskText(xkb, type->mods.real_mods, type->mods.vmods));
        entry = type->map;
        for (n = 0; n < type->map_count; n++, entry++) {
            char *str;

            str = XkbVModMaskText(xkb, entry->mods.real_mods,
                                  entry->mods.vmods);
            fprintf(file, "        map[%s]= Level%d;\n", str, entry->level + 1);
            if ((type->preserve) && ((type->preserve[n].real_mods) ||
                                     (type->preserve[n].vmods))) {
                fprintf(file, "        preserve[%s]= ", str);
                fprintf(file, "%s;\n", XkbVModMaskText(xkb,
                                                       type->preserve[n].
                                                       real_mods,
                                                       type->preserve[n].vmods
                                                       ));
            }
        }
        if (type->level_names != NULL) {
            Atom *name = type->level_names;

            for (n = 0; n < type->num_levels; n++, name++) {
                if ((*name) == None)
                    continue;
                fprintf(file, "        level_name[Level%d]= \"%s\";\n", n + 1,
                        XkbAtomText(*name));
            }
        }
        fprintf(file, "    };\n");
    }
    if (addOn)
        (*addOn) (file, xkb, topLevel, showImplicit, XkmTypesIndex, priv);
    fprintf(file, "};\n\n");
    return TRUE;
}

static Bool
WriteXKBIndicatorMap(FILE * file,
                     XkbDescPtr xkb,
                     Atom name,
                     XkbIndicatorMapPtr led, XkbFileAddOnFunc addOn, void *priv)
{

    fprintf(file, "    indicator \"%s\" {\n", NameForAtom(name));
    if (led->flags & XkbIM_NoExplicit)
        fprintf(file, "        !allowExplicit;\n");
    if (led->flags & XkbIM_LEDDrivesKB)
        fprintf(file, "        indicatorDrivesKeyboard;\n");
    if (led->which_groups != 0) {
        if (led->which_groups != XkbIM_UseEffective) {
            fprintf(file, "        whichGroupState= %s;\n",
                    XkbIMWhichStateMaskText(led->which_groups));
        }
        fprintf(file, "        groups= 0x%02x;\n", led->groups);
    }
    if (led->which_mods != 0) {
        if (led->which_mods != XkbIM_UseEffective) {
            fprintf(file, "        whichModState= %s;\n",
                    XkbIMWhichStateMaskText(led->which_mods));
        }
        fprintf(file, "        modifiers= %s;\n",
                XkbVModMaskText(xkb,
                                led->mods.real_mods, led->mods.vmods));
    }
    if (led->ctrls != 0) {
        fprintf(file, "        controls= %s;\n",
                XkbControlsMaskText(led->ctrls));
    }
    if (addOn)
        (*addOn) (file, xkb, FALSE, TRUE, XkmIndicatorsIndex, priv);
    fprintf(file, "    };\n");
    return TRUE;
}

Bool
XkbWriteXKBCompatMap(FILE * file,
                     XkbDescPtr xkb,
                     Bool topLevel,
                     Bool showImplicit, XkbFileAddOnFunc addOn, void *priv)
{
    register unsigned i;
    XkbSymInterpretPtr interp;

    if ((!xkb) || (!xkb->compat) || (!xkb->compat->sym_interpret)) {
        return FALSE;
    }
    if ((xkb->names == NULL) || (xkb->names->compat == None))
        fprintf(file, "xkb_compatibility {\n\n");
    else
        fprintf(file, "xkb_compatibility \"%s\" {\n\n",
                XkbAtomText(xkb->names->compat));
    WriteXKBVModDecl(file, xkb,
                     (showImplicit ? VMOD_COMMENT_VALUE : VMOD_HIDE_VALUE));

    fprintf(file, "    interpret.useModMapMods= AnyLevel;\n");
    fprintf(file, "    interpret.repeat= FALSE;\n");
    fprintf(file, "    interpret.locking= FALSE;\n");
    interp = xkb->compat->sym_interpret;
    for (i = 0; i < xkb->compat->num_si; i++, interp++) {
        fprintf(file, "    interpret %s+%s(%s) {\n",
                ((interp->sym == NoSymbol) ? "Any" :
                 XkbKeysymText(interp->sym)),
                XkbSIMatchText(interp->match),
                XkbModMaskText(interp->mods));
        if (interp->virtual_mod != XkbNoModifier) {
            fprintf(file, "        virtualModifier= %s;\n",
                    XkbVModIndexText(xkb, interp->virtual_mod));
        }
        if (interp->match & XkbSI_LevelOneOnly)
            fprintf(file, "        useModMapMods=level1;\n");
        if (interp->flags & XkbSI_LockingKey)
            fprintf(file, "        locking= TRUE;\n");
        if (interp->flags & XkbSI_AutoRepeat)
            fprintf(file, "        repeat= TRUE;\n");
        fprintf(file, "        action= ");
        WriteXKBAction(file, xkb, &interp->act);
        fprintf(file, ";\n");
        fprintf(file, "    };\n");
    }
    for (i = 0; i < XkbNumKbdGroups; i++) {
        XkbModsPtr gc;

        gc = &xkb->compat->groups[i];
        if ((gc->real_mods == 0) && (gc->vmods == 0))
            continue;
        fprintf(file, "    group %d = %s;\n", i + 1, XkbVModMaskText(xkb,
                                                                     gc->
                                                                     real_mods,
                                                                     gc->vmods
                                                                     ));
    }
    if (xkb->indicators) {
        for (i = 0; i < XkbNumIndicators; i++) {
            XkbIndicatorMapPtr map = &xkb->indicators->maps[i];

            if ((map->flags != 0) || (map->which_groups != 0) ||
                (map->groups != 0) || (map->which_mods != 0) ||
                (map->mods.real_mods != 0) || (map->mods.vmods != 0) ||
                (map->ctrls != 0)) {
                WriteXKBIndicatorMap(file, xkb, xkb->names->indicators[i], map,
                                     addOn, priv);
            }
        }
    }
    if (addOn)
        (*addOn) (file, xkb, topLevel, showImplicit, XkmCompatMapIndex, priv);
    fprintf(file, "};\n\n");
    return TRUE;
}

Bool
XkbWriteXKBSymbols(FILE * file,
                   XkbDescPtr xkb,
                   Bool topLevel,
                   Bool showImplicit, XkbFileAddOnFunc addOn, void *priv)
{
    register unsigned i, tmp;
    XkbClientMapPtr map;
    XkbServerMapPtr srv;
    Bool showActions;

    if (!xkb) {
        return FALSE;
    }

    map = xkb->map;
    if ((!map) || (!map->syms) || (!map->key_sym_map)) {
        return FALSE;
    }
    if ((!xkb->names) || (!xkb->names->keys)) {
        return FALSE;
    }
    if ((xkb->names == NULL) || (xkb->names->symbols == None))
        fprintf(file, "xkb_symbols {\n\n");
    else
        fprintf(file, "xkb_symbols \"%s\" {\n\n",
                XkbAtomText(xkb->names->symbols));
    for (tmp = i = 0; i < XkbNumKbdGroups; i++) {
        if (xkb->names->groups[i] != None) {
            fprintf(file, "    name[group%d]=\"%s\";\n", i + 1,
                    XkbAtomText(xkb->names->groups[i]));
            tmp++;
        }
    }
    if (tmp > 0)
        fprintf(file, "\n");
    srv = xkb->server;
    for (i = xkb->min_key_code; i <= xkb->max_key_code; i++) {
        Bool simple;

        if ((int) XkbKeyNumSyms(xkb, i) < 1)
            continue;
        if (XkbFindKeycodeByName(xkb, xkb->names->keys[i].name, TRUE) != i)
            continue;
        simple = TRUE;
        fprintf(file, "    key %6s {",
                XkbKeyNameText(xkb->names->keys[i].name));
        if (srv->explicit) {
            if (((srv->explicit[i] & XkbExplicitKeyTypesMask) != 0) ||
                (showImplicit)) {
                int typeNdx, g;
                Bool multi;
                const char *comment = "  ";

                if ((srv->explicit[i] & XkbExplicitKeyTypesMask) == 0)
                    comment = "//";
                multi = FALSE;
                typeNdx = XkbKeyKeyTypeIndex(xkb, i, 0);
                for (g = 1; (g < XkbKeyNumGroups(xkb, i)) && (!multi); g++) {
                    if (XkbKeyKeyTypeIndex(xkb, i, g) != typeNdx)
                        multi = TRUE;
                }
                if (multi) {
                    for (g = 0; g < XkbKeyNumGroups(xkb, i); g++) {
                        typeNdx = XkbKeyKeyTypeIndex(xkb, i, g);
                        if (srv->explicit[i] & (1 << g)) {
                            fprintf(file, "\n%s      type[group%d]= \"%s\",",
                                    comment, g + 1,
                                    XkbAtomText(map->types[typeNdx].name));
                        }
                        else if (showImplicit) {
                            fprintf(file, "\n//      type[group%d]= \"%s\",",
                                    g + 1, XkbAtomText(map->types[typeNdx].name
                                                       ));
                        }
                    }
                }
                else {
                    fprintf(file, "\n%s      type= \"%s\",", comment,
                            XkbAtomText(map->types[typeNdx].name));
                }
                simple = FALSE;
            }
            if (((srv->explicit[i] & XkbExplicitAutoRepeatMask) != 0) &&
                (xkb->ctrls != NULL)) {
                if (xkb->ctrls->per_key_repeat[i / 8] & (1 << (i % 8)))
                    fprintf(file, "\n        repeat= Yes,");
                else
                    fprintf(file, "\n        repeat= No,");
                simple = FALSE;
            }
            if ((xkb->server != NULL) && (xkb->server->vmodmap != NULL) &&
                (xkb->server->vmodmap[i] != 0)) {
                if ((srv->explicit[i] & XkbExplicitVModMapMask) != 0) {
                    fprintf(file, "\n        virtualMods= %s,",
                            XkbVModMaskText(xkb, 0,
                                            xkb->server->vmodmap[i]));
                }
                else if (showImplicit) {
                    fprintf(file, "\n//      virtualMods= %s,",
                            XkbVModMaskText(xkb, 0, xkb->server->vmodmap[i]));
                }
            }
        }
        switch (XkbOutOfRangeGroupAction(XkbKeyGroupInfo(xkb, i))) {
        case XkbClampIntoRange:
            fprintf(file, "\n        groupsClamp,");
            break;
        case XkbRedirectIntoRange:
            fprintf(file, "\n        groupsRedirect= Group%d,",
                    XkbOutOfRangeGroupNumber(XkbKeyGroupInfo(xkb, i)) + 1);
            break;
        }
        if (srv->behaviors != NULL) {
            unsigned type;

            type = srv->behaviors[i].type & XkbKB_OpMask;

            if (type != XkbKB_Default) {
                simple = FALSE;
                fprintf(file, "\n        %s,",
                        XkbBehaviorText(xkb, &srv->behaviors[i]));
            }
        }
        if ((srv->explicit == NULL) || showImplicit ||
            ((srv->explicit[i] & XkbExplicitInterpretMask) != 0))
            showActions = XkbKeyHasActions(xkb, i);
        else
            showActions = FALSE;

        if (((unsigned) XkbKeyNumGroups(xkb, i) > 1) || showActions)
            simple = FALSE;
        if (simple) {
            KeySym *syms;
            unsigned s;

            syms = XkbKeySymsPtr(xkb, i);
            fprintf(file, "         [ ");
            for (s = 0; s < XkbKeyGroupWidth(xkb, i, XkbGroup1Index); s++) {
                if (s != 0)
                    fprintf(file, ", ");
                fprintf(file, "%15s", XkbKeysymText(*syms++));
            }
            fprintf(file, " ] };\n");
        }
        else {
            unsigned g, s;
            KeySym *syms;
            XkbAction *acts;

            syms = XkbKeySymsPtr(xkb, i);
            acts = XkbKeyActionsPtr(xkb, i);
            for (g = 0; g < XkbKeyNumGroups(xkb, i); g++) {
                if (g != 0)
                    fprintf(file, ",");
                fprintf(file, "\n        symbols[Group%d]= [ ", g + 1);
                for (s = 0; s < XkbKeyGroupWidth(xkb, i, g); s++) {
                    if (s != 0)
                        fprintf(file, ", ");
                    fprintf(file, "%15s", XkbKeysymText(syms[s]));
                }
                fprintf(file, " ]");
                syms += XkbKeyGroupsWidth(xkb, i);
                if (showActions) {
                    fprintf(file, ",\n        actions[Group%d]= [ ", g + 1);
                    for (s = 0; s < XkbKeyGroupWidth(xkb, i, g); s++) {
                        if (s != 0)
                            fprintf(file, ", ");
                        WriteXKBAction(file, xkb, (XkbAnyAction *) &acts[s]);
                    }
                    fprintf(file, " ]");
                    acts += XkbKeyGroupsWidth(xkb, i);
                }
            }
            fprintf(file, "\n    };\n");
        }
    }
    if (map && map->modmap) {
        for (i = xkb->min_key_code; i <= xkb->max_key_code; i++) {
            if (map->modmap[i] != 0) {
                register int n, bit;

                for (bit = 1, n = 0; n < XkbNumModifiers; n++, bit <<= 1) {
                    if (map->modmap[i] & bit) {
                        char buf[5];

                        memcpy(buf, xkb->names->keys[i].name, 4);
                        buf[4] = '\0';
                        fprintf(file, "    modifier_map %s { <%s> };\n",
                                XkbModIndexText(n), buf);
                    }
                }
            }
        }
    }
    if (addOn)
        (*addOn) (file, xkb, topLevel, showImplicit, XkmSymbolsIndex, priv);
    fprintf(file, "};\n\n");
    return TRUE;
}

static Bool
WriteXKBOutline(FILE * file,
                XkbShapePtr shape,
                XkbOutlinePtr outline, int lastRadius, int first, int indent)
{
    register int i;
    XkbPointPtr pt;
    char *iStr;

    fprintf(file, "%s", iStr = XkbIndentText(first));
    if (first != indent)
        iStr = XkbIndentText(indent);
    if (outline->corner_radius != lastRadius) {
        fprintf(file, "corner= %s,", XkbGeomFPText(outline->corner_radius));
        if (shape != NULL) {
            fprintf(file, "\n%s", iStr);
        }
    }
    if (shape) {
        if (outline == shape->approx)
            fprintf(file, "approx= ");
        else if (outline == shape->primary)
            fprintf(file, "primary= ");
    }
    fprintf(file, "{");
    for (pt = outline->points, i = 0; i < outline->num_points; i++, pt++) {
        if (i == 0)
            fprintf(file, " ");
        else if ((i % 4) == 0)
            fprintf(file, ",\n%s  ", iStr);
        else
            fprintf(file, ", ");
        fprintf(file, "[ %3s, %3s ]", XkbGeomFPText(pt->x),
                XkbGeomFPText(pt->y));
    }
    fprintf(file, " }");
    return TRUE;
}

static Bool
WriteXKBDoodad(FILE * file,
               unsigned indent, XkbGeometryPtr geom, XkbDoodadPtr doodad)
{
    register char *i_str;
    XkbShapePtr shape;
    XkbColorPtr color;

    i_str = XkbIndentText(indent);
    fprintf(file, "%s%s \"%s\" {\n", i_str, XkbDoodadTypeText(doodad->any.type),
            XkbAtomText(doodad->any.name));
    fprintf(file, "%s    top=      %s;\n", i_str,
            XkbGeomFPText(doodad->any.top));
    fprintf(file, "%s    left=     %s;\n", i_str,
            XkbGeomFPText(doodad->any.left));
    fprintf(file, "%s    priority= %d;\n", i_str, doodad->any.priority);
    switch (doodad->any.type) {
    case XkbOutlineDoodad:
    case XkbSolidDoodad:
        if (doodad->shape.angle != 0) {
            fprintf(file, "%s    angle=  %s;\n", i_str,
                    XkbGeomFPText(doodad->shape.angle));
        }
        if (doodad->shape.color_ndx != 0) {
            fprintf(file, "%s    color= \"%s\";\n", i_str,
                    XkbShapeDoodadColor(geom, &doodad->shape)->spec);
        }
        shape = XkbShapeDoodadShape(geom, &doodad->shape);
        fprintf(file, "%s    shape= \"%s\";\n", i_str,
                XkbAtomText(shape->name));
        break;
    case XkbTextDoodad:
        if (doodad->text.angle != 0) {
            fprintf(file, "%s    angle=  %s;\n", i_str,
                    XkbGeomFPText(doodad->text.angle));
        }
        if (doodad->text.width != 0) {
            fprintf(file, "%s    width=  %s;\n", i_str,
                    XkbGeomFPText(doodad->text.width));

        }
        if (doodad->text.height != 0) {
            fprintf(file, "%s    height=  %s;\n", i_str,
                    XkbGeomFPText(doodad->text.height));

        }
        if (doodad->text.color_ndx != 0) {
            color = XkbTextDoodadColor(geom, &doodad->text);
            fprintf(file, "%s    color= \"%s\";\n", i_str,
                    XkbStringText(color->spec));
        }
        fprintf(file, "%s    XFont= \"%s\";\n", i_str,
                XkbStringText(doodad->text.font));
        fprintf(file, "%s    text=  \"%s\";\n", i_str,
                XkbStringText(doodad->text.text));
        break;
    case XkbIndicatorDoodad:
        shape = XkbIndicatorDoodadShape(geom, &doodad->indicator);
        color = XkbIndicatorDoodadOnColor(geom, &doodad->indicator);
        fprintf(file, "%s    onColor= \"%s\";\n", i_str,
                XkbStringText(color->spec));
        color = XkbIndicatorDoodadOffColor(geom, &doodad->indicator);
        fprintf(file, "%s    offColor= \"%s\";\n", i_str,
                XkbStringText(color->spec));
        fprintf(file, "%s    shape= \"%s\";\n", i_str,
                XkbAtomText(shape->name));
        break;
    case XkbLogoDoodad:
        fprintf(file, "%s    logoName= \"%s\";\n", i_str,
                XkbStringText(doodad->logo.logo_name));
        if (doodad->shape.angle != 0) {
            fprintf(file, "%s    angle=  %s;\n", i_str,
                    XkbGeomFPText(doodad->logo.angle));
        }
        if (doodad->shape.color_ndx != 0) {
            fprintf(file, "%s    color= \"%s\";\n", i_str,
                    XkbLogoDoodadColor(geom, &doodad->logo)->spec);
        }
        shape = XkbLogoDoodadShape(geom, &doodad->logo);
        fprintf(file, "%s    shape= \"%s\";\n", i_str,
                XkbAtomText(shape->name));
        break;
    }
    fprintf(file, "%s};\n", i_str);
    return TRUE;
}

 /*ARGSUSED*/ static Bool
WriteXKBOverlay(FILE * file,
                unsigned indent, XkbGeometryPtr geom, XkbOverlayPtr ol)
{
    register char *i_str;
    int r, k, nOut;
    XkbOverlayRowPtr row;
    XkbOverlayKeyPtr key;

    i_str = XkbIndentText(indent);
    if (ol->name != None) {
        fprintf(file, "%soverlay \"%s\" {\n", i_str, XkbAtomText(ol->name));
    }
    else
        fprintf(file, "%soverlay {\n", i_str);
    for (nOut = r = 0, row = ol->rows; r < ol->num_rows; r++, row++) {
        for (k = 0, key = row->keys; k < row->num_keys; k++, key++) {
            char *over, *under;

            over = XkbKeyNameText(key->over.name);
            under = XkbKeyNameText(key->under.name);
            if (nOut == 0)
                fprintf(file, "%s    %6s=%6s", i_str, under, over);
            else if ((nOut % 4) == 0)
                fprintf(file, ",\n%s    %6s=%6s", i_str, under, over);
            else
                fprintf(file, ", %6s=%6s", under, over);
            nOut++;
        }
    }
    fprintf(file, "\n%s};\n", i_str);
    return TRUE;
}

static Bool
WriteXKBSection(FILE * file, XkbSectionPtr s, XkbGeometryPtr geom)
{
    register int i;
    XkbRowPtr row;
    int dfltKeyColor = 0;

    fprintf(file, "    section \"%s\" {\n", XkbAtomText(s->name));
    if (s->rows && (s->rows->num_keys > 0)) {
        dfltKeyColor = s->rows->keys[0].color_ndx;
        fprintf(file, "        key.color= \"%s\";\n",
                XkbStringText(geom->colors[dfltKeyColor].spec));
    }
    fprintf(file, "        priority=  %d;\n", s->priority);
    fprintf(file, "        top=       %s;\n", XkbGeomFPText(s->top));
    fprintf(file, "        left=      %s;\n", XkbGeomFPText(s->left));
    fprintf(file, "        width=     %s;\n", XkbGeomFPText(s->width));
    fprintf(file, "        height=    %s;\n", XkbGeomFPText(s->height));
    if (s->angle != 0) {
        fprintf(file, "        angle=  %s;\n", XkbGeomFPText(s->angle));
    }
    for (i = 0, row = s->rows; i < s->num_rows; i++, row++) {
        fprintf(file, "        row {\n");
        fprintf(file, "            top=  %s;\n", XkbGeomFPText(row->top));
        fprintf(file, "            left= %s;\n", XkbGeomFPText(row->left));
        if (row->vertical)
            fprintf(file, "            vertical;\n");
        if (row->num_keys > 0) {
            register int k;
            register XkbKeyPtr key;
            int forceNL = 0;
            int nThisLine = 0;

            fprintf(file, "            keys {\n");
            for (k = 0, key = row->keys; k < row->num_keys; k++, key++) {
                XkbShapePtr shape;

                if (key->color_ndx != dfltKeyColor)
                    forceNL = 1;
                if (k == 0) {
                    fprintf(file, "                ");
                    nThisLine = 0;
                }
                else if (((nThisLine % 2) == 1) || (forceNL)) {
                    fprintf(file, ",\n                ");
                    forceNL = nThisLine = 0;
                }
                else {
                    fprintf(file, ", ");
                    nThisLine++;
                }
                shape = XkbKeyShape(geom, key);
                fprintf(file, "{ %6s, \"%s\", %3s",
                        XkbKeyNameText(key->name.name),
                        XkbAtomText(shape->name),
                        XkbGeomFPText(key->gap));
                if (key->color_ndx != dfltKeyColor) {
                    fprintf(file, ", color=\"%s\"",
                            XkbKeyColor(geom, key)->spec);
                    forceNL = 1;
                }
                fprintf(file, " }");
            }
            fprintf(file, "\n            };\n");
        }
        fprintf(file, "        };\n");
    }
    if (s->doodads != NULL) {
        XkbDoodadPtr doodad;

        for (i = 0, doodad = s->doodads; i < s->num_doodads; i++, doodad++) {
            WriteXKBDoodad(file, 8, geom, doodad);
        }
    }
    if (s->overlays != NULL) {
        XkbOverlayPtr ol;

        for (i = 0, ol = s->overlays; i < s->num_overlays; i++, ol++) {
            WriteXKBOverlay(file, 8, geom, ol);
        }
    }
    fprintf(file, "    }; // End of \"%s\" section\n\n", XkbAtomText(s->name));
    return TRUE;
}

Bool
XkbWriteXKBGeometry(FILE * file,
                    XkbDescPtr xkb,
                    Bool topLevel,
                    Bool showImplicit, XkbFileAddOnFunc addOn, void *priv)
{
    register unsigned i, n;
    XkbGeometryPtr geom;

    if ((!xkb) || (!xkb->geom)) {
        return FALSE;
    }
    geom = xkb->geom;
    if (geom->name == None)
        fprintf(file, "xkb_geometry {\n\n");
    else
        fprintf(file, "xkb_geometry \"%s\" {\n\n", XkbAtomText(geom->name));
    fprintf(file, "    width=       %s;\n", XkbGeomFPText(geom->width_mm));
    fprintf(file, "    height=      %s;\n\n", XkbGeomFPText(geom->height_mm));

    if (geom->key_aliases != NULL) {
        XkbKeyAliasPtr pAl;

        pAl = geom->key_aliases;
        for (i = 0; i < geom->num_key_aliases; i++, pAl++) {
            fprintf(file, "    alias %6s = %6s;\n",
                    XkbKeyNameText(pAl->alias),
                    XkbKeyNameText(pAl->real));
        }
        fprintf(file, "\n");
    }

    if (geom->base_color != NULL)
        fprintf(file, "    baseColor=   \"%s\";\n",
                XkbStringText(geom->base_color->spec));
    if (geom->label_color != NULL)
        fprintf(file, "    labelColor=  \"%s\";\n",
                XkbStringText(geom->label_color->spec));
    if (geom->label_font != NULL)
        fprintf(file, "    xfont=       \"%s\";\n",
                XkbStringText(geom->label_font));
    if ((geom->num_colors > 0) && (showImplicit)) {
        XkbColorPtr color;

        for (color = geom->colors, i = 0; i < geom->num_colors; i++, color++) {
            fprintf(file, "//     color[%d]= \"%s\"\n", i,
                    XkbStringText(color->spec));
        }
        fprintf(file, "\n");
    }
    if (geom->num_properties > 0) {
        XkbPropertyPtr prop;

        for (prop = geom->properties, i = 0; i < geom->num_properties;
             i++, prop++) {
            fprintf(file, "    %s= \"%s\";\n", prop->name,
                    XkbStringText(prop->value));
        }
        fprintf(file, "\n");
    }
    if (geom->num_shapes > 0) {
        XkbShapePtr shape;
        XkbOutlinePtr outline;
        int lastR;

        for (shape = geom->shapes, i = 0; i < geom->num_shapes; i++, shape++) {
            lastR = 0;
            fprintf(file, "    shape \"%s\" {",
                    XkbAtomText(shape->name));
            outline = shape->outlines;
            if (shape->num_outlines > 1) {
                for (n = 0; n < shape->num_outlines; n++, outline++) {
                    if (n == 0)
                        fprintf(file, "\n");
                    else
                        fprintf(file, ",\n");
                    WriteXKBOutline(file, shape, outline, lastR, 8, 8);
                    lastR = outline->corner_radius;
                }
                fprintf(file, "\n    };\n");
            }
            else {
                WriteXKBOutline(file, NULL, outline, lastR, 1, 8);
                fprintf(file, " };\n");
            }
        }
    }
    if (geom->num_sections > 0) {
        XkbSectionPtr section;

        for (section = geom->sections, i = 0; i < geom->num_sections;
             i++, section++) {
            WriteXKBSection(file, section, geom);
        }
    }
    if (geom->num_doodads > 0) {
        XkbDoodadPtr doodad;

        for (i = 0, doodad = geom->doodads; i < geom->num_doodads;
             i++, doodad++) {
            WriteXKBDoodad(file, 4, geom, doodad);
        }
    }
    if (addOn)
        (*addOn) (file, xkb, topLevel, showImplicit, XkmGeometryIndex, priv);
    fprintf(file, "};\n\n");
    return TRUE;
}
