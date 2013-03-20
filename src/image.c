/**
 * =====================================================================================
 *
 *          @file  image.c
 *         @brief  Image imresize.
 *
 *        @author  dengos (W), dengos.w@gmail.com, 
 *       @version  1.0
 *          @date  09/02/2012 02:16:53 PM
 *
 * =====================================================================================
 */

#include <VLIB_prototypes.h>
#include <VLIB_testUtils.h>


#include "parameters.h"
//#include "fastrts_i.h"


static int   x_idxs[DW_WIDTH];
static int   y_idxs[DW_HEIGHT];
static float x_diffs[DW_WIDTH];
static float x_rdiffs[DW_WIDTH];
static float y_diffs[DW_HEIGHT];
static float y_rdiffs[DW_HEIGHT];

/**
 * @brief Since we go through the same x, y, and x_diff, y_diff over and over
 * again, when we try to imresize a given image into a fixed one. We can always
 * re-compute all the x, y, x_diff, y_diff before, and then simply use it.
 *
 * @param swidth The width of src.
 * @param sheight The height of src.
 * @param dwidth The width of dst.
 * @param dheight The height of dst.
 *
 * @return 0 if OK.
 */
static int
init ( int swidth, int sheight, int dwidth, int dheight )
{
    int index;
    float x_ratio, y_ratio, accumulate;

    x_ratio = divsp_i(intsp_i(swidth - 1), intsp_i(dwidth));
    y_ratio = divsp_i(intsp_i(sheight - 1), intsp_i(dheight));

    /* we know: x_idxs[0] = 0, x_diffs[0] = 0.0 */
    x_idxs[0]   = 0;
    x_diffs[0]  = 0.0F;
    x_rdiffs[0] = 1.0F;
    for (index  = 1; index < dwidth; ++index)
    {
        accumulate      = mpysp_i(x_ratio, intsp_i(index));
        x_idxs[index]   = spint_i(accumulate);
        x_diffs[index]  = subsp_i(accumulate, intsp_i(x_idxs[index]));
        x_rdiffs[index] = subsp_i(1.0F, x_diffs[index]);
    }

    y_idxs[0]   = 0;
    y_diffs[0]  = 0.0F;
    y_rdiffs[0] = 1.0F;
    for (index  = 1; index < dheight; ++index)
    {
        accumulate      = mpysp_i(y_ratio, intsp_i(index));
        y_idxs[index]   = spint_i(accumulate);
        y_diffs[index]  = subsp_i(accumulate, intsp_i(y_idxs[index]));
        y_rdiffs[index] = subsp_i(1.0F, y_diffs[index]);
    }

    return 0;
}		/* -----  end of function init  ----- */


/**
 * @brief A image imresize function using bilinear algorithm.
 * IMPORTANT: 
 * (1) The function using here only deal with gray scale image.
 * (2) The dst width shouldn't be bigger than DW_WIDTH; and
 * the dst height shouldn't be bigger than DW_HEIGHT.
 *
 * @param src A source image.
 * @param swidth The width of src.
 * @param sheight The height of src.
 * @param dst A image after imresize.
 * @param dwidth The width of dst.
 * @param dheight The height of dst.
 *
 * @return 0 if OK.
 */
int
imresize ( const unsigned char *src, int swidth, int sheight, 
         unsigned char *dst, int dwidth, int dheight)
{
//	static int has_call_init = 0;
    int dst_x, dst_y;
    int index_base, index;
    int A, B, C, D;
	float value;

	init(swidth, sheight, dwidth, dheight);

    for (dst_y = 0; dst_y < dheight; ++dst_y)
    {
        index_base = y_idxs[dst_y] * swidth;
        for (dst_x = 0; dst_x < dwidth; ++dst_x) 
        {
            index = index_base + x_idxs[dst_x];
            A = src[index];
            B = src[index + 1];
            C = src[index + swidth];
            D = src[index + swidth + 1];

            value = mpysp_i(intsp_i(A), mpysp_i(x_rdiffs[dst_x], y_rdiffs[dst_y]));
            value = addsp_i(value,
                    mpysp_i(intsp_i(B), mpysp_i(x_diffs[dst_x], y_rdiffs[dst_y])));
            value = addsp_i(value,
                    mpysp_i(intsp_i(C), mpysp_i(x_rdiffs[dst_x], y_diffs[dst_y])));
            value = addsp_i(value,
                    mpysp_i(intsp_i(D), mpysp_i(x_diffs[dst_x], y_diffs[dst_y])));
            
            *dst++ = spint_i(value) & 0xFF;
        }
    }

    return 0;
}		/* -----  end of function imresize  ----- */



/**
 * @brief Extract a blob image from a given image. Usually, the
 * given image is bigger then blob image. The image is a gray
 * scale image.
 *
 * @param image A big image[const input].
 * @param width The width of big image.
 * @param xmin The top left x position of given blob.
 * @param ymin The top left y position of given blob.
 * @param xmax The bottom right x position of given blob.
 * @param ymax The bottom right y position of given blob.
 * @param blob_image A blob image[output]. The blob image buffer
 * should be at least (xmax-xmin+1) * (ymax-ymin+1) size.
 *
 * @return 
 */
int
extract_blob ( 
        const unsigned char *image,
        int width,
        int xmin , int ymin,
        int xmax , int ymax,
        unsigned char *blob_image)
{
    int x, y;

    for (y = ymin; y < ymax-ymin+1; y++) 
    {
        for (x = xmin; x < xmax-xmin+1; x++) 
        {
            *blob_image++ = image[x];
        }
        image += width;
    }
    return 0;
}		/* -----  end of function extract_blob  ----- */



int
draw_hline(char *yuv_frame,
	int x, int y, int len, int color)
{
	int idx = 0;
	unsigned int *ptr = (unsigned int *)yuv_frame + DSP_IMG_WIDTH / 2 * y;
	idx = sizeof(unsigned int);
	for (idx = x / 2; idx < x/2 + len/2; idx++)
	{
		ptr[idx] = color;
	}
	return 0;
}

int 
draw_vline(char *yuv_frame,
	int x, int y, int len, int color)
{
	int idx = 0;
	unsigned int *ptr = (unsigned int *)yuv_frame + DSP_IMG_WIDTH / 2 * y + x / 2;
	for (idx = y; idx < y + len; idx++)
	{
		*ptr = color;
		ptr += DSP_IMG_WIDTH / 2;
	}
	return 0;
}


int 
draw_rectangle(char *yuv_frame, 
	int xmin, int ymin, 
	int xmax, int ymax, int color)
{
	draw_hline(yuv_frame, xmin, ymin, xmax-xmin, color);
	draw_hline(yuv_frame, xmin, ymax, xmax-xmin, color);
	draw_vline(yuv_frame, xmin, ymin, ymax-ymin, color);
	draw_vline(yuv_frame, xmax, ymin, ymax-ymin, color);
	return 0;
}


int
draw_zoom_rectangle (char *yuv_frame, VLIB_CC *blob, 
        float hfactor, float vfactor, int color)
{
    int xmin, ymin, xmax, ymax; 
    xmin = spint_i(mpysp_i(intsp_i(blob->xmin), hfactor));
    xmax = spint_i(mpysp_i(intsp_i(blob->xmax), hfactor));
    ymin = spint_i(mpysp_i(intsp_i(blob->ymin), vfactor));
    ymax = spint_i(mpysp_i(intsp_i(blob->ymax), vfactor));

	draw_hline(yuv_frame, xmin, ymin, xmax-xmin, color);
	draw_hline(yuv_frame, xmin, ymax, xmax-xmin, color);
	draw_vline(yuv_frame, xmin, ymin, ymax-ymin, color);
	draw_vline(yuv_frame, xmax, ymin, ymax-ymin, color);
    return 0;
}		/* -----  end of function draw_zoom_rectangle  ----- */

