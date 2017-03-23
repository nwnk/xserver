
#ifndef _XF86RAMDAC_H
#define _XF86RAMDAC_H 1

#include "colormapst.h"
#include "xf86Cursor.h"

/* Define unique vendor codes for RAMDAC's */
#define VENDOR_IBM	0x0000
#define VENDOR_BT	0x0001
#define VENDOR_TI	0x0002

typedef struct _RamDacRegRec {
/* This is probably the nastiest assumption, we allocate 1024 slots for
 * ramdac registers, should be enough. I've checked IBM and TVP series
 * and they seem o.k
 * Then we allocate 768 entries for the DAC too. IBM640 needs 1024 -FIXME
 */
    unsigned short DacRegs[0x400];      /* register set */
    unsigned char DAC[0x300];   /* colour map */
    Bool Overlay;
} RamDacRegRec, *RamDacRegRecPtr;

typedef struct _RamDacHWRegRec {
    RamDacRegRec SavedReg;
    RamDacRegRec ModeReg;
} RamDacHWRec, *RamDacHWRecPtr;

typedef struct _RamDacRec {
    CARD32 RamDacType;

    void (*LoadPalette) (ScrnInfoPtr pScrn,
                         int numColors,
                         int *indices, LOCO * colors, VisualPtr pVisual);

    unsigned char (*ReadDAC) (ScrnInfoPtr pScrn, CARD32);

    void (*WriteDAC) (ScrnInfoPtr pScrn, CARD32, unsigned char, unsigned char);

    void (*WriteAddress) (ScrnInfoPtr pScrn, CARD32);

    void (*WriteData) (ScrnInfoPtr pScrn, unsigned char);

    void (*ReadAddress) (ScrnInfoPtr pScrn, CARD32);

    unsigned char (*ReadData) (ScrnInfoPtr pScrn);
} RamDacRec, *RamDacRecPtr;

typedef struct _RamDacHelperRec {
    CARD32 RamDacType;

    void (*Restore) (ScrnInfoPtr pScrn,
                     RamDacRecPtr ramdacPtr, RamDacRegRecPtr ramdacReg);

    void (*Save) (ScrnInfoPtr pScrn,
                  RamDacRecPtr ramdacPtr, RamDacRegRecPtr ramdacReg);

    void (*SetBpp) (ScrnInfoPtr pScrn, RamDacRegRecPtr ramdacReg);

    void (*HWCursorInit) (xf86CursorInfoPtr infoPtr);
} RamDacHelperRec, *RamDacHelperRecPtr;

#define RAMDACHWPTR(p) ((RamDacHWRecPtr)((p)->privates[RamDacGetHWIndex()].ptr))

typedef struct _RamdacScreenRec {
    RamDacRecPtr RamDacRec;
} RamDacScreenRec, *RamDacScreenRecPtr;

#define RAMDACSCRPTR(p) ((RamDacScreenRecPtr)((p)->privates[RamDacGetScreenIndex()].ptr))->RamDacRec

extern XORG_EXPORT int RamDacHWPrivateIndex;
extern XORG_EXPORT int RamDacScreenPrivateIndex;

typedef struct {
    int token;
} RamDacSupportedInfoRec, *RamDacSupportedInfoRecPtr;

extern XORG_EXPORT RamDacRecPtr RamDacCreateInfoRec(void);
extern XORG_EXPORT RamDacHelperRecPtr RamDacHelperCreateInfoRec(void);
extern XORG_EXPORT void RamDacDestroyInfoRec(RamDacRecPtr RamDacRec);
extern XORG_EXPORT void RamDacHelperDestroyInfoRec(RamDacHelperRecPtr RamDacRec);
extern XORG_EXPORT Bool RamDacInit(ScrnInfoPtr pScrn, RamDacRecPtr RamDacRec);
extern XORG_EXPORT Bool RamDacHandleColormaps(ScreenPtr pScreen, int maxColors,
                                            int sigRGBbits, unsigned int flags);
extern XORG_EXPORT void RamDacFreeRec(ScrnInfoPtr pScrn);
extern XORG_EXPORT int RamDacGetHWIndex(void);

#endif                          /* _XF86RAMDAC_H */
