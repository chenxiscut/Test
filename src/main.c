/*
 *  Copyright 2008 by Texas Instruments Incorporated.
 *  All rights reserved. Property of Texas Instruments Incorporated.
 *  Restricted rights to use, duplicate or disclose this code are
 *  granted through contract.
 *
 *  @(#) dvsdk_1_11_00_00 1.11.00.00 061808 (biosdvsdk-c00x)
 */
/*
 * ======== main.c ========
 *
 */

/* Type systems: */
//#include <xdc/std.h>
#include <ti/bios/include/std.h>

/* BIOS include files: */
#include <tsk.h>
#include <sys.h>
#include <trc.h>

/* Codec Engine: */
//#include <ti/sdo/ce/CERuntime.h>

/* PSP include files: */
#include <ti/sdo/pspdrivers/soc/dm6437/dsp/soc.h>
#include <ti/sdo/pspdrivers/soc/dm6437/dsp/cslr_sys.h>

/* BSL include files */
#include <evmdm6437.h>
#include <evmdm6437_dip.h>

#include <time.h>
clock_t start,end;

/* Extra qdma setup function */
//#include <qdma_setup.h>

//#include "setupdsp.h"

extern Int video_process(Int argc, String argv[]);//³ÌÐò¿ªÊ¼Èë¿Ú

static String taskName = "video_encdec";

static CSL_SysRegsOvly sysModuleRegs = (CSL_SysRegsOvly )CSL_SYS_0_REGS;

/*
 * ======== main =========================
 */
Int main(Int argc, String argv[])
{
    TSK_Attrs attrs = TSK_ATTRS;
    attrs.stacksize = 32 * 1024;   // H.264 Dec needs 12K.
    attrs.name = taskName;

    /* Initialize BSL library to read jumper switches: */
    EVMDM6437_DIP_init();

    /* VPSS PinMuxing */
    sysModuleRegs -> PINMUX0    &= (0x005482A3u);
    sysModuleRegs -> PINMUX0    |= (0x005482A3u);
    /* PCIEN    =   0: PINMUX1 - Bit 0 */
    sysModuleRegs -> PINMUX1 &= (0xFFFFFFFEu);
    sysModuleRegs -> VPSS_CLKCTL = (0x18u);

    /* Initialize Codec Engine: */
//    CERuntime_init();

    /* Configure QMDA priority of codecs to be lower than VPBE */
 //   qdma_setup();
       
    if (TSK_create((Fxn)video_process, &attrs, argc, argv) == NULL) {
        SYS_abort("main: failed to create video_encdec thread.");
    }

    return (0);
}


