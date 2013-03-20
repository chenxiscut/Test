/*
 * =====================================================================
 *
 *       Filename:  float_hog.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/05/2012 08:09:55 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  dengos (), dengos.w@gmail.com
 *        Company:  scut
 *
 * =====================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "float_hog.h"
//#include "fastrts_i.h"


float
logN ( float x, float base )
{
    int integer = 0;
    float decimal = 0.0F;
    float partial = 0.5F;
    float result = 0.0F;

    if (x < 1.0F && base < 1.0F)
        printf ( "error!!!!!!!!!!\n" );

    while (x < 1.0F)
    {
        integer--;
        x = mpysp_i(x, base);
    }

    while (x >= base)
    {
        integer++;
        x = divsp_i(x, base);
    }

    x = mpysp_i(x, x);
    while (partial > 1e-6)
    {
        if (x >= base)
        {
            decimal += partial;
            x = divsp_i(x, base);
        }
        partial = mpysp_i(partial, 0.5F);
        x = mpysp_i(x, x);
    }

    result = addsp_i(decimal, intsp_i(integer));
    return result;
}		/* -----  end of function logN  ----- */


/* 
 * ===  FUNCTION  ======================================================
 *         Name:  get_gradient
 *  Description:  
 * =====================================================================
 */
int
get_gradient (const unsigned char *img, 
        const int width, const int height,
        int *gradx, int *grady)
{
    int x, y; 

    static const unsigned char zeros[DW_WIDTH] = {0};
    const unsigned char *curr_line = NULL;
    const unsigned char *prev_line = NULL;
    const unsigned char *next_line = NULL;

    prev_line = zeros;
    curr_line = img;
    next_line = img + width;
    for ( y = 0; y < height; ) 
    {
        // when x = 0
        *gradx++ = -0 + curr_line[1];
        *grady++ = prev_line[0] - next_line[0];
        // when x in [1, DW_WIDTH - 1)
        for ( x = 1; x < width - 1; ++x ) 
        {
            *gradx++ = -curr_line[x-1] + curr_line[x+1]; 
            *grady++ = prev_line[x] - next_line[x]; 
        }
        // when x = DW_WIDTH - 1
        *gradx++ = -curr_line[x-1] + 0;
        *grady++ = prev_line[x] - next_line[x];

        prev_line = curr_line;
        curr_line = next_line;
        if (++y == height - 1)
            next_line = zeros;   // the last line
        else
            next_line += width;
    }

    return 0;
}		
/* -----  end of function get_gradient  ----- */



/* 
 * ===  FUNCTION  ======================================================
 *         Name:  get_bin_label
 *  Description:  
 * =====================================================================
 */
static int
get_bin_label (float x, float y)
{
    int label;
    // we have 9 bins
    static float bounders[NUM_BINS - 1] = {
        -2.747477e+00F,
        -1.191754e+00F,
        -5.773503e-01F,
        -1.763270e-01F,
         1.763270e-01F,
         5.773503e-01F,
         1.191754e+00F,
         2.747477e+00F,
    };

    float ratio = divsp_i(y, addsp_i(x, 1e-05F));     
    for ( label = 0; label < NUM_BINS - 1; ++label ) 
    {
        if (ratio < bounders[label])
            break;
    }

    return label;
}		
/* -----  end of function get_bin_label  ----- */


/* 
 * ===  FUNCTION  ======================================================
 *         Name:  get_cells_r_lahog
 *  Description:  
 * =====================================================================
 */
int
get_cells_r_lahog (
        const int *gradx, const int *grady, 
        const int width, const int height, 
        const int cell_width, const int cell_height,
        float *cells_hog)
{
    int pixel_idx = 0; 
    int cell_x, cell_y, cell_max_x; 
    int label = 0; 

    float x, y; 
    float magnitude; 
    float *cell_hog = NULL; 


    cell_max_x = width / cell_width; 

    memset(cells_hog, 0, 
            sizeof(float) * (width / cell_width) * (height / cell_height) * NUM_BINS); 

    for ( pixel_idx = 0; pixel_idx < width * height; ++pixel_idx ) 
    {
        //  pixel_x / CELL_WIDTH
        cell_x = (pixel_idx % width) / cell_width; 
        //  pixel_y / CELL_HEIGHT
        cell_y = (pixel_idx / width) / cell_height; 

        x = intsp_i(*gradx++); 
        y = intsp_i(*grady++); 
        magnitude = sqrtsp_i(addsp_i(mpysp_i(x, x), mpysp_i(y, y))); 

        label = get_bin_label(x, y);

        cell_hog = cells_hog + (cell_y * cell_max_x  + cell_x) * NUM_BINS; 
        cell_hog[label] = addsp_i(cell_hog[label], magnitude);
    }

    return 0;
}		
/* -----  end of function get_cells_r_lahog  ----- */



/* 
 * ===  FUNCTION  ======================================================
 *         Name:  get_blocks_r_lahog
 *  Description:  
 * =====================================================================
 */
int
get_blocks_r_lahog (const float *cells_hog, 
        const int width, const int height,
        const int cell_width, const int cell_height,
        float *blocks_hog)
{
    int block_x, block_y, bin_idx;
    int block_max_x, block_max_y, cell_max_x; 
    
    int num_cells_per_block 
        = BLOCK_WIDTH_CELL * BLOCK_HEIGHT_CELL; 
    int size_block_hog = num_cells_per_block * NUM_BINS; 

    float normalizer; 
    float tmp; 
    float sum_of_block; 
    float block_weight; 
    float log_ret;
    float *block_hog = NULL; 
    float *block_hog_begin = NULL; 
    const float *cell_hog = NULL; 


    // num of blocks in current detected window
//    float weight[5 * 11]; 
//    float *ptr_weight = weight; 

    block_max_x = width / cell_width - 1;
    block_max_y = height / cell_height - 1; 
    cell_max_x = block_max_x + 1; 
    for ( block_y = 0; block_y < block_max_y; ++block_y ) 
    {
        for ( block_x = 0; block_x < block_max_x; ++block_x ) 
        {
            /*
             * using the original way to compute the block_hog
             * and cell_hog, but be cleared that there are plenty
             * of optimize work can be done.
             */
//            block_hog_begin = blocks_hog; 
//          blocks_hog(:, 1, x, y) = cells_hog(:, x, y)
            block_hog_begin
                = blocks_hog + (block_y * block_max_x + block_x) 
                    * size_block_hog; 
            block_hog = block_hog_begin; 
            cell_hog 
                = cells_hog + (block_y * cell_max_x + block_x) 
                    * NUM_BINS; 
            memcpy(block_hog, cell_hog, sizeof(float) * NUM_BINS); 
//          blocks_hog(:, 2, x, y) = cells_hog(:, x+1, y)
            block_hog += NUM_BINS; 
            cell_hog
                = cells_hog + (block_y * cell_max_x + block_x + 1) 
                    * NUM_BINS; 
            memcpy(block_hog, cell_hog, sizeof(float) * NUM_BINS); 
//          blocks_hog(:, 3, x, y) = cells_hog(:, x, y+1)
            block_hog += NUM_BINS; 
            cell_hog
                = cells_hog + ((block_y+1) * cell_max_x + block_x) 
                    * NUM_BINS; 
             memcpy(block_hog, cell_hog, sizeof(float) * NUM_BINS); 
//          blocks_hog(:, 4, x, y) = cells_hog(:, x+1, y+1)
             block_hog += NUM_BINS; 
            cell_hog
                = cells_hog + ((block_y+1) * cell_max_x + block_x + 1) 
                    * NUM_BINS; 
             memcpy(block_hog, cell_hog, sizeof(float) * NUM_BINS); 

             normalizer = 0.0F; 
             sum_of_block = 0.0F; 
             // reset block_hog to the begin of current block
             block_hog = block_hog_begin; 

             for ( bin_idx = 0; bin_idx < size_block_hog; ++bin_idx ) 
             {
                 normalizer 
                     = addsp_i(normalizer, 
                             mpysp_i(block_hog[bin_idx], block_hog[bin_idx]));
                 // added by R_LAHOG
                 sum_of_block = addsp_i(sum_of_block, block_hog[bin_idx]);
                 // end added
             }
             normalizer = sqrtsp_i(addsp_i(normalizer, 1e-6F)); 

             // added by R_LAHOG
             block_weight = 0.0F; 

             if (sum_of_block < -1e-6 || sum_of_block > 1e-6)
             {
                 // sum_of_block != 0
                 for ( bin_idx = 0; 
                         bin_idx < size_block_hog; ++bin_idx ) 
                 {
                     tmp = divsp_i(block_hog[bin_idx], sum_of_block); 
                     if (tmp > -1e-6 && tmp < 1e-6)
                         tmp = 1.0F; // tmp  ==  

                     log_ret = logN(tmp, 2.0F);
//                     log_ret = _IQ19toF(_IQ19log(_FtoIQ19(tmp)));
                     block_weight = 
                         addsp_i(block_weight, mpysp_i(-tmp, log_ret));
//                     block_weight 
//                         = addsp_i(block_weight, mpysp_i(-tmp, divsp_i(log_ret, log_base)));
                 }
             }
             // end added

             normalizer = recipsp_i(normalizer);
             for ( bin_idx = 0; bin_idx < size_block_hog; ++bin_idx ) 
             {
                 // no global hog weight normalization
                 block_hog[bin_idx] = mpysp_i(block_hog[bin_idx], normalizer);
                 if (block_hog[bin_idx] > -1e-6 && block_hog[bin_idx] < 1e-6)
                     block_hog[bin_idx] = 1e-4;
                 block_hog[bin_idx] = mpysp_i(block_weight, block_hog[bin_idx]);
             } 
        }
        // added by R_LAHOG
        // end added
    }

    // added by R_LAHOG


    // end added

    return 0;
}		
/* -----  end of function get_blocks_r_lahog  ----- */



/* 
 * ===  FUNCTION  ======================================================
 *         Name:  get_r_lahog_features
 *  Description:  
 * =====================================================================
 */
int
get_r_lahog_features (const unsigned char *img, 
        const int width, const int height,
        const int cell_width, const int cell_height,
        char *mem_buf, 
        float *hog_features
        )
{
    struct hog_mem *ptr_hog_mem = (struct hog_mem *)mem_buf; 
    int *gradx = ptr_hog_mem->gradx; 
    int *grady = ptr_hog_mem->grady; 
    float *cells_hog = ptr_hog_mem->cells_hog; 

    get_gradient (img, width, height, gradx, grady); 

    get_cells_r_lahog(gradx, grady, 
            width, height, cell_width, cell_height, cells_hog); 

    get_blocks_r_lahog(cells_hog, 
            width, height, cell_width, cell_height, hog_features); 

    return 0;
}		
/* -----  end of function get_r_lahog_features  ----- */



/* 
 * ===  FUNCTION  ======================================================
 *         Name:  get_cells_hog
 *  Description:  
 * =====================================================================
 */
int
get_cells_hog (const int *gradx, const int *grady, 
        const int width, const int height,
        const int cell_width, const int cell_height,
        float *cells_hog)
{
    size_t pixel_idx = 0; 
    size_t cell_x, cell_y, cell_max_x; 
    int label = 0; 

    float x, y; 
    float magnitude; 
    float *cell_hog = NULL; 


    cell_max_x = width / cell_width; 

    memset(cells_hog, 0, 
            sizeof(float) * (width / cell_width) * (height / cell_height) * NUM_BINS); 

    for ( pixel_idx = 0; pixel_idx < width * height; ++pixel_idx ) 
    {
        //  pixel_x / CELL_WIDTH
        cell_x = (pixel_idx % width) / cell_width; 
        //  pixel_y / CELL_HEIGHT
        cell_y = (pixel_idx / width) / cell_height; 

        x = intsp_i(*gradx++); 
        y = intsp_i(*grady++); 
        magnitude = sqrtsp_i(addsp_i(mpysp_i(x, x), mpysp_i(y, y))); 
        
        label = get_bin_label(x, y);

        cell_hog = cells_hog + (cell_y * cell_max_x  + cell_x) * NUM_BINS; 
        cell_hog[label] = addsp_i(cell_hog[label], magnitude);
    }

    return 0;
}		
/* -----  end of function get_cells_hog  ----- */



/* 
 * ===  FUNCTION  ======================================================
 *         Name:  get_blocks_hog
 *  Description:  
 * =====================================================================
 */
int
get_blocks_hog (const float *cells_hog, 
        const int width, const int height,
        const int cell_width, const int cell_height,
        float *blocks_hog)
{
    size_t block_x, block_y, bin_idx;
    size_t block_max_x, block_max_y, cell_max_x; 
    
    size_t num_cells_per_block 
        = BLOCK_WIDTH_CELL * BLOCK_HEIGHT_CELL; 
    size_t size_block_hog = num_cells_per_block * NUM_BINS; 

    float normalizer; 
    float *block_hog = NULL; 
    float *block_hog_begin = NULL; 
    const float *cell_hog = NULL; 

    block_max_x = width / cell_width - 1;
    block_max_y = height / cell_height - 1; 
    cell_max_x = block_max_x + 1; 
    for ( block_y = 0; block_y < block_max_y; ++block_y ) 
    {
        for ( block_x = 0; block_x < block_max_x; ++block_x ) 
        {
            /*
             * using the original way to compute the block_hog
             * and cell_hog, but be cleared that there are plenty
             * of optimize work can be done.
             */
//            block_hog_begin = blocks_hog; 
//          blocks_hog(:, 1, x, y) = cells_hog(:, x, y)
            block_hog_begin
                = blocks_hog + (block_y * block_max_x + block_x) 
                    * size_block_hog; 
            block_hog = block_hog_begin; 
            cell_hog 
                = cells_hog + (block_y * cell_max_x + block_x) 
                    * NUM_BINS; 
            memcpy(block_hog, cell_hog, sizeof(float) * NUM_BINS); 
//          blocks_hog(:, 2, x, y) = cells_hog(:, x+1, y)
            block_hog += NUM_BINS; 
            cell_hog
                = cells_hog + (block_y * cell_max_x + block_x + 1) 
                    * NUM_BINS; 
            memcpy(block_hog, cell_hog, sizeof(float) * NUM_BINS); 
//          blocks_hog(:, 3, x, y) = cells_hog(:, x, y+1)
            block_hog += NUM_BINS; 
            cell_hog
                = cells_hog + ((block_y+1) * cell_max_x + block_x) 
                    * NUM_BINS; 
             memcpy(block_hog, cell_hog, sizeof(float) * NUM_BINS); 
//          blocks_hog(:, 4, x, y) = cells_hog(:, x+1, y+1)
             block_hog += NUM_BINS; 
            cell_hog
                = cells_hog + ((block_y+1) * cell_max_x + block_x + 1) 
                    * NUM_BINS; 
             memcpy(block_hog, cell_hog, sizeof(float) * NUM_BINS); 

             normalizer = 0.0F; 
             // reset block_hog to the begin of current block
             block_hog = block_hog_begin; 
             for ( bin_idx = 0; bin_idx < size_block_hog; ++bin_idx) 
             {
                 normalizer 
                     = addsp_i(normalizer, 
                             mpysp_i(block_hog[bin_idx], block_hog[bin_idx]));

             }
             normalizer = sqrtsp_i(addsp_i(normalizer, 1e-6F)); 
             normalizer = recipsp_i(normalizer);

             for ( bin_idx = 0; bin_idx < size_block_hog; ++bin_idx) 
             {
                 block_hog[bin_idx] = mpysp_i(block_hog[bin_idx], normalizer);
             } 
        }
    }

    return 0;
}		
/* -----  end of function get_blocks_hog  ----- */




/* 
 * ===  FUNCTION  ======================================================
 *         Name:  get_hog_features
 *  Description:  
 * =====================================================================
 */
int
get_hog_features (const unsigned char *img, 
        const int width, const int height,
        const int cell_width, const int cell_height,
        char *mem_buf, 
        float *hog_features)
{
    struct hog_mem *ptr_hog_mem = (struct hog_mem *)mem_buf; 
    int *gradx = ptr_hog_mem->gradx; 
    int *grady = ptr_hog_mem->grady; 
    float *cells_hog = ptr_hog_mem->cells_hog; 

    get_gradient(img, width, height, gradx, grady); 

    get_cells_hog(gradx, grady, 
            width, height, cell_width, cell_height,
            cells_hog); 

    get_blocks_hog(cells_hog, 
            width, height, cell_width, cell_height,
            hog_features); 

    return 0;
}		
/* -----  end of function get_hog_features  ----- */



