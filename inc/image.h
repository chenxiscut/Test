/**
 * =====================================================================================
 *
 *          @file  image.h
 *         @brief  head file
 *
 *        @author  dengos (W), dengos.w@gmail.com, 
 *       @version  1.0
 *          @date  09/02/2012 03:50:44 PM
 *
 * =====================================================================================
 */



#ifndef  image_INC
#define  image_INC
#include <VLIB_prototypes.h>
#include <VLIB_testUtils.h>


int
imresize ( const unsigned char *src, int swidth, int sheight, 
         unsigned char *dst, int dwidth, int dheight);

int
extract_blob ( 
        const unsigned char *image,
        int width,
        int xmin , int ymin,
        int xmax , int ymax,
        unsigned char *blob_image);

int
draw_hline(char *yuv_frame,
	int x, int y, int len, int color);		
		
		
int 
draw_vline(char *yuv_frame,
	int x, int y, int len, int color);
		
int 
draw_rectangle(char *yuv_frame, 
	int xmin, int ymin, 
	int xmax, int ymax, int color);

int
draw_zoom_rectangle (char *yuv_frame, VLIB_CC *blob, 
        float hfactor, float vfactor, int color);

#endif   /* ----- #ifndef image_INC  ----- */


