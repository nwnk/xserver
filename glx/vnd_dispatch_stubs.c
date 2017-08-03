
#include <dix-config.h>
#include <dix.h>
#include "vndserver.h"

// HACK: The opcode in old glxproto.h has a typo in it.
#if !defined(X_GLXCreateContextAttribsARB)
#define X_GLXCreateContextAttribsARB X_GLXCreateContextAtrribsARB
#endif

static int dispatch_Render(ClientPtr client)
{
    REQUEST(xGLXRenderReq);
    REQUEST_AT_LEAST_SIZE(*stuff);
    CARD32 contextTag = GlxCheckSwap(client, stuff->contextTag);
    GlxServerVendor *vendor = NULL;
    if (!glxServer.getContextTag(client, contextTag, &vendor, NULL)) {
        vendor = NULL;
    }
    if (vendor != NULL) {
        int ret;
        ret = glxServer.forwardRequest(vendor, client);
        return ret;
    } else {
        client->errorValue = contextTag;
        return GlxErrorBase + GLXBadContextTag;
    }
}
static int dispatch_RenderLarge(ClientPtr client)
{
    REQUEST(xGLXRenderLargeReq);
    REQUEST_AT_LEAST_SIZE(*stuff);
    CARD32 contextTag = GlxCheckSwap(client, stuff->contextTag);
    GlxServerVendor *vendor = NULL;
    if (!glxServer.getContextTag(client, contextTag, &vendor, NULL)) {
        vendor = NULL;
    }
    if (vendor != NULL) {
        int ret;
        ret = glxServer.forwardRequest(vendor, client);
        return ret;
    } else {
        client->errorValue = contextTag;
        return GlxErrorBase + GLXBadContextTag;
    }
}
static int dispatch_CreateContext(ClientPtr client)
{
    REQUEST(xGLXCreateContextReq);
    REQUEST_SIZE_MATCH(*stuff);
    CARD32 screen = GlxCheckSwap(client, stuff->screen);
    GlxServerVendor *vendor = NULL;
    CARD32 context = GlxCheckSwap(client, stuff->context);
    LEGAL_NEW_RESOURCE(context, client);
    if (screen < screenInfo.numScreens) {
        vendor = glxServer.getVendorForScreen(client, screenInfo.screens[screen]);
    }
    if (vendor != NULL) {
        int ret;
        if (!glxServer.addXIDMap(context, vendor)) {
            return BadAlloc;
        }
        ret = glxServer.forwardRequest(vendor, client);
        if (ret != Success) {
            glxServer.removeXIDMap(context);
        }
        return ret;
    } else {
        client->errorValue = screen;
        return BadMatch;
    }
}
static int dispatch_DestroyContext(ClientPtr client)
{
    REQUEST(xGLXDestroyContextReq);
    REQUEST_SIZE_MATCH(*stuff);
    CARD32 context = GlxCheckSwap(client, stuff->context);
    GlxServerVendor *vendor = NULL;
    vendor = glxServer.getXIDMap(context);
    if (vendor != NULL) {
        int ret;
        ret = glxServer.forwardRequest(vendor, client);
        if (ret == Success) {
            glxServer.removeXIDMap(context);
        }
        return ret;
    } else {
        client->errorValue = context;
        return GlxErrorBase + GLXBadContext;
    }
}
static int dispatch_WaitGL(ClientPtr client)
{
    REQUEST(xGLXWaitGLReq);
    REQUEST_SIZE_MATCH(*stuff);
    CARD32 contextTag = GlxCheckSwap(client, stuff->contextTag);
    GlxServerVendor *vendor = NULL;
    if (!glxServer.getContextTag(client, contextTag, &vendor, NULL)) {
        vendor = NULL;
    }
    if (vendor != NULL) {
        int ret;
        ret = glxServer.forwardRequest(vendor, client);
        return ret;
    } else {
        client->errorValue = contextTag;
        return GlxErrorBase + GLXBadContextTag;
    }
}
static int dispatch_WaitX(ClientPtr client)
{
    REQUEST(xGLXWaitXReq);
    REQUEST_SIZE_MATCH(*stuff);
    CARD32 contextTag = GlxCheckSwap(client, stuff->contextTag);
    GlxServerVendor *vendor = NULL;
    if (!glxServer.getContextTag(client, contextTag, &vendor, NULL)) {
        vendor = NULL;
    }
    if (vendor != NULL) {
        int ret;
        ret = glxServer.forwardRequest(vendor, client);
        return ret;
    } else {
        client->errorValue = contextTag;
        return GlxErrorBase + GLXBadContextTag;
    }
}
static int dispatch_UseXFont(ClientPtr client)
{
    REQUEST(xGLXUseXFontReq);
    REQUEST_SIZE_MATCH(*stuff);
    CARD32 contextTag = GlxCheckSwap(client, stuff->contextTag);
    GlxServerVendor *vendor = NULL;
    if (!glxServer.getContextTag(client, contextTag, &vendor, NULL)) {
        vendor = NULL;
    }
    if (vendor != NULL) {
        int ret;
        ret = glxServer.forwardRequest(vendor, client);
        return ret;
    } else {
        client->errorValue = contextTag;
        return GlxErrorBase + GLXBadContextTag;
    }
}
static int dispatch_CreateGLXPixmap(ClientPtr client)
{
    REQUEST(xGLXCreateGLXPixmapReq);
    REQUEST_SIZE_MATCH(*stuff);
    CARD32 screen = GlxCheckSwap(client, stuff->screen);
    GlxServerVendor *vendor = NULL;
    CARD32 glxpixmap = GlxCheckSwap(client, stuff->glxpixmap);
    LEGAL_NEW_RESOURCE(glxpixmap, client);
    if (screen < screenInfo.numScreens) {
        vendor = glxServer.getVendorForScreen(client, screenInfo.screens[screen]);
    }
    if (vendor != NULL) {
        int ret;
        if (!glxServer.addXIDMap(glxpixmap, vendor)) {
            return BadAlloc;
        }
        ret = glxServer.forwardRequest(vendor, client);
        if (ret != Success) {
            glxServer.removeXIDMap(glxpixmap);
        }
        return ret;
    } else {
        client->errorValue = screen;
        return BadMatch;
    }
}
static int dispatch_GetVisualConfigs(ClientPtr client)
{
    REQUEST(xGLXGetVisualConfigsReq);
    REQUEST_SIZE_MATCH(*stuff);
    CARD32 screen = GlxCheckSwap(client, stuff->screen);
    GlxServerVendor *vendor = NULL;
    if (screen < screenInfo.numScreens) {
        vendor = glxServer.getVendorForScreen(client, screenInfo.screens[screen]);
    }
    if (vendor != NULL) {
        int ret;
        ret = glxServer.forwardRequest(vendor, client);
        return ret;
    } else {
        client->errorValue = screen;
        return BadMatch;
    }
}
static int dispatch_DestroyGLXPixmap(ClientPtr client)
{
    REQUEST(xGLXDestroyGLXPixmapReq);
    REQUEST_SIZE_MATCH(*stuff);
    CARD32 glxpixmap = GlxCheckSwap(client, stuff->glxpixmap);
    GlxServerVendor *vendor = NULL;
    vendor = glxServer.getXIDMap(glxpixmap);
    if (vendor != NULL) {
        int ret;
        ret = glxServer.forwardRequest(vendor, client);
        return ret;
    } else {
        client->errorValue = glxpixmap;
        return GlxErrorBase + GLXBadPixmap;
    }
}
static int dispatch_QueryExtensionsString(ClientPtr client)
{
    REQUEST(xGLXQueryExtensionsStringReq);
    REQUEST_SIZE_MATCH(*stuff);
    CARD32 screen = GlxCheckSwap(client, stuff->screen);
    GlxServerVendor *vendor = NULL;
    if (screen < screenInfo.numScreens) {
        vendor = glxServer.getVendorForScreen(client, screenInfo.screens[screen]);
    }
    if (vendor != NULL) {
        int ret;
        ret = glxServer.forwardRequest(vendor, client);
        return ret;
    } else {
        client->errorValue = screen;
        return BadMatch;
    }
}
static int dispatch_QueryServerString(ClientPtr client)
{
    REQUEST(xGLXQueryServerStringReq);
    REQUEST_SIZE_MATCH(*stuff);
    CARD32 screen = GlxCheckSwap(client, stuff->screen);
    GlxServerVendor *vendor = NULL;
    if (screen < screenInfo.numScreens) {
        vendor = glxServer.getVendorForScreen(client, screenInfo.screens[screen]);
    }
    if (vendor != NULL) {
        int ret;
        ret = glxServer.forwardRequest(vendor, client);
        return ret;
    } else {
        client->errorValue = screen;
        return BadMatch;
    }
}
static int dispatch_ChangeDrawableAttributes(ClientPtr client)
{
    REQUEST(xGLXChangeDrawableAttributesReq);
    REQUEST_AT_LEAST_SIZE(*stuff);
    CARD32 drawable = GlxCheckSwap(client, stuff->drawable);
    GlxServerVendor *vendor = NULL;
    vendor = glxServer.getXIDMap(drawable);
    if (vendor != NULL) {
        int ret;
        ret = glxServer.forwardRequest(vendor, client);
        return ret;
    } else {
        client->errorValue = drawable;
        return BadDrawable;
    }
}
static int dispatch_CreateNewContext(ClientPtr client)
{
    REQUEST(xGLXCreateNewContextReq);
    REQUEST_SIZE_MATCH(*stuff);
    CARD32 screen = GlxCheckSwap(client, stuff->screen);
    GlxServerVendor *vendor = NULL;
    CARD32 context = GlxCheckSwap(client, stuff->context);
    LEGAL_NEW_RESOURCE(context, client);
    if (screen < screenInfo.numScreens) {
        vendor = glxServer.getVendorForScreen(client, screenInfo.screens[screen]);
    }
    if (vendor != NULL) {
        int ret;
        if (!glxServer.addXIDMap(context, vendor)) {
            return BadAlloc;
        }
        ret = glxServer.forwardRequest(vendor, client);
        if (ret != Success) {
            glxServer.removeXIDMap(context);
        }
        return ret;
    } else {
        client->errorValue = screen;
        return BadMatch;
    }
}
static int dispatch_CreatePbuffer(ClientPtr client)
{
    REQUEST(xGLXCreatePbufferReq);
    REQUEST_AT_LEAST_SIZE(*stuff);
    CARD32 screen = GlxCheckSwap(client, stuff->screen);
    GlxServerVendor *vendor = NULL;
    CARD32 pbuffer = GlxCheckSwap(client, stuff->pbuffer);
    LEGAL_NEW_RESOURCE(pbuffer, client);
    if (screen < screenInfo.numScreens) {
        vendor = glxServer.getVendorForScreen(client, screenInfo.screens[screen]);
    }
    if (vendor != NULL) {
        int ret;
        if (!glxServer.addXIDMap(pbuffer, vendor)) {
            return BadAlloc;
        }
        ret = glxServer.forwardRequest(vendor, client);
        if (ret != Success) {
            glxServer.removeXIDMap(pbuffer);
        }
        return ret;
    } else {
        client->errorValue = screen;
        return BadMatch;
    }
}
static int dispatch_CreatePixmap(ClientPtr client)
{
    REQUEST(xGLXCreatePixmapReq);
    REQUEST_AT_LEAST_SIZE(*stuff);
    CARD32 screen = GlxCheckSwap(client, stuff->screen);
    GlxServerVendor *vendor = NULL;
    CARD32 glxpixmap = GlxCheckSwap(client, stuff->glxpixmap);
    LEGAL_NEW_RESOURCE(glxpixmap, client);
    if (screen < screenInfo.numScreens) {
        vendor = glxServer.getVendorForScreen(client, screenInfo.screens[screen]);
    }
    if (vendor != NULL) {
        int ret;
        if (!glxServer.addXIDMap(glxpixmap, vendor)) {
            return BadAlloc;
        }
        ret = glxServer.forwardRequest(vendor, client);
        if (ret != Success) {
            glxServer.removeXIDMap(glxpixmap);
        }
        return ret;
    } else {
        client->errorValue = screen;
        return BadMatch;
    }
}
static int dispatch_CreateWindow(ClientPtr client)
{
    REQUEST(xGLXCreateWindowReq);
    REQUEST_AT_LEAST_SIZE(*stuff);
    CARD32 screen = GlxCheckSwap(client, stuff->screen);
    GlxServerVendor *vendor = NULL;
    CARD32 glxwindow = GlxCheckSwap(client, stuff->glxwindow);
    LEGAL_NEW_RESOURCE(glxwindow, client);
    if (screen < screenInfo.numScreens) {
        vendor = glxServer.getVendorForScreen(client, screenInfo.screens[screen]);
    }
    if (vendor != NULL) {
        int ret;
        if (!glxServer.addXIDMap(glxwindow, vendor)) {
            return BadAlloc;
        }
        ret = glxServer.forwardRequest(vendor, client);
        if (ret != Success) {
            glxServer.removeXIDMap(glxwindow);
        }
        return ret;
    } else {
        client->errorValue = screen;
        return BadMatch;
    }
}
static int dispatch_CreateContextAttribsARB(ClientPtr client)
{
    REQUEST(xGLXCreateContextAttribsARBReq);
    REQUEST_AT_LEAST_SIZE(*stuff);
    CARD32 screen = GlxCheckSwap(client, stuff->screen);
    GlxServerVendor *vendor = NULL;
    CARD32 context = GlxCheckSwap(client, stuff->context);
    LEGAL_NEW_RESOURCE(context, client);
    if (screen < screenInfo.numScreens) {
        vendor = glxServer.getVendorForScreen(client, screenInfo.screens[screen]);
    }
    if (vendor != NULL) {
        int ret;
        if (!glxServer.addXIDMap(context, vendor)) {
            return BadAlloc;
        }
        ret = glxServer.forwardRequest(vendor, client);
        if (ret != Success) {
            glxServer.removeXIDMap(context);
        }
        return ret;
    } else {
        client->errorValue = screen;
        return BadMatch;
    }
}
static int dispatch_DestroyPbuffer(ClientPtr client)
{
    REQUEST(xGLXDestroyPbufferReq);
    REQUEST_SIZE_MATCH(*stuff);
    CARD32 pbuffer = GlxCheckSwap(client, stuff->pbuffer);
    GlxServerVendor *vendor = NULL;
    vendor = glxServer.getXIDMap(pbuffer);
    if (vendor != NULL) {
        int ret;
        ret = glxServer.forwardRequest(vendor, client);
        if (ret == Success) {
            glxServer.removeXIDMap(pbuffer);
        }
        return ret;
    } else {
        client->errorValue = pbuffer;
        return GlxErrorBase + GLXBadPbuffer;
    }
}
static int dispatch_DestroyPixmap(ClientPtr client)
{
    REQUEST(xGLXDestroyPixmapReq);
    REQUEST_SIZE_MATCH(*stuff);
    CARD32 glxpixmap = GlxCheckSwap(client, stuff->glxpixmap);
    GlxServerVendor *vendor = NULL;
    vendor = glxServer.getXIDMap(glxpixmap);
    if (vendor != NULL) {
        int ret;
        ret = glxServer.forwardRequest(vendor, client);
        if (ret == Success) {
            glxServer.removeXIDMap(glxpixmap);
        }
        return ret;
    } else {
        client->errorValue = glxpixmap;
        return GlxErrorBase + GLXBadPixmap;
    }
}
static int dispatch_DestroyWindow(ClientPtr client)
{
    REQUEST(xGLXDestroyWindowReq);
    REQUEST_SIZE_MATCH(*stuff);
    CARD32 glxwindow = GlxCheckSwap(client, stuff->glxwindow);
    GlxServerVendor *vendor = NULL;
    vendor = glxServer.getXIDMap(glxwindow);
    if (vendor != NULL) {
        int ret;
        ret = glxServer.forwardRequest(vendor, client);
        if (ret == Success) {
            glxServer.removeXIDMap(glxwindow);
        }
        return ret;
    } else {
        client->errorValue = glxwindow;
        return GlxErrorBase + GLXBadWindow;
    }
}
static int dispatch_GetDrawableAttributes(ClientPtr client)
{
    REQUEST(xGLXGetDrawableAttributesReq);
    REQUEST_SIZE_MATCH(*stuff);
    CARD32 drawable = GlxCheckSwap(client, stuff->drawable);
    GlxServerVendor *vendor = NULL;
    vendor = glxServer.getXIDMap(drawable);
    if (vendor != NULL) {
        int ret;
        ret = glxServer.forwardRequest(vendor, client);
        return ret;
    } else {
        client->errorValue = drawable;
        return BadDrawable;
    }
}
static int dispatch_GetFBConfigs(ClientPtr client)
{
    REQUEST(xGLXGetFBConfigsReq);
    REQUEST_SIZE_MATCH(*stuff);
    CARD32 screen = GlxCheckSwap(client, stuff->screen);
    GlxServerVendor *vendor = NULL;
    if (screen < screenInfo.numScreens) {
        vendor = glxServer.getVendorForScreen(client, screenInfo.screens[screen]);
    }
    if (vendor != NULL) {
        int ret;
        ret = glxServer.forwardRequest(vendor, client);
        return ret;
    } else {
        client->errorValue = screen;
        return BadMatch;
    }
}
static int dispatch_QueryContext(ClientPtr client)
{
    REQUEST(xGLXQueryContextReq);
    REQUEST_SIZE_MATCH(*stuff);
    CARD32 context = GlxCheckSwap(client, stuff->context);
    GlxServerVendor *vendor = NULL;
    vendor = glxServer.getXIDMap(context);
    if (vendor != NULL) {
        int ret;
        ret = glxServer.forwardRequest(vendor, client);
        return ret;
    } else {
        client->errorValue = context;
        return GlxErrorBase + GLXBadContext;
    }
}
static int dispatch_IsDirect(ClientPtr client)
{
    REQUEST(xGLXIsDirectReq);
    REQUEST_SIZE_MATCH(*stuff);
    CARD32 context = GlxCheckSwap(client, stuff->context);
    GlxServerVendor *vendor = NULL;
    vendor = glxServer.getXIDMap(context);
    if (vendor != NULL) {
        int ret;
        ret = glxServer.forwardRequest(vendor, client);
        return ret;
    } else {
        client->errorValue = context;
        return GlxErrorBase + GLXBadContext;
    }
}
