/**
 * ========================================================================
 *
 *          @file  segmentation.c
 *
 *         @brief  A source file provide image segment function.
 *
 *
 *        @author  dengos (W), dengos.w@gmail.com, scut
 *       @version  1.0
 *          @date  08/10/2012 11:22:15 AM
 * =========================================================================
 */
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#include "parameters.h"
#include "segmentation.h"
// dsp FastRTS C function
//#include "fastrts_i.h"
//#include "IQmath.h"


/**
 * @brief Compute the maximum.
 *
 * @param a A float type value
 * @param b A float type value
 *
 * @return The maximum value of given two.
 */
float
max (float a, float b)
{
    return a > b ? a : b;
}		
/* -----  end of function max  ----- */



/**
 * @brief Compute the minimum.
 *
 * @param a A float type value.
 * @param b A float type value.
 *
 * @return The minimum value of given two.
 */
float
min (float a, float b)
{
    return a < b ? a : b;
}		
/* -----  end of function min  ----- */



float
newton_sqrt ( float n )
{
    float x = 0.0;
    float xn = 0.0;
    float val;
    int iters = 0;
    int fixed_n = spint_i(n);
    int i;
    for (i = 0; i <= fixed_n; ++i) 
    {
        val = subsp_i(intsp_i(i * i), n);
        if (val == 0.0)     return intsp_i(i);
        if (val > 0.0)
        {
            xn = mpysp_i(intsp_i(i + (i - 1)), 0.5);
            break;
        }
    }

    while (!(iters++ >= 100 || x == xn))
    {
        x = xn;
        xn = subsp_i(x, divsp_i(subsp_i(mpysp_i(x, x), n), mpysp_i(x, 2.0)));
    }
    return xn;
}		/* -----  end of function newton_sqrt  ----- */


/**
 * @brief A step function of the Gradient Projected Dual-threshold
 * segmentation algorthms.
 * In this gradient project function, we only project the gradient
 * under x dimenstion.
 *
 * @param img A gray scale image.[input]
 * @param ver_projection A projected result.[output]
 *
 * @return 0 if OK.
 */
int
gradient_project (const unsigned char *img, int *ver_projection)
{
    int x, y;
    int gradx, grady, mag_square;
    int threshold_square = SEG_THRESHOLD * SEG_THRESHOLD;

    /**
     * @brief compute the magnitude of given image, and then project given 
     * image according to the segment thrush hold to get the a binary image, 
     * after all, project this binary image into x dimension, which is 
     * ver_projection.
     */
    static const unsigned char zeros[IMG_WIDTH] = {0};
    const unsigned char *curr_line = NULL;
    const unsigned char *prev_line = NULL;
    const unsigned char *next_line = NULL;

    prev_line = zeros;
    curr_line = img;
    next_line = img + IMG_WIDTH;
    for ( y = 0; y < IMG_HEIGHT; ) 
    {
        // when x = 0;
        gradx = -0 + curr_line[1];
        grady = prev_line[0] - next_line[0];
        mag_square = gradx * gradx + grady * grady;
        ver_projection[0] += mag_square >= threshold_square ? 1: 0;
        // when x in [1, width -1]
        for ( x = 1; x < IMG_WIDTH - 1; ++x ) 
        {
            gradx = -curr_line[x-1] + curr_line[x+1];
            grady = prev_line[x] - next_line[x];
            mag_square = gradx * gradx + grady * grady;
            ver_projection[x] += mag_square >= threshold_square ? 1:0; 
        }
        // when x = width - 1
        gradx = -curr_line[x-1] + 0;
        grady = prev_line[x] - next_line[x];
        mag_square = gradx * gradx + grady * grady;
        ver_projection[x] += mag_square >= threshold_square ? 1: 0;

        prev_line = curr_line;
        curr_line = next_line;
        if (++y == IMG_HEIGHT -1)
            next_line = zeros;      // the last line
        else
            next_line += IMG_WIDTH;
    }

    return 0;
}		
/* -----  end of function gradient_project  ----- */



int
get_std_threshold ( const int *ver_projection )
{
    int x, fixed_threshold, int_sum;
    float length, sum, mean, std, diff, threshold, lambda;

    length = intsp_i(IMG_WIDTH - 2 * SEG_GPDT_OMIGA); 
    lambda = SEG_LAMBDA;
    
    int_sum = 0; 
    for ( x = SEG_GPDT_OMIGA - 1; x < IMG_WIDTH - SEG_GPDT_OMIGA -1; ++x ) 
		int_sum += ver_projection[x];

    mean = divsp_i(intsp_i(int_sum), length);

    sum = 0.0F; 
    for ( x = SEG_GPDT_OMIGA - 1; x < IMG_WIDTH - SEG_GPDT_OMIGA - 1; ++x ) 
    {
        diff = subsp_i(intsp_i(ver_projection[x]), mean); 
        sum  = addsp_i(sum, mpysp_i(diff, diff)); 
    }

    std       = sqrtsp_i(divsp_i(sum, subsp_i(length, 1.0F)));
    threshold = mpysp_i(lambda, std);
    // truncate from float into int
    // Do relalized that all value in ver_projection are int type.
    fixed_threshold = spint_i(addsp_i(threshold, 0.9999F));

    return fixed_threshold;
}		/* -----  end of function get_std_threshold  ----- */

/**
 * @brief A step function of the Gradient Projected Dual-threshold
 * image segmentation algorithm to compute the variance of the
 * projected data.
 *
 * @param ver_projection A gradient projected on x dimenstion.
 *
 * @return The integer variance of input data: ver_projection.
 */
int
get_mean_threshold (const int *ver_projection)
{
    int x, fixed_threshold, int_sum;
    float length, sum, mean, std, diff, threshold;

    length = intsp_i(IMG_WIDTH - 2 * SEG_GPDT_OMIGA); 

    // mean
    int_sum = 0; 
    for ( x = SEG_GPDT_OMIGA - 1; x < IMG_WIDTH - SEG_GPDT_OMIGA - 1; ++x ) 
		int_sum += ver_projection[x];
    mean = divsp_i(intsp_i(int_sum), length);
     
    // std
    sum = 0.0F; 
    for ( x = SEG_GPDT_OMIGA - 1; x < IMG_WIDTH - SEG_GPDT_OMIGA - 1; ++x ) 
    {
        diff = subsp_i(intsp_i(ver_projection[x]), mean); 
        sum  = addsp_i(sum, mpysp_i(diff, diff)); 
    }

    std = sqrtsp_i(divsp_i(sum, subsp_i(length, 1.0F))); 
    // truncate from float into int
    // Do relalized that all value in ver_projection are int type.
    threshold = subsp_i(mean, mpysp_i(std, 0.5));
    fixed_threshold = spint_i(addsp_i(threshold, 0.9999F));

    return fixed_threshold;
}		
/* -----  end of function get_mean_threshold  ----- */


    
/**
 * @brief Since we project all the gradient information into x dimenstion,
 * the ver_projection, which we provide some kind of region information.
 * We add this get_strips function to pre-compute this region information,
 * before actually do the dual-threshold segment. Remember, you don't want
 * to go-through the whole image, and compute the same region information,
 * (ok, the strip is more offical).
 * More about the strip information array: strip_indicate.
 * In this array, we mark the begin of a strip by provide the end index of 
 * given strip, which means only the begin of a strip can have non-zero value
 * in this array. For example:
 *    If I say strip_indicate[10] = 30, which means there is a strip region
 *    [10, 30].
 *
 * @param ver_projection A gradient project on x dimenstion.
 * @param fixed_threshold The integer version of variance of ver_projection.
 * @param strip_indicate A output array, which provide the strip information.
 * @param begin A output variable, indicate the begin of whole image line.
 * @param end A output variable, indicate the end of whole image line.
 *
 * @return 0 if OK.
 */
int
get_strips (const int *ver_projection, const int fixed_threshold,
        int *strip_indicate, int *begin, int *end)
{
    int is_strip = 0;
    int x, seg_begin, seg_end, strip_begin, strip_end;

    int omiga  = SEG_GPDT_OMIGA;


    /**
     * @brief dual-threshold segmentation based on strip, which is a
     * continuous region with the verticle projection of all members 
     * greater than the fixed variance.
     */

    is_strip  = 0;
    // the default value of seg_begin and seg_end
    seg_begin = (SEG_GPDT_OMIGA * 1.2F); // floor(omiga * 1.2)
    seg_end   = IMG_WIDTH - (SEG_GPDT_OMIGA * 1.2F);
    // pre-compute the association strip index array
    for ( x = 0; x < IMG_WIDTH; ++x ) 
    {
        // with strip_indicate[strip_begin] = {strip_indicate}
        // if strip_indicate[strip_begin] != 0, then there is a legal
        // strip in region [strip_begin, strip_begin+strip_indicate]
//        if (ver_projection[x] * ver_projection[x] < fixed_threshold)
		if (ver_projection[x] < fixed_threshold)
        {
            if (is_strip)
            {
                is_strip = 0; // end of a strip
                // compute the strip_indicate
                // the length of a legal strip >= SEG_STRIP_IMG_WIDTH 
                if (x - strip_begin < SEG_STRIP_WIDTH)
                    continue;

                // a strip between [strip_begin, strip_end]
                // have found a strip, we then need to do dual-threshold
                // segmentation based on this strip. 

                if (strip_begin < omiga && x > IMG_WIDTH - SEG_GPDT_OMIGA)
                {
                    // strip tag case 1
                    // the rare case, a image with one complete strip
                    seg_begin = SEG_GPDT_OMIGA * 1.5F;
                    seg_end   = IMG_WIDTH - (SEG_GPDT_OMIGA * 1.5F);
                }

                if (strip_begin < omiga)
                    strip_begin = omiga;    // strip tag case 1 & 2

                if (x > IMG_WIDTH - SEG_GPDT_OMIGA)
                    strip_end   = IMG_WIDTH - SEG_GPDT_OMIGA;  // strip tag case 1 & 3
                else
                    strip_end   = x+1;   // strip tag case 2 & 4
                //

//				if (strip_end - strip_begin < SEG_STRIP_WIDTH)
//					continue;
                strip_indicate[strip_begin] = strip_end;
            }
            // else not a strip
        }
        else
        {
            if (!is_strip)
            {
                // a begin of a strip
                is_strip = 1; 
                strip_begin = x; 
            }
            // strip extending
        }
    }

    *begin = seg_begin;
    *end   = seg_end;
    return 0;
}		
/* -----  end of function get_strips  ----- */



/**
 * @brief 
 * Shrink the segment result, which contain image pixel value 255 or 0 into
 * binary format, where use bit instead of char to indicate the pixel value.
 * More specifically, we use bit value 1 for pixel value 255, 0 for pixel
 * value 0.
 * For a 352 image line, we only need 352 / 32 = 11 (32-bit int).
 *
 * @param seg_img A image after perform segmentation.
 * @param bin_img A binary image which packed the segment information into
 * 32-bit packed.
 *
 * @return 0 if OK.
 */
int
pack_binary_image ( const unsigned char *seg_img, int size, unsigned int *bin_img)
{
    int idx, x;
    x = 0;
    for ( idx = 0; idx < size; ++idx)
    {
        bin_img[idx] = 0;
        // (idx + 1) << 5  is equal to (idx + 1) * 32
        for (; x < (idx+1) << 5; ++x)
        {
            bin_img[idx] <<= 1;
            bin_img[idx] += seg_img[x] == 255 ? 1 : 0;
        }
    }
    return 0;
}		/* -----  end of function pack_binary_image  ----- */


/**
 * @brief Sometime you may want to unpack the binary image, this is
 * the one function for you..
 *
 * @param binary A binary format segmentation image
 * @param seg_image A Dual-value segmentation image
 *
 * @return 0 if ok.
 */
int
unpack_binary_image ( unsigned int *binary, unsigned char *seg_image )
{
    int idx, x, j;
    unsigned int *bin_line = NULL;
    int value;
    bin_line = binary;
    for (idx = 0; idx < IMG_HEIGHT; ++idx)
    {
        for (x = 0; x < BIN_IMG_WIDTH; ++x)
        {
            value = bin_line[x];
            for (j = 0; j < 32; ++j)
            {
                value = bin_line[x] << j;
                *seg_image++ = value < 0 ? 255 : 0;
            }

        }
        bin_line += BIN_IMG_WIDTH;
    }

    return 0;
}		/* -----  end of function unpack_binary_image  ----- */


/**
 * @brief A step function of the Gradient Projected Dual-threshold
 * segmentation algorithm. After pre-compute the strip information
 * based on the gradient projected information on x dimenstion, we
 * now come to segment the given gray scale image.
 *
 * @param image A gray scale image.
 * @param strip_indicate A array contain the strip information of 
 * given image.
 * @param seg_begin
 * @param seg_end
 * @param bin_image
 *
 * @return 
 */
int
do_dt_segmentation ( const unsigned char *image,
        const int *strip_indicate, const int seg_begin, const int seg_end,
        unsigned int *bin_image)
{
    int x, y, idx;

    const unsigned char *img_line     = NULL;
    unsigned int *bin_line            = NULL;
    unsigned char seg_line[IMG_WIDTH] = {0};

    float trd_L, trd_L1, trd_L2, trd_L3, trd_H;
    int fixed_trd_L, fixed_trd_H;

    int sum;

    int   omiga  = SEG_GPDT_OMIGA;
    float weight = SEG_GPDT_WEIGHT;
    float alpha  = SEG_GPDT_ALPHA;
    float beta   = SEG_GPDT_BETA;
	float gamma  = SEG_GPDT_GAMMA;
    float gray_h = SEG_GPDT_GRAY_H;
    float normalizer = SEG_GPDT_NORMALIZER;

    img_line = image;
    bin_line = bin_image;
    for ( y = 0; y < IMG_HEIGHT; ++y ) 
    {
        // no need to preset seg_lien to 0
        // memset(seg_line, 0, sizeof(char) * IMG_WIDTH); // given the local cache
        for ( x = omiga; x < IMG_WIDTH - SEG_GPDT_OMIGA; ++x ) 
        {
            if (!strip_indicate[x])
                continue;   // no strip begin

            // given a strip, which:
            //    strip begin = x, strip end = strip_indicate[x];

            // compute sum(x-omiga:x+omiga);
            // to save the sum over loop, we add overhead x-omiga and left
            // x+omiga in pre-compute sum.
            sum = img_line[x - omiga];
            for ( idx = x - omiga; idx < x + omiga; ++idx ) 
            {
                sum += img_line[idx];
            }

            
            for ( idx = x; idx < strip_indicate[x]; ++idx ) 
            {
                // compute the sum
                sum +=
                    img_line[idx+omiga] - img_line[idx-omiga];

                trd_L  = addsp_i(mpysp_i(intsp_i(sum), normalizer), alpha);
                trd_L3 = max(mpysp_i(weight, subsp_i(trd_L, alpha)), addsp_i(trd_L, beta));
                trd_L2 = min(trd_L3, addsp_i(trd_L, gamma));
                trd_L1 = min(trd_L2, gray_h);
                trd_H  = max(trd_L1, trd_L);

                fixed_trd_L = spint_i(addsp_i(trd_L, 0.9999F));
                fixed_trd_H = spint_i(addsp_i(trd_H, 0.9999F));

                if ( img_line[idx] >= fixed_trd_H ) 
                    seg_line[idx] = 255;
                else if ( img_line[idx] < fixed_trd_L)
                    seg_line[idx] = 0;
                else
                {
                    if (idx != x)  // not the first pixel of given strip
                        seg_line[idx] = seg_line[idx-1];
                    else
                        seg_line[idx] = 0; // the first pixel of given strip
                }

                // prepare the inter-mid sum for next ride
                sum +=
                    img_line[idx-omiga+1] - img_line[idx-omiga];

            }

            // to avoid some-case infinite loop ==> consistense with matlab version
            if (strip_indicate[x] > x)
                x = strip_indicate[x] - 1; // ++x in for
        }
        // clear the overhead and tail of given line

        for ( idx = 0; idx < seg_begin; ++idx )
            seg_line[idx] = 0;

        for ( idx = seg_end; idx < IMG_WIDTH; ++idx )
            seg_line[idx] = 0;

        // create the binary packed based on segmentation result.
        pack_binary_image(seg_line, BIN_IMG_WIDTH, bin_line);

        img_line += IMG_WIDTH;
        bin_line += BIN_IMG_WIDTH;
    }

    return 0;
}		/* -----  end of function do_dt_segmentation  ----- */


/**
 * @brief A step function of the Gradient Projected Dual-threshold
 * segmentation algorithm. After pre-compute the strip information
 * based on the gradient projected information on x dimenstion, we
 * now come to segment the given gray scale image.
 *
 * @param image A gray scale image.
 * @param strip_indicate A array contain the strip information of 
 * given image.
 * @param seg_begin
 * @param seg_end
 * @param bin_image
 *
 * @return 
 */
int
do_rough_segmentation ( const unsigned char *image,
        const int *strip_indicate, const int seg_begin, const int seg_end,
        unsigned int *bin_image)
{
    int x, y, idx;

    const unsigned char *img_line     = NULL;
    unsigned int *bin_line            = NULL;
    unsigned char seg_line[IMG_WIDTH] = {0};

    float trd_L, trd_L1, trd_L2, trd_L3, trd_H;
    int fixed_trd_L, fixed_trd_H;

    int sum;
    int   omiga  = SEG_GPDT_OMIGA;
    float weight = SEG_GPDT_WEIGHT;
    float alpha  = SEG_GPDT_ALPHA;
    float beta   = SEG_GPDT_BETA;
	float gamma  = SEG_GPDT_GAMMA;
    float gray_h = SEG_GPDT_GRAY_H;
    float normalizer = SEG_GPDT_NORMALIZER;

    img_line = image;
    bin_line = bin_image;
    for ( y = 0; y < IMG_HEIGHT; y+=2 ) 
    {
        // no need to preset seg_lien to 0
        // memset(seg_line, 0, sizeof(char) * IMG_WIDTH); // given the local cache
        for ( x = omiga; x < IMG_WIDTH - SEG_GPDT_OMIGA; ++x ) 
        {
            if (!strip_indicate[x])
                continue;   // no strip begin

            // given a strip, which:
            //    strip begin = x, strip end = strip_indicate[x];

            // compute sum(x-omiga:x+omiga);
            // to save the sum over loop, we add overhead x-omiga and left
            // x+omiga in pre-compute sum.
            sum = img_line[x - omiga];
            for ( idx = x - omiga; idx < x + omiga; ++idx ) 
            {
                sum += img_line[idx];
            }

            
            for ( idx = x; idx < strip_indicate[x]; ++idx ) 
            {
                // compute the sum
                sum +=
                    img_line[idx+omiga] - img_line[idx-omiga];

                trd_L  = addsp_i(mpysp_i(intsp_i(sum), normalizer), alpha);
                trd_L3 = max(mpysp_i(weight, subsp_i(trd_L, alpha)), addsp_i(trd_L, beta));
                trd_L2 = min(trd_L3, addsp_i(trd_L, gamma));
                trd_L1 = min(trd_L2, gray_h);
                trd_H  = max(trd_L1, trd_L);

                fixed_trd_L = spint_i(addsp_i(trd_L, 0.9999F));
                fixed_trd_H = spint_i(addsp_i(trd_H, 0.9999F));

                if ( img_line[idx] >= fixed_trd_H ) 
                    seg_line[idx] = 255;
                else if ( img_line[idx] < fixed_trd_L)
                    seg_line[idx] = 0;
                else
                {
                    if (idx != x)  // not the first pixel of given strip
                        seg_line[idx] = seg_line[idx-1];
                    else
                        seg_line[idx] = 0; // the first pixel of given strip
                }

                // prepare the inter-mid sum for next ride
                sum +=
                    img_line[idx-omiga+1] - img_line[idx-omiga];

            }

            // to avoid some-case infinite loop ==> consistense with matlab version
            if (strip_indicate[x] > x)
                x = strip_indicate[x] - 1; // ++x in for
        }
        // clear the overhead and tail of given line

        for ( idx = 0; idx < seg_begin; ++idx )
            seg_line[idx] = 0;

        for ( idx = seg_end; idx < IMG_WIDTH; ++idx )
            seg_line[idx] = 0;

        // create the binary packed based on segmentation result.
        pack_binary_image(seg_line, BIN_IMG_WIDTH, bin_line);

        // simply copy the odd line result from even line to reduce the work..
        for (x = 0; x < BIN_IMG_WIDTH; ++x) 
            bin_line[x + BIN_IMG_WIDTH] = bin_line[x];

        img_line += 2 * IMG_WIDTH;
        bin_line += 2 * BIN_IMG_WIDTH;
    }

    return 0;
}		/* -----  end of function do_rough_segmentation  ----- */



/**
 * @brief Gradient Projected Dual-threshold segmentation.
 *
 * @param image A gray scale image, with size IMG_WIDTH x IMG_HEIGHT
 * @param bin_image A 32-bit packed bin image
 *
 * @return 0 if OK.
 */
int
gp_dt_segment ( const unsigned char *image, unsigned int *bin_image )
{
     // initialize to 0
    int ver_projection[IMG_WIDTH] = {0}; 
    int strip_indicate[IMG_WIDTH] = {0}; 
    int fixed_threshold, seg_begin, seg_end; 
     
    // compute the magnitude of given image, and then 
    // project given image according to the segment thrush hold to
    // get the a binary image, after all, project this binary image
    // into x dimension, which is ver_projection
    gradient_project(image, ver_projection);

    fixed_threshold = get_std_threshold(ver_projection);

    get_strips(ver_projection, fixed_threshold, 
            strip_indicate, &seg_begin, &seg_end);

    // compute the dual-threshold
    do_dt_segmentation(image, strip_indicate, seg_begin, seg_end, bin_image);
    return 0;
}		/* -----  end of function gp_dt_segment  ----- */


/**
 * @brief Gradient Projected Dual-threshold segmentation.
 *
 * @param image A gray scale image, with size IMG_WIDTH x IMG_HEIGHT
 * @param bin_image A 32-bit packed bin image
 *
 * @return 0 if OK.
 */
int
rough_gp_dt_segment ( const unsigned char *image, unsigned int *bin_image )
{
     // initialize to 0
    int ver_projection[IMG_WIDTH] = {0}; 
    int strip_indicate[IMG_WIDTH] = {0}; 
    int fixed_threshold, seg_begin, seg_end; 
     
    // compute the magnitude of given image, and then 
    // project given image according to the segment thrush hold to
    // get the a binary image, after all, project this binary image
    // into x dimension, which is ver_projection
    gradient_project(image, ver_projection);

    fixed_threshold = get_std_threshold(ver_projection);

    get_strips(ver_projection, fixed_threshold, 
            strip_indicate, &seg_begin, &seg_end);

    // compute the dual-threshold
    do_rough_segmentation(image, strip_indicate, seg_begin, seg_end, bin_image);
    return 0;
}		/* -----  end of function rough_gp_dt_segment  ----- */


int
gp_mean_segment ( const unsigned char *image, unsigned int *bin_image )
{
     // initialize to 0
    int ver_projection[IMG_WIDTH] = {0}; 
    int strip_indicate[IMG_WIDTH] = {0}; 
    int fixed_threshold, seg_begin, seg_end; 
     
    // compute the magnitude of given image, and then 
    // project given image according to the segment thrush hold to
    // get the a binary image, after all, project this binary image
    // into x dimension, which is ver_projection
    gradient_project(image, ver_projection);

    fixed_threshold = get_mean_threshold(ver_projection);

    get_strips(ver_projection, fixed_threshold, 
            strip_indicate, &seg_begin, &seg_end);
   
    // compute the dual-threshold
    do_dt_segmentation(image, strip_indicate, seg_begin, seg_end, bin_image);

    return 0;
}		/* -----  end of function gp_mean_segment  ----- */


int
rough_gp_mean_segment ( const unsigned char *image, unsigned int *bin_image )
{
     // initialize to 0
    int ver_projection[IMG_WIDTH] = {0}; 
    int strip_indicate[IMG_WIDTH] = {0}; 
    int fixed_threshold, seg_begin, seg_end; 
     
    // compute the magnitude of given image, and then 
    // project given image according to the segment thrush hold to
    // get the a binary image, after all, project this binary image
    // into x dimension, which is ver_projection
    gradient_project(image, ver_projection);

    fixed_threshold = get_mean_threshold(ver_projection);

    get_strips(ver_projection, fixed_threshold, 
            strip_indicate, &seg_begin, &seg_end);
   
    // compute the dual-threshold
    do_rough_segmentation(image, strip_indicate, seg_begin, seg_end, bin_image);

    return 0;
}		/* -----  end of function rough_gp_mean_segment  ----- */



/**
 * @brief A function combine two operation: dual-threshold segment and
 * using the segmentation image to create a 32-bit packed bin image.
 *
 * @param image A gray scale image, has size IMG_HEIGHT x IMG_WIDTH. 
 * @param bin_image A 32-bit packed bin image after performing dual-threshold
 * segmentation.
 *
 * @return 0 if OK.
 */
int
simple_dt_segment ( const unsigned char *image, unsigned int *bin_image )
{
    int x, y;
    int sum;

    const unsigned char *img_line     = NULL;
    unsigned int *bin_line            = NULL;
    // segmentation image line buffer
    unsigned char seg_line[IMG_WIDTH] = {0};

    float trd_L, trd_L3, trd_L2, trd_L1, trd_H;
    int fixed_trd_L, fixed_trd_H;
    int overhead  = SEG_SDT_OVERHEAD;

    int   omiga   = SEG_SDT_OMIGA;
    float weight  = SEG_SDT_WEIGHT;
    float alpha   = SEG_SDT_ALPHA;
    float beta    = SEG_SDT_BETA;
	float gamma   = SEG_SDT_GAMMA;
    float gray_h  = SEG_SDT_GRAH_H;
    float normalizer = SEG_SDT_NORMALIZER;

    img_line = image;
    bin_line = bin_image;
    for ( y = 0; y < IMG_HEIGHT; ++y ) 
    {
        // pre-compute the sum
        // extra img_line[x-omiga], lack img_line[x+omiga]
        sum = img_line[0];
        for ( x = 0; x < 2 * SEG_SDT_OMIGA; ++x ) 
            sum += img_line[x];

        for ( x = omiga; x < IMG_WIDTH - SEG_SDT_OMIGA; ++x) 
        {
            // compute the sum
            sum +=
                 img_line[x+omiga] - img_line[x-omiga];

            trd_L  = addsp_i(mpysp_i(intsp_i(sum), normalizer), alpha);
            trd_L3 = max(mpysp_i(weight, subsp_i(trd_L, alpha)), addsp_i(trd_L, beta));
            trd_L2 = min(trd_L3, addsp_i(trd_L, gamma));
            trd_L1 = min(trd_L2, gray_h);
            trd_H  = max(trd_L1, trd_L);

            fixed_trd_L = spint_i(addsp_i(trd_L, 0.9999F));
            fixed_trd_H = spint_i(addsp_i(trd_H, 0.9999F));

            if ( img_line[x] >= fixed_trd_H ) 
                seg_line[x] = 255;
            else if ( img_line[x] < fixed_trd_L)
                seg_line[x] = 0;
            else
            {
                if (x != omiga)  // not the first pixel counted
                    seg_line[x] = seg_line[x-1];
                else
                    seg_line[x] = 0; // the first pixel counted
            }
           
            // pre-compute the inter-mid sum
            sum +=
                img_line[x-omiga+1] - img_line[x-omiga];
        }

        // ignore the omiga * 1.5 head
        for ( x = 0; x < overhead; ++x ) 
            seg_line[x] = 0;
        // ignore the omiga * 1.5 tail
        for ( x = IMG_WIDTH - overhead - 1; x < IMG_WIDTH; ++x)
            seg_line[x] = 0;

        // create the binary packed based on segmentation result.
        pack_binary_image(seg_line, BIN_IMG_WIDTH, bin_line);

        img_line += IMG_WIDTH;
        bin_line += BIN_IMG_WIDTH;
    }

    return 0;
}		/* -----  end of function simple_dt_segment  ----- */


/**
 * @brief A function combine two operation: dual-threshold segment and
 * using the segmentation image to create a 32-bit packed bin image.
 *
 * @param image A gray scale image, has size IMG_HEIGHT x IMG_WIDTH. 
 * @param bin_image A 32-bit packed bin image after performing dual-threshold
 * segmentation.
 *
 * @return 0 if OK.
 */
int
rough_simple_dt_segment ( const unsigned char *image, unsigned int *bin_image )
{
    int x, y;
    int sum;

    const unsigned char *img_line     = NULL;
    unsigned int *bin_line            = NULL;
    // segmentation image line buffer
    unsigned char seg_line[IMG_WIDTH] = {0};

    float trd_L, trd_L3, trd_L2, trd_L1, trd_H;
    int fixed_trd_L, fixed_trd_H;
    int overhead  = SEG_SDT_OVERHEAD;

    int   omiga   = SEG_SDT_OMIGA;
    float weight  = SEG_SDT_WEIGHT;
    float alpha   = SEG_SDT_ALPHA;
    float beta    = SEG_SDT_BETA;
	float gamma   = SEG_SDT_GAMMA;
    float gray_h  = SEG_SDT_GRAH_H;
    float normalizer = SEG_SDT_NORMALIZER;

    img_line = image;
    bin_line = bin_image;
    for ( y = 0; y < IMG_HEIGHT; y+=2 ) 
    {
        // pre-compute the sum
        // extra img_line[x-omiga], lack img_line[x+omiga]
        sum = img_line[0];
        for ( x = 0; x < 2 * SEG_SDT_OMIGA; ++x ) 
            sum += img_line[x];

        for ( x = omiga; x < IMG_WIDTH - SEG_SDT_OMIGA; ++x) 
        {
            // compute the sum
            sum +=
                 img_line[x+omiga] - img_line[x-omiga];

            trd_L  = addsp_i(mpysp_i(intsp_i(sum), normalizer), alpha);
            trd_L3 = max(mpysp_i(weight, subsp_i(trd_L, alpha)), addsp_i(trd_L, beta));
            trd_L2 = min(trd_L3, addsp_i(trd_L, gamma));
            trd_L1 = min(trd_L2, gray_h);
            trd_H  = max(trd_L1, trd_L);

            fixed_trd_L = spint_i(addsp_i(trd_L, 0.9999F));
            fixed_trd_H = spint_i(addsp_i(trd_H, 0.9999F));

            if ( img_line[x] >= fixed_trd_H ) 
                seg_line[x] = 255;
            else if ( img_line[x] < fixed_trd_L)
                seg_line[x] = 0;
            else
            {
                if (x != omiga)  // not the first pixel counted
                    seg_line[x] = seg_line[x-1];
                else
                    seg_line[x] = 0; // the first pixel counted
            }
           
            // pre-compute the inter-mid sum
            sum +=
                img_line[x-omiga+1] - img_line[x-omiga];
        }

        // ignore the omiga * 1.5 head
        for ( x = 0; x < overhead; ++x ) 
            seg_line[x] = 0;
        // ignore the omiga * 1.5 tail
        for ( x = IMG_WIDTH - overhead - 1; x < IMG_WIDTH; ++x)
            seg_line[x] = 0;

        // create the binary packed based on segmentation result.
        pack_binary_image(seg_line, BIN_IMG_WIDTH, bin_line);

        // simply copy the odd line result from even line to reduce the work..
        for (x = 0; x < BIN_IMG_WIDTH; x++) 
            bin_line[x + BIN_IMG_WIDTH] = bin_line[x];

        img_line += 2 * IMG_WIDTH;
        bin_line += 2 * BIN_IMG_WIDTH;
    }

    return 0;
}		/* -----  end of function rough_simple_dt_segment  ----- */



