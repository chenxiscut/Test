/*
 * =====================================================================
 *
 *       Filename:  parameters.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/18/2012 11:42:29 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  dengos (), dengos.w@gmail.com
 *        Company:  scut
 *
 * =====================================================================
 */


#ifndef  parameters_INC
#define  parameters_INC

#include "fastrts_i.h"

#ifndef NULL
#define 	NULL 			0
#endif

#define DSP_IMG_WIDTH  720
#define DSP_IMG_HEIGHT 576
#define DSP_IMG_FRAME_RATE 25

#define     IMG_WIDTH       352
#define     IMG_HEIGHT      288
// 32 for 32-bit int
#define     BIN_IMG_WIDTH   IMG_WIDTH / 32


/*----------------------------------------------------------------------
 *  parameters for hog
 *--------------------------------------------------------------------*/
#define NEAR        1
#define MID         2
#define FAR         3

#define BLOCK_WIDTH_CELL    2
#define BLOCK_HEIGHT_CELL   2

// the width of detect window
#define DW_WIDTH    32
// the height of detect window
#define DW_HEIGHT   80 

//---------   near
#define DW_NWIDTH       32
#define DW_NHEIGHT      80
#define CELL_NWIDTH     8
#define CELL_NHEIGHT    8
#define FEATURE_NSIZE   ((DW_NWIDTH/CELL_NWIDTH-1)*(DW_NHEIGHT/CELL_NHEIGHT-1)\
        *BLOCK_HEIGHT_CELL*BLOCK_WIDTH_CELL*NUM_BINS)
//---------   middle
#define DW_MWIDTH       24
#define DW_MHEIGHT      64
#define CELL_MWIDTH     6
#define CELL_MHEIGHT    8
#define FEATURE_MSIZE   ((DW_MWIDTH/CELL_MWIDTH-1)*(DW_MHEIGHT/CELL_MHEIGHT-1)\
        *BLOCK_HEIGHT_CELL*BLOCK_WIDTH_CELL*NUM_BINS)

//---------   far
#define DW_FWIDTH       12
#define DW_FHEIGHT      32
#define CELL_FWIDTH     3
#define CELL_FHEIGHT    4
#define FEATURE_FSIZE   ((DW_FWIDTH/CELL_FWIDTH-1)*(DW_FHEIGHT/CELL_FHEIGHT-1)\
        *BLOCK_HEIGHT_CELL*BLOCK_WIDTH_CELL*NUM_BINS)


#define FEATURE_SIZE    FEATURE_NSIZE


// 8 * 2
#define BLOCK_WIDTH     16 
// 8 * 2
#define BLOCK_HEIGHT    16  
#define CELL_WIDTH      8
#define CELL_HEIGHT     8
#define NUM_BINS        9



#define NUM_BLOCKS ((DW_WIDTH/CELL_WIDTH - 1) * (DW_HEIGHT/CELL_HEIGHT - 1))
#define NUM_CELLS  ((DW_WIDTH/CELL_WIDTH) * (DW_HEIGHT/CELL_HEIGHT)) 
#define NUM_DW_PIXELS (DW_WIDTH * DW_HEIGHT) 
#define NUM_CELLS_PER_BLOCK  ((BLOCK_WIDTH/CELL_WIDTH) \
        * (BLOCK_HEIGHT/CELL_HEIGHT))
#define NUM_CELLS_PER_LINE DW_WIDTH/CELL_WIDTH 
#define NUM_BLOCKS_PER_LINE (NUM_CELLS_PER_LINE - 1)

#define SIZE_CELLS_HOG 648
#define SIZE_BLOCKS_HOG 1980
#define NUM_FEATURES FEATURE_SIZE
#define ROI_FEATURES_SIZE 4


/*----------------------------------------------------------------------
 *  end of hog
 *--------------------------------------------------------------------*/


#define HFACTOR 2.0455
#define VFACTOR 2.0

/*----------------------------------------------------------------------
 *  parameters for segmentation
 *--------------------------------------------------------------------*/

#define SEG_THRESHOLD       20
#define SEG_LAMBDA           1.1F
#define SEG_STRIP_WIDTH     8

// parameters for gp_dt_segment
#define SEG_GPDT_OMIGA           12
#define SEG_GPDT_WEIGHT          1.06F
#define SEG_GPDT_GRAY_H          220
#define SEG_GPDT_ALPHA           1
#define SEG_GPDT_BETA            4
#define SEG_GPDT_GAMMA           5
#define SEG_GPDT_NORMALIZER      0.04 // 1.0 / (2 * SEG_SDT_OMIGA + 1.0)


// parameters for simple_dt_segment
#define SEG_SDT_OMIGA           12
#define SEG_SDT_WEIGHT          1.06
#define SEG_SDT_GRAH_H          220
#define SEG_SDT_ALPHA           1
#define SEG_SDT_BETA            4
#define SEG_SDT_GAMMA 			5
#define SEG_SDT_NORMALIZER      0.04 // 1.0 / (2 * SEG_SDT_OMIGA + 1.0)
#define SEG_SDT_OVERHEAD        18   // SEG_SDT_OMIGA * 1.5

/*----------------------------------------------------------------------
 *  end of segmentation
 *--------------------------------------------------------------------*/
 
 
#define GREEN (0x952B9515)
#define RED (0x4CFF4C55)
#define BLUE (0x1C191C6E)

#endif   /* ----- #ifndef parameters_INC  ----- */

