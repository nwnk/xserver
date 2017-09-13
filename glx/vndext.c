/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * unaltered in all copies or substantial portions of the Materials.
 * Any additions, deletions, or changes to the original source files
 * must be clearly indicated in accompanying documentation.
 *
 * If only executable code is distributed, then the accompanying
 * documentation must state that "this software is based in part on the
 * work of the Khronos Group."
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 */

#include "vndserver.h"

#include <string.h>
#include <scrnintstr.h>
#include <windowstr.h>
#include <dixstruct.h>
#include <extnsionst.h>

#include <GL/glxproto.h>
#include "vndservervendor.h"

int GlxErrorBase = 0;
static CallbackListPtr vndInitCallbackList;
static DevPrivateKeyRec glvXGLVScreenPrivKey;
static DevPrivateKeyRec glvXGLVClientPrivKey;

// The resource type used to keep track of the vendor library for XID's.
RESTYPE idResource;

static int idResourceDeleteCallback(void *value, XID id)
{
    return 0;
}

static GlxScreenPriv *xglvGetScreenPrivate(ScreenPtr pScreen)
{
    return dixLookupPrivate(&pScreen->devPrivates, &glvXGLVScreenPrivKey);
}

static void xglvSetScreenPrivate(ScreenPtr pScreen, void *priv)
{
    dixSetPrivate(&pScreen->devPrivates, &glvXGLVScreenPrivKey, priv);
}

GlxScreenPriv *GlxGetScreen(ScreenPtr pScreen)
{
    if (pScreen != NULL) {
        GlxScreenPriv *priv = xglvGetScreenPrivate(pScreen);
        if (priv == NULL) {
            priv = calloc(1, sizeof(GlxScreenPriv));
            if (priv == NULL) {
                return NULL;
            }

            xglvSetScreenPrivate(pScreen, priv);
        }
        return priv;
    } else {
        return NULL;
    }
}

static void GlxMappingReset(void)
{
    int i;

    for (i=0; i<screenInfo.numScreens; i++) {
        GlxScreenPriv *priv = xglvGetScreenPrivate(screenInfo.screens[i]);
        if (priv != NULL) {
            xglvSetScreenPrivate(screenInfo.screens[i], NULL);
            free(priv);
        }
    }
}

static Bool GlxMappingInit(void)
{
    int i;

    for (i=0; i<screenInfo.numScreens; i++) {
        if (GlxGetScreen(screenInfo.screens[i]) == NULL) {
            GlxMappingReset();
            return FALSE;
        }
    }

    idResource = CreateNewResourceType(idResourceDeleteCallback,
                                       "GLXServerIDRes");
    if (idResource == RT_NONE)
    {
        GlxMappingReset();
        return FALSE;
    }
    return TRUE;
}

static GlxClientPriv *xglvGetClientPrivate(ClientPtr pClient)
{
    return dixLookupPrivate(&pClient->devPrivates, &glvXGLVClientPrivKey);
}

static void xglvSetClientPrivate(ClientPtr pClient, void *priv)
{
    dixSetPrivate(&pClient->devPrivates, &glvXGLVClientPrivKey, priv);
}

GlxClientPriv *GlxGetClientData(ClientPtr client)
{
    GlxClientPriv *cl = xglvGetClientPrivate(client);
    if (cl == NULL) {
        cl = calloc(1, sizeof(GlxClientPriv));
        if (cl != NULL) {
            xglvSetClientPrivate(client, cl);
        }
    }
    return cl;
}

void GlxFreeClientData(ClientPtr client)
{
    GlxClientPriv *cl = xglvGetClientPrivate(client);
    if (cl != NULL) {
        xglvSetClientPrivate(client, NULL);
        free(cl->contextTags);
        free(cl);
    }
}

static void GLXClientCallback(CallbackListPtr *list, void *closure, void *data)
{
    NewClientInfoRec *clientinfo = (NewClientInfoRec *) data;
    ClientPtr client = clientinfo->client;

    switch (client->clientState)
    {
        case ClientStateRetained:
        case ClientStateGone:
            GlxFreeClientData(client);
            break;
    }
}

static void GLXReset(ExtensionEntry *extEntry)
{
    // xf86Msg(X_INFO, "GLX: GLXReset\n");

    GlxVendorExtensionReset(extEntry);
    GlxDispatchReset();
    GlxMappingReset();
}

void GlxExtensionInit(void)
{
    ExtensionEntry *extEntry;

    if (xorg_list_is_empty(&GlxVendorList))
        return;

    // Init private keys, per-screen data
    if (!dixRegisterPrivateKey(&glvXGLVScreenPrivKey, PRIVATE_SCREEN, 0))
        return;
    if (!dixRegisterPrivateKey(&glvXGLVClientPrivKey, PRIVATE_CLIENT, 0))
        return;

    if (!GlxMappingInit()) {
        return;
    }

    if (!GlxDispatchInit()) {
        return;
    }

    if (!AddCallback(&ClientStateCallback, GLXClientCallback, NULL)) {
        return;
    }

    extEntry = AddExtension(GLX_EXTENSION_NAME, __GLX_NUMBER_EVENTS,
                            __GLX_NUMBER_ERRORS, GlxDispatchRequest,
                            GlxDispatchRequest, GLXReset, StandardMinorOpcode);
    if (!extEntry) {
        return;
    }

    GlxErrorBase = extEntry->errorBase;
    CallCallbacks(&vndInitCallbackList, extEntry);
}

int ForwardRequest(GlxServerVendor *vendor, ClientPtr client)
{
    return vendor->glxvc.handleRequest(client);
}

Bool GetContextTag(ClientPtr client, GLXContextTag tag, GlxServerVendor **vendor, void **data)
{
    GlxContextTagInfo *tagInfo = GlxLookupContextTag(client, tag);
    GlxServerVendor *foundVendor = NULL;
    void *foundData = NULL;

    if (tagInfo != NULL) {
        foundVendor = tagInfo->vendor;
        foundData = tagInfo->data;
    }
    if (vendor != NULL) {
        *vendor = foundVendor;
    }
    if (data != NULL) {
        *data = foundData;
    }
    return (foundVendor != NULL);
}

GlxServerImports *GlxAllocateServerImports(void)
{
    return calloc(1, sizeof(GlxServerImports));
}

void GlxFreeServerImports(GlxServerImports *imports)
{
    free(imports);
}

_X_EXPORT const GlxServerExports glxServer = {
    GLXSERVER_VENDOR_ABI_MAJOR_VERSION, // majorVersion
    GLXSERVER_VENDOR_ABI_MINOR_VERSION, // minorVersion

    &vndInitCallbackList,

    GlxAllocateServerImports, // allocateServerImports
    GlxFreeServerImports, // freeServerImports

    GlxCreateVendor, // createVendor
    GlxDestroyVendor, // destroyVendor
    GlxSetScreenVendor, // setScreenVendor

    GlxAddXIDMap, // addXIDMap
    GlxGetXIDMap, // getXIDMap
    GlxRemoveXIDMap, // removeXIDMap
    GetContextTag, // getContextTag
    GlxGetVendorForScreen, // getVendorForScreen
    ForwardRequest, // forwardRequest
};

_X_EXPORT const GlxServerExports *glvndGetExports(void)
{
    return &glxServer;
}
