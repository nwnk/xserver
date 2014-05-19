/* xf86DDC.c
 *
 * Copyright 1998,1999 by Egbert Eich <Egbert.Eich@Physik.TU-Darmstadt.DE>
 */

/*
 * A note on terminology.  DDC1 is the original dumb serial protocol, and
 * can only do up to 128 bytes of EDID.  DDC2 is I2C-encapsulated and
 * introduces extension blocks.  EDID is the old display identification
 * block, DisplayID is the new one.
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include "misc.h"
#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86DDC.h"
#include <string.h>

#define RETRIES 4

typedef enum {
    DDCOPT_NODDC1,
    DDCOPT_NODDC2,
    DDCOPT_NODDC
} DDCOpts;

static const OptionInfoRec DDCOptions[] = {
    {DDCOPT_NODDC1, "NoDDC1", OPTV_BOOLEAN, {0}, FALSE},
    {DDCOPT_NODDC2, "NoDDC2", OPTV_BOOLEAN, {0}, FALSE},
    {DDCOPT_NODDC, "NoDDC", OPTV_BOOLEAN, {0}, FALSE},
    {-1, NULL, OPTV_NONE, {0}, FALSE},
};

static int
DDC_checksum(const unsigned char *block, int len)
{
    int i, result = 0;
    int not_null = 0;

    for (i = 0; i < len; i++) {
        not_null |= block[i];
        result += block[i];
    }

#ifdef DEBUG
    if (result & 0xFF)
        ErrorF("DDC checksum not correct\n");
    if (!not_null)
        ErrorF("DDC read all Null\n");
#endif

    /* catch the trivial case where all bytes are 0 */
    if (!not_null)
        return 1;

    return result & 0xFF;
}

/* DDC2 */

static I2CDevPtr
DDC2MakeDevice(I2CBusPtr pBus, int address, const char *name)
{
    I2CDevPtr dev = NULL;

    if (!(dev = xf86I2CFindDev(pBus, address))) {
        dev = xf86CreateI2CDevRec();
        dev->DevName = name;
        dev->SlaveAddr = address;
        dev->ByteTimeout = 2200;        /* VESA DDC spec 3 p. 43 (+10 %) */
        dev->StartTimeout = 550;
        dev->BitTimeout = 40;
        dev->AcknTimeout = 40;

        dev->pI2CBus = pBus;
        if (!xf86I2CDevInit(dev)) {
            xf86DrvMsg(pBus->scrnIndex, X_PROBED, "No DDC2 device\n");
            return NULL;
        }
    }

    return dev;
}

static I2CDevPtr
DDC2Init(I2CBusPtr pBus)
{
    I2CDevPtr dev = NULL;

    /*
     * Slow down the bus so that older monitors don't
     * miss things.
     */
    pBus->RiseFallTime = 20;

    dev = DDC2MakeDevice(pBus, 0x00A0, "ddc2");
    if (xf86I2CProbeAddress(pBus, 0x0060))
        DDC2MakeDevice(pBus, 0x0060, "E-EDID segment register");

    return dev;
}

/* Mmmm, smell the hacks */
static void
EEDIDStop(I2CDevPtr d)
{
}

/* block is the EDID block number.  a segment is two blocks. */
static Bool
DDC2Read(I2CDevPtr dev, int block, unsigned char *R_Buffer)
{
    unsigned char W_Buffer[1];
    int i, segment;
    I2CDevPtr seg;
    void (*stop) (I2CDevPtr);

    for (i = 0; i < RETRIES; i++) {
        /* Stop bits reset the segment pointer to 0, so be careful here. */
        segment = block >> 1;
        if (segment) {
            Bool b;

            if (!(seg = xf86I2CFindDev(dev->pI2CBus, 0x0060)))
                return FALSE;

            W_Buffer[0] = segment;

            stop = dev->pI2CBus->I2CStop;
            dev->pI2CBus->I2CStop = EEDIDStop;

            b = xf86I2CWriteRead(seg, W_Buffer, 1, NULL, 0);

            dev->pI2CBus->I2CStop = stop;
            if (!b) {
                dev->pI2CBus->I2CStop(dev);
                continue;
            }
        }

        W_Buffer[0] = (block & 0x01) * EDID1_LEN;

        if (xf86I2CWriteRead(dev, W_Buffer, 1, R_Buffer, EDID1_LEN)) {
            if (!DDC_checksum(R_Buffer, EDID1_LEN))
                return TRUE;
        }
    }

    return FALSE;
}

/**
 * Attempts to probe the monitor for EDID information, if NoDDC and NoDDC2 are
 * unset.  EDID information blocks are interpreted and the results returned in
 * an xf86MonPtr.  Unlike xf86DoEDID_DDC[12](), this function will return
 * the complete EDID data, including all extension blocks, if the 'complete'
 * parameter is TRUE;
 *
 * This function does not affect the list of modes used by drivers -- it is up
 * to the driver to decide policy on what to do with EDID information.
 *
 * @return pointer to a new xf86MonPtr containing the EDID information.
 * @return NULL if no monitor attached or failure to interpret the EDID.
 */
xf86MonPtr
xf86DoEEDID(ScrnInfoPtr pScrn, I2CBusPtr pBus, Bool complete)
{
    unsigned char *EDID_block = NULL;
    xf86MonPtr tmp = NULL;
    I2CDevPtr dev = NULL;

    /* Default DDC and DDC2 to enabled. */
    Bool noddc = FALSE, noddc2 = FALSE;
    OptionInfoPtr options;

    options = malloc(sizeof(DDCOptions));
    if (!options)
        return NULL;
    memcpy(options, DDCOptions, sizeof(DDCOptions));
    xf86ProcessOptions(pScrn->scrnIndex, pScrn->options, options);

    xf86GetOptValBool(options, DDCOPT_NODDC, &noddc);
    xf86GetOptValBool(options, DDCOPT_NODDC2, &noddc2);
    free(options);

    if (noddc || noddc2)
        return NULL;

    if (!(dev = DDC2Init(pBus)))
        return NULL;

    EDID_block = calloc(1, EDID1_LEN);
    if (!EDID_block)
        return NULL;

    if (DDC2Read(dev, 0, EDID_block)) {
        int i, n = EDID_block[0x7e];

        if (complete && n) {
            EDID_block = reallocarray(EDID_block, 1 + n, EDID1_LEN);

            for (i = 0; i < n; i++)
                DDC2Read(dev, i + 1, EDID_block + (EDID1_LEN * (1 + i)));
        }

        tmp = xf86InterpretEEDID(pScrn->scrnIndex, EDID_block);
    }

    if (tmp && complete)
        tmp->flags |= MONITOR_EDID_COMPLETE_RAWDATA;

    return tmp;
}

/**
 * Attempts to probe the monitor for EDID information, if NoDDC and NoDDC2 are
 * unset.  EDID information blocks are interpreted and the results returned in
 * an xf86MonPtr.
 *
 * This function does not affect the list of modes used by drivers -- it is up
 * to the driver to decide policy on what to do with EDID information.
 *
 * @return pointer to a new xf86MonPtr containing the EDID information.
 * @return NULL if no monitor attached or failure to interpret the EDID.
 */
xf86MonPtr
xf86DoEDID_DDC2(ScrnInfoPtr pScrn, I2CBusPtr pBus)
{
    return xf86DoEEDID(pScrn, pBus, FALSE);
}
