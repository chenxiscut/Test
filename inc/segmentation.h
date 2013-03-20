/*
 * =====================================================================
 *
 *       Filename:  segmentation.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/17/2012 10:11:00 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  dengos (), dengos.w@gmail.com
 *        Company:  scut
 *
 * =====================================================================
 */

#ifndef  segmentation_INC
#define  segmentation_INC

int
gp_dt_segment ( const unsigned char *image, unsigned int *bin_image );

int
rough_gp_dt_segment ( const unsigned char *image, unsigned int *bin_image );

int
gp_mean_segment ( const unsigned char *image, unsigned int *bin_image );

int
rough_gp_mean_segment ( const unsigned char *image, unsigned int *bin_image );

int
simple_dt_segment ( const unsigned char *image, unsigned int *bin_image );

int
rough_simple_dt_segment ( const unsigned char *image, unsigned int *bin_image );

int
pack_binary_image ( const unsigned char *seg_img, int size, unsigned int *bin_img);

int
unpack_binary_image ( unsigned int *binary, unsigned char *seg_image );

#endif   /* ----- #ifndef segmentation_INC  ----- */


