/* This file has to be built with -mcpu=ev56 */
#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include "xf86.h"
#include "compiler.h"

int
xf86ReadMmio8(void *Base, register unsigned long Offset)
{
    mem_barrier();
    return *(volatile CARD8 *) ((unsigned long) Base + (Offset));
}

int
xf86ReadMmio16(void *Base, register unsigned long Offset)
{
    mem_barrier();
    return *(volatile CARD16 *) ((unsigned long) Base + (Offset));
}

int
xf86ReadMmio32(void *Base, register unsigned long Offset)
{
    mem_barrier();
    return *(volatile CARD32 *) ((unsigned long) Base + (Offset));
}

void
xf86WriteMmio8(int Value, void *Base, register unsigned long Offset)
{
    write_mem_barrier();
    *(volatile CARD8 *) ((unsigned long) Base + (Offset)) = Value;
}

void
xf86WriteMmio16(int Value, void *Base, register unsigned long Offset)
{
    write_mem_barrier();
    *(volatile CARD16 *) ((unsigned long) Base + (Offset)) = Value;
}

void
xf86WriteMmio32(int Value, void *Base, register unsigned long Offset)
{
    write_mem_barrier();
    *(volatile CARD32 *) ((unsigned long) Base + (Offset)) = Value;
}
