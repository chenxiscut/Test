
/* BIOS include files: */
#include <ti/bios/include/std.h>
#include <tsk.h>

/* Run Time lib include files: */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <VLIB_prototypes.h>
#include <VLIB_testUtils.h>


#include <std.h>
#include <gio.h>
#include <mem.h>
#include <log.h>
#include <string.h>
#include <iom.h>
#include <ti/sdo/pspdrivers/drivers/resizer/psp_resizer.h>
#include <ti/sdo/pspdrivers/pal_os/bios/psputils.h>
#include <ti/sdo/pspdrivers/pal_os/pal_oscache.h>


/* These header names may change */
#include <ti/sdo/pspdrivers/system/dm6437/bios/evmDM6437/video/fvid.h>
#include <ti/sdo/pspdrivers/system/dm6437/bios/evmDM6437/video/fvid_evmDM6437.h>
#include <ti/sdo/pspdrivers/drivers/vport/edc.h>
#include <ti/sdo/pspdrivers/drivers/vport/vport.h>
#include <ti/sdo/pspdrivers/drivers/vport/vportcap.h>
#include <ti/sdo/pspdrivers/drivers/vport/vportdis.h>
#include <ti/sdo/pspdrivers/drivers/vpfe/psp_vpfe.h>
#include <ti/sdo/pspdrivers/drivers/vpbe/psp_vpbe.h>


/* BSL include files */
#include <evmdm6437.h>
#include <evmdm6437_dip.h>

/* Video Params Defaults */
#include <vid_params_default.h>
/*
#include <ti/sdo/pspdrivers/system/dm6437/bios/evmDM6437/video/psp_tvp5146_extVidDecoder.h>
*/

#include "parameters.h"

/* Number of buffers to allocate for video capture + display: */
#define FRAME_BUFF_CNT 6

/* This example supports either PAL or NTSC depending on position of JP1 */
#define STANDARD_PAL  0
#define STANDARD_NTSC 1



/******************************************************************************
  Macros for resize
 ******************************************************************************/
#define HORZ_COEFS -1, 19, 110, 110, 19, -1, 0, 0, 0, 7, 87, 125, 38, -1, 0, 0, 0, 0, 61, 134, 61, 0, 0, 0, 0, -1, 38, 125, 87, 7, 0, 0
#define VERT_COEFS 39, 178, 39, 0, 25, 175, 56, 0, 14, 164, 78, 0, 8, 146, 101, 1, 4, 124, 124, 4, 1, 101, 146, 8, 0, 78, 164, 14, 0, 56, 175, 25
#define gRDRV_reszFilter4TapHighQuality  0, 256, 0, 0, -6, 246, 16, 0, -7, \
    219, 44, 0, -5, 179, 83, -1, -3, 130, 132, -3, -1, 83, 179, -5, 0, \
44, 219, -7, 0, 16, 246, -6
#define gRDRV_reszFilter7TapHighQuality -1, 19, 108, 112, 19, -1, 0, 0, \
    0, 6, 88, 126, 37, -1, 0, 0, 0, 0, 61, 134, 61, 0, 0, 0, 0, -1, 37, \
126, 88, 6, 0, 0

/* debug print macros */
#define RSZ_APP_PRINT          PSP_DEBUG
#define RSZ_APP_PRINT_VAL      PSP_DEBUG1

/* end of macro for resize */

/* Video Encoder parameters for D1, setting depending on PAL vs NTSC: */
static int intraFrameInterval;
static int maxFrameRate;

int inputWidth;
int inputHeight;
/* Encoder Input, Encoder output, and Decoder Output Frame sizes */
static int framesize;
static int frameNum = 0;

/* Video Frame buffers: */
static FVID_Frame *frameBuffTable[FRAME_BUFF_CNT];
static FVID_Frame *frameBuffPtr   = NULL;
static FVID_Frame *OutFrameBuf    = NULL;
static FVID_Frame *erodeFrameBuf  = NULL;
static FVID_Frame *dilateFrameBuf = NULL;

/* Video Driver Handles: */
static FVID_Handle hGioVpfeCcdc = NULL;
static FVID_Handle hGioVpbeVid0 = NULL;
static FVID_Handle hGioVpbeVenc = NULL;


/* Params structure for configuration of parameters for channel 1 */
static PSP_RSZparams params =
{
    DSP_IMG_WIDTH,                        /* Input image width */
    DSP_IMG_HEIGHT,                       /* Input image height */
    DSP_IMG_WIDTH*(2u),                     /* Input image pitch */
    PSP_RSZ_INTYPE_YCBCR422_16BIT,  /* Input image type */
    0,                              /* Vertical starting pixel */
    0,                              /* Horizontal starting pixel */
    0,                              /* Chroma position */
    PSP_RSZ_PIX_FMT_YUYV,           /* Image format */
    IMG_WIDTH,                       /* Intermediate image width */
    IMG_HEIGHT,                      /* Intermediate image height */
    IMG_WIDTH*(2u),                  /* Intermediate image pitch */
    0,                              /* Horizontal starting phase */
    0,                              /* Vertical starting phase */
	HORZ_COEFS,
//    gRDRV_reszFilter4TapHighQuality, /* Horizontal filter coefficients */ 
	VERT_COEFS,
//    gRDRV_reszFilter4TapHighQuality, /* Vertical filter coefficients */
    PSP_RSZ_YENH_DISABLE             /* Luma enhancement options */
};

static char yuv_frame[IMG_WIDTH * IMG_HEIGHT * (2u)];
#pragma DATA_ALIGN(yuv_frame, 128);



Int video_process(Int argc, String argv[])
{
	int result, i;
    int status = 0;
	char *dst_ptr = NULL;
	char *src_ptr = NULL;

    GIO_Attrs gioAttrs = GIO_ATTRS;
    GIO_Handle rszfd;
    int memSegID = 0;

    PSP_RSZresize resize;
	
    /* Set video display/capture driver params to defaults */
    PSP_VPFE_TVP5146_ConfigParams tvp5146Params = {
        TRUE,                               /* enable656Sync */
        PSP_VPFE_TVP5146_FORMAT_COMPOSITE,  /* format        */
        PSP_VPFE_TVP5146_MODE_AUTO          /* mode          */
    };
    //VID_PARAMS_TVP5146_DEFAULT;
    PSP_VPFECcdcConfigParams      vpfeCcdcConfigParams = {
        FVID_VI_BT656_8BIT,                  /* dataFlow     */
        FVID_FRAME_MODE,                    /* ffMode       */
        DSP_IMG_HEIGHT,                                /* height       */
        DSP_IMG_WIDTH,                                /* width        */
        (DSP_IMG_WIDTH *2),                           /* pitch        */
        0,                                  /* horzStartPix */
        0,                                  /* vertStartPix */
        NULL,                               /* appCallback  */
        {
           PSP_VPFE_TVP5146_Open,         /* extVD Fxn    */
           PSP_VPFE_TVP5146_Close,
           PSP_VPFE_TVP5146_Control,
        },
        0                                   /*segId         */
     };

    //VID_PARAMS_CCDC_DEFAULT_D1;
    PSP_VPBEOsdConfigParams vpbeOsdConfigParams = {
        FVID_FRAME_MODE,                    /* ffmode       */
        FVID_BPP_BITS16,                    /* bitsPerPixel */
        FVID_YCbCr422_INTERLEAVED,          /* colorFormat  */
        (DSP_IMG_WIDTH *  (16/8u)),                   /* pitch        */
//        (IMG_WIDTH * (2u)),
        0,                                  /* leftMargin   */
        0,                                  /* topMargin    */
        DSP_IMG_WIDTH,                                /* width        */
        DSP_IMG_HEIGHT,                                /* height       */
//        IMG_WIDTH,
//        IMG_HEIGHT,
        0,                                  /* segId        */
        PSP_VPBE_ZOOM_IDENTITY,             /* hScaling     */
        PSP_VPBE_ZOOM_IDENTITY,             /* vScaling     */
        PSP_VPBE_EXP_IDENTITY,              /* hExpansion   */
        PSP_VPBE_EXP_IDENTITY,              /* vExpansion   */
        NULL                                /* appCallback  */

   };

    //VID_PARAMS_OSD_DEFAULT_D1;
    PSP_VPBEVencConfigParams vpbeVencConfigParams;

    printf("video_encdec started.\n");
  
    inputWidth  = DSP_IMG_WIDTH;
    inputHeight = DSP_IMG_HEIGHT;
    intraFrameInterval = DSP_IMG_FRAME_RATE;
    maxFrameRate = 25000;
    vpbeVencConfigParams.displayStandard = PSP_VPBE_DISPLAY_PAL_INTERLACED_COMPOSITE;

    framesize = (inputWidth * inputHeight * 2 * sizeof(Int8));

//    vpfeCcdcConfigParams.height = vpbeOsdConfigParams.height = inputHeight;
//    vpfeCcdcConfigParams.width = vpbeOsdConfigParams.width = inputWidth;
//    vpfeCcdcConfigParams.pitch = vpbeOsdConfigParams.pitch = inputWidth * 2;

    /* Initialize Video Display Driver:  */

    /* create video input channel */
    if (status == 0) {
       PSP_VPFEChannelParams vpfeChannelParams;
       vpfeChannelParams.id     = PSP_VPFE_CCDC;
       vpfeChannelParams.params = (PSP_VPFECcdcConfigParams*)&vpfeCcdcConfigParams;
       hGioVpfeCcdc = FVID_create("/VPFE0",IOM_INOUT,NULL,&vpfeChannelParams,NULL);
       status = (hGioVpfeCcdc == NULL ? -1 : 0);
    }

    /* create video output channel, plane 0 */
    if (status == 0) {
       PSP_VPBEChannelParams vpbeChannelParams;
       vpbeChannelParams.id     = PSP_VPBE_VIDEO_0;
       vpbeChannelParams.params = (PSP_VPBEOsdConfigParams*)&vpbeOsdConfigParams;
       hGioVpbeVid0 = FVID_create("/VPBE0",IOM_INOUT,NULL,&vpbeChannelParams,NULL);
       status = (hGioVpbeVid0 == NULL ? -1 : 0);
    }

    /* create video output channel, venc */
    if (status == 0) {
       PSP_VPBEChannelParams vpbeChannelParams;
       vpbeChannelParams.id     = PSP_VPBE_VENC;
       vpbeChannelParams.params = (PSP_VPBEVencConfigParams *)&vpbeVencConfigParams;
       hGioVpbeVenc = FVID_create("/VPBE0",IOM_INOUT,NULL,&vpbeChannelParams,NULL);
       status = (hGioVpbeVenc == NULL ? -1 : 0);
    }

    /* configure the TVP5146 video decoder */
    if (status == 0) {
       result = FVID_control(hGioVpfeCcdc, 
            VPFE_ExtVD_BASE+PSP_VPSS_EXT_VIDEO_DECODER_CONFIG, &tvp5146Params);
       status = (result == IOM_COMPLETED ? 0 : -1);
    }

    /* allocate display/capture frame buffers */
    for (i=0; i<FRAME_BUFF_CNT; i++) {
       frameBuffTable[i] = NULL;
    }
    if (status == 0) {
       for (i=0; i<FRAME_BUFF_CNT && status == 0; i++) {
            result = FVID_allocBuffer(hGioVpfeCcdc, &frameBuffTable[i]);
            status = (result == IOM_COMPLETED && frameBuffTable[i] != NULL ? 0 : -1);
       }
    }

    if (status == 0)
     {
            result = FVID_allocBuffer(hGioVpfeCcdc, &OutFrameBuf);
			result = FVID_allocBuffer(hGioVpfeCcdc, &erodeFrameBuf);
			result = FVID_allocBuffer(hGioVpfeCcdc, &dilateFrameBuf);
            status = (result == IOM_COMPLETED && OutFrameBuf != NULL ? 0 : -1);
     }
  
    /* prime up the video capture channel */
    if (status == 0) {
        FVID_queue(hGioVpfeCcdc, frameBuffTable[0]);
        FVID_queue(hGioVpfeCcdc, frameBuffTable[1]);
        FVID_queue(hGioVpfeCcdc, frameBuffTable[2]);
    }

    /* prime up the video display channel */
     if (status == 0) {
        FVID_queue(hGioVpbeVid0, frameBuffTable[3]);
        FVID_queue(hGioVpbeVid0, frameBuffTable[4]);
        FVID_queue(hGioVpbeVid0, frameBuffTable[5]);
    }

    /* grab first buffer from input queue */
    if (status == 0) {
        FVID_dequeue(hGioVpfeCcdc, &frameBuffPtr);
    }


    rszfd = GIO_create("/resizer", IOM_INOUT, NULL, (void *)&memSegID, &gioAttrs);

    if (rszfd == NULL)
    {
        RSZ_APP_PRINT("\nError opening resizer device;");
        status = -1;
    }

    if (status == 0)
    {
        status = GIO_control(rszfd, PSP_RSZ_IOCTL_SET_PARAMS, &params);
        if (status > 0)
            RSZ_APP_PRINT(" IOCTL Failed. \n");
    }

	init_pedestrian_detection();

    while (1)
    {
        /* grab a fresh video input frame */

        FVID_exchange(hGioVpfeCcdc, &frameBuffPtr);

        if (status == 0)
        {
            resize.inBuf = frameBuffPtr->frame.frameBufferPtr;
            resize.outBuf = yuv_frame;
            resize.inBufSize = DSP_IMG_WIDTH * DSP_IMG_HEIGHT * (2u);
            resize.outBufSize = IMG_WIDTH * IMG_HEIGHT * (2u);

            status = GIO_control(rszfd, PSP_RSZ_IOCTL_RESIZE, &resize);
        }

/*		
        if (status == 0)
        {
            // detect pedestrian routine
			dst_ptr = frameBuffPtr->frame.frameBufferPtr;
			src_ptr = yuv_frame;
			for (i = 0; i < IMG_HEIGHT; i++)
			{
				memcpy(dst_ptr, src_ptr, IMG_WIDTH * 2u);
				dst_ptr += (DSP_IMG_WIDTH * 2u);
				src_ptr += (IMG_WIDTH * 2u);
			}
        }
*/		
//
		detect_pedestrian(frameBuffPtr->frame.frameBufferPtr, yuv_frame);
//		detect_pedestrian(frameBuffPtr->frame.frameBufferPtr);
        
        FVID_exchange(hGioVpbeVid0, &frameBuffPtr);
    }

    GIO_delete(rszfd);



    /* Free Video Driver resources:  */
    if (hGioVpfeCcdc)  {
       for (i=0; i<FRAME_BUFF_CNT && status == 0; i++) {
            FVID_freeBuffer(hGioVpfeCcdc, frameBuffTable[i]);
       }
	   FVID_freeBuffer(hGioVpfeCcdc, OutFrameBuf);
	   FVID_freeBuffer(hGioVpfeCcdc, erodeFrameBuf);
	   FVID_freeBuffer(hGioVpfeCcdc, dilateFrameBuf);
    }


    /* Delete Channels */
    if (hGioVpfeCcdc)  {
       FVID_delete(hGioVpfeCcdc);
    }
    if (hGioVpbeVid0)  {
       FVID_delete(hGioVpbeVid0);
    }
    if (hGioVpbeVenc)  {
       FVID_delete(hGioVpbeVenc);
    }

    printf("video_encdec done.\n");
    return (0);
}


