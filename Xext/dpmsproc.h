/* Prototypes for functions that the DDX must provide */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#ifndef _DPMSPROC_H_
#define _DPMSPROC_H_

#include "dixstruct.h"

int XORG_EXPORT DPMSSet(ClientPtr client, int level);
Bool XORG_EXPORT DPMSSupported(void);

#endif
