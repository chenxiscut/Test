/*
 * =====================================================================
 *
 *       Filename:  float_hog.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/05/2012 08:11:06 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  dengos (), dengos.w@gmail.com
 *        Company:  scut
 *
 * =====================================================================
 */


#ifndef  float_hog_INC
#define  float_hog_INC


#include "parameters.h"

struct hog_mem
{
    int   gradx[DW_WIDTH * DW_HEIGHT]; 
    int   grady[DW_WIDTH * DW_HEIGHT]; 
    float   cells_hog[SIZE_CELLS_HOG]; 

}; 
/* ----------  end of struct hog_mem  ---------- */


#define HOG_MEM_SIZE sizeof(struct hog_mem_buf)

int
get_gradient (
        const unsigned char *img, 
        const int width, const int height,
        int *gradx, int *grady);


int
get_cells_r_lahog (
        const int *gradx, const int *grady, 
        const int width, const int height, 
        const int cell_width, const int cell_height,
        float *cells_hog);


int
get_blocks_r_lahog (
        const float *cells_hog, 
        const int width, const int height,
        const int cell_width, const int cell_height,
        float *blocks_hog);


int
get_r_lahog_features (
        const unsigned char *img, 
        const int width, const int height,
        const int cell_width, const int cell_height,
        char *mem_buf, float *hog_features);


int
get_cells_hog (
        const int *gradx, const int *grady, 
        const int width, const int height,
        const int cell_width, const int cell_height,
        float *cells_hog);


int
get_blocks_hog (
        const float *cells_hog, 
        const int width, const int height,
        const int cell_width, const int cell_height,
        float *blocks_hog);


int
get_hog_features (
        const unsigned char *img, 
        const int width, const int height,
        const int cell_width, const int cell_height,
        char *mem_buf, float *hog_features);

#endif   /* ----- #ifndef float_hog_INC  ----- */
