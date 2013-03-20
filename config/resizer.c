/*******************************************************************************
**+--------------------------------------------------------------------------+**
**|                            ****                                          |**
**|                            ****                                          |**
**|                            ******o***                                    |**
**|                      ********_///_****                                   |**
**|                      ***** /_//_/ ****                                   |**
**|                       ** ** (__/ ****                                    |**
**|                           *********                                      |**
**|                            ****                                          |**
**|                            ***                                           |**
**|                                                                          |**
**|         Copyright (c) 2006-2010 Texas Instruments Incorporated           |**
**|                        ALL RIGHTS RESERVED                               |**
**|                                                                          |**
**| Permission is hereby granted to licensees of Texas Instruments           |**
**| Incorporated (TI) products to use this computer program for the sole     |**
**| purpose of implementing a licensee product based on TI products.         |**
**| No other rights to reproduce, use, or disseminate this computer          |**
**| program, whether in part or in whole, are granted.                       |**
**|                                                                          |**
**| TI makes no representation or warranties with respect to the             |**
**| performance of this computer program, and specifically disclaims         |**
**| any responsibility for any damages, special or consequential,            |**
**| connected with the use of this program.                                  |**
**|                                                                          |**
**+--------------------------------------------------------------------------+**
*******************************************************************************/

/**
 *  \file   psp_bios_resz_downscale_st_example.c
 *
 *  \brief  resizer application
 *
 *  This file contains the function which is used for demonstrating the upscale
 * from( 640 480) to (320, 240)
 *  serivces of the rsz device driver.
 *
 *  (C) Copyright 2010, Texas Instruments, Inc
 *
 *  \author     EI3
 *
 *  \version    1.0     Created
 */

/******************************************************************************
  Header File Inclusion
 ******************************************************************************/
#include <std.h>
#include <tsk.h>
#include <gio.h>
#include <mem.h>
#include <log.h>
#include <string.h>
#include <iom.h>
#include <ti/sdo/pspdrivers/drivers/resizer/psp_resizer.h>
#include <ti/sdo/pspdrivers/pal_os/bios/psputils.h>
#include <ti/sdo/pspdrivers/pal_os/pal_oscache.h>

/******************************************************************************
  Macros
 ******************************************************************************/
#define INHEIGHT    (480u)
#define INWIDTH     (640u)
#define OUTHEIGHT   (240u)
#define OUTWIDTH    (320u)
#define gRDRV_reszFilter4TapHighQuality  0, 256, 0, 0, -6, 246, 16, 0, -7, \
    219, 44, 0, -5, 179, 83, -1, -3, 130, 132, -3, -1, 83, 179, -5, 0, \
44, 219, -7, 0, 16, 246, -6
#define gRDRV_reszFilter7TapHighQuality -1, 19, 108, 112, 19, -1, 0, 0, \
    0, 6, 88, 126, 37, -1, 0, 0, 0, 0, 61, 134, 61, 0, 0, 0, 0, -1, 37, \
126, 88, 6, 0, 0

/* debug print macros */
#define RSZ_APP_PRINT          PSP_DEBUG
#define RSZ_APP_PRINT_VAL      PSP_DEBUG1


/******************************************************************************
  Globals
 ******************************************************************************/
Char *inBufRSZ=0;
Char *outBufRSZ=0;
Char ycharoutput[OUTHEIGHT *OUTWIDTH];
Char cbcharoutput[OUTHEIGHT *(OUTWIDTH/(2u))];
Char crcharoutput[OUTHEIGHT *(OUTWIDTH/(2u))];


/* make Y buffer 128 bytes aligned */
#pragma DATA_ALIGN(ycharoutput, 128);
/* make Y buffer 128 bytes aligned */
#pragma DATA_ALIGN(cbcharoutput, 128);
/* make Y buffer 128 bytes aligned */
#pragma DATA_ALIGN(crcharoutput, 128);

/* Params structure for configuration of parameters for channel 1 */
static PSP_RSZparams params =
{
    INWIDTH,                        /* Input image width */
    INHEIGHT,                       /* Input image height */
    INWIDTH*(2u),                     /* Input image pitch */
    PSP_RSZ_INTYPE_YCBCR422_16BIT,  /* Input image type */
    0,                              /* Vertical starting pixel */
    0,                              /* Horizontal starting pixel */
    0,                              /* Chroma position */
    PSP_RSZ_PIX_FMT_YUYV,           /* Image format */
    OUTWIDTH,                       /* Intermediate image width */
    OUTHEIGHT,                      /* Intermediate image height */
    OUTWIDTH*(2u),                  /* Intermediate image pitch */
    0,                              /* Horizontal starting phase */
    0,                              /* Vertical starting phase */
    gRDRV_reszFilter4TapHighQuality, /* Horizontal filter coefficients */
    gRDRV_reszFilter4TapHighQuality, /* Vertical filter coefficients */
    PSP_RSZ_YENH_DISABLE             /* Luma enhancement options */
};
/******************************************************************************
  Function definition
 ******************************************************************************/
/* Function to take the input image */
static void TakeRszInputImage();
/* Function to take the output image */
static void TakeRszOutputImage();

/******************************************************************************
  Resizer Task Function
 ******************************************************************************/
/**
 * \brief   RSZTskFun
 *          Resizer task function
 *
 */
Int RSZTskFun()
{

    GIO_Attrs gioAttrs = GIO_ATTRS;
    GIO_Handle rszfd;
    Int statusOP = 0;
    Int memSegId = 0;

    PSP_RSZresize resize;



    if ((NULL == inBufRSZ) && (NULL == outBufRSZ))
    {
        /* Memory allocation for  output buffers */

        outBufRSZ =(Char *)MEM_alloc(0, OUTHEIGHT*OUTWIDTH*(2u), (128u));
        if (outBufRSZ == NULL)
        {
            statusOP = -1;
        }

    }
    if (-1 != statusOP)
    {
        /* Creating the logical channel */
        rszfd = GIO_create("/resizer", IOM_INOUT, NULL,
            (void *)&memSegId, &gioAttrs);

        if (NULL == rszfd)
        {
            RSZ_APP_PRINT("\nError opening resizer device;");
            statusOP = -1;
        }

        /* setting the parameters */
        statusOP = GIO_control(rszfd, PSP_RSZ_IOCTL_SET_PARAMS, &params);
        if (0 > statusOP)
        {
            RSZ_APP_PRINT(" IOCTL failed \n");
        }
        /* Taking the input image */
        TakeRszInputImage();

        resize.inBuf = inBufRSZ;
        resize.outBuf = outBufRSZ;
        resize.inBufSize =INHEIGHT*INWIDTH*(2u);
        resize.outBufSize =OUTHEIGHT*OUTWIDTH*(2u);

        /* resizing */
        statusOP = GIO_control(rszfd, PSP_RSZ_IOCTL_RESIZE, &resize);
        if (0 > statusOP)
        {
            RSZ_APP_PRINT(" IOCTL failed \n");

        }
        /* Taking the output image */
        TakeRszOutputImage();

        /* Closing logical channel */
       GIO_delete(rszfd);


        MEM_free(0, outBufRSZ, OUTHEIGHT*OUTWIDTH*(2u));


    }
    if (0 == statusOP)
    {
        RSZ_APP_PRINT(" \n Test succesful \n\n");
    }
    else
    {
        RSZ_APP_PRINT("\n Test failed \n");
    }
    return statusOP;
}
/**
 * \brief   TakeRszInputImage
 *          This function takes the input image from file
 *
 */
static void TakeRszInputImage()
{

//    inBufRSZ = (char *)array;

}
/**
 * \brief   TakeRszOutputImage
 *          This function splits output image to see in CCS image viewer
 *
 */
static void TakeRszOutputImage()
{
    /* Image for viewing in CCS image viewer*/

    Int i;


    PAL_osCacheInvalidate((PAL_osCacheMemAddrSpace)NULL, (Uint32)outBufRSZ,
        OUTHEIGHT*OUTWIDTH*(2u));

    /* taking the output image */
    for (i=0;i<(OUTHEIGHT*(OUTWIDTH/(2u)));i++)
    {

        crcharoutput[i] = *outBufRSZ;
        outBufRSZ++;
        ycharoutput[(i*2)] = *outBufRSZ;
        outBufRSZ++;
        cbcharoutput[i] = *outBufRSZ;
        outBufRSZ++;
        ycharoutput[(i*2)+1] = *outBufRSZ;
        outBufRSZ++;


    }

    PAL_osCacheFlushAndInvalidate((PAL_osCacheMemAddrSpace)NULL,
        (Uint32)ycharoutput, OUTHEIGHT*OUTWIDTH);
    PAL_osCacheFlushAndInvalidate((PAL_osCacheMemAddrSpace)NULL,
        (Uint32)cbcharoutput, (OUTHEIGHT *(OUTWIDTH/(2u))));
    PAL_osCacheFlushAndInvalidate((PAL_osCacheMemAddrSpace)NULL,
        (Uint32)crcharoutput, (OUTHEIGHT *(OUTWIDTH/(2u))));

}

