/**
 * ===================================================================
 *
 *          @file  pedestrian_detection.c
 *         @brief
 *
 *        @author  dengos (W), dengos.w@gmail.com,
 *       @version  1.0
 *          @date  08/31/2012 03:27:07 PM
 *
 * ===================================================================
 */

#include <VLIB_prototypes.h>
#include <VLIB_testUtils.h>
#include "parameters.h"
#include "segmentation.h"
#include "image.h"
#include "float_hog.h"
#include "svm.h"
//#include "IQmath.h"

extern float far_weight[];
extern float far_bias;
extern float mid_weight[];
extern float mid_bias;
extern float near_weight[];
extern float near_bias;

static unsigned char gray_image_mem[IMG_HEIGHT * IMG_WIDTH];
static unsigned int  bin_image_mem[IMG_HEIGHT * BIN_IMG_WIDTH];
static unsigned char blob_image_mem[DW_HEIGHT * DW_WIDTH];
static unsigned char std_blob_image_mem[DW_HEIGHT * DW_WIDTH];

static float features_mem[NUM_FEATURES + ROI_FEATURES_SIZE];
static struct hog_mem hog_buffer_mem;

/**
 * @brief A static global variable for pedestrian detection.
 * This variable must be initialized by
 * VLIB_initConnectedComponentsList function, which will be
 * called in init_pedestrian_detection function, before
 * perform any VLIB Connected Components function on it.
 *
 */
static VLIB_CCHandle *handle = NULL;

/**
 * @brief A macro which define the size of internal memory
 * buffers, which is recommended to use in pPrimaryBuf.
 */
#define INTERNAL_BUF_SIZE  1024 * 500
/**
 * @brief A macro which define the size of external memory
 * buffers, which is recommended to use in pOverflowBuf.
 */
#define EXTERNAL_BUF_SIZE  (1024 * 500)
unsigned char pPrimaryBuf1[INTERNAL_BUF_SIZE];
unsigned char pPrimaryBuf2[INTERNAL_BUF_SIZE];
unsigned char pOverflowBuf1[EXTERNAL_BUF_SIZE];
unsigned char pOverflowBuf2[EXTERNAL_BUF_SIZE];


int
draw_zoom_rectangle (char *yuv_frame, VLIB_CC *blob,
        float hfactor, float vfactor, int color);

extern int
VLIB_extractLumaFromUYUV(
        char *,
        unsigned short,
        unsigned short,
        unsigned short,
        char *);

/**
 * @brief
 * Documentataion below is from DSP VLIB pdf, Page 35-36.
 *
 * 1. Introduction
 * A video surveillance application may represent foreground objects
 * as connected components, or blobs of image pixels. Detection or
 * segmentation algorithms can produce a binary mask that identifies
 * each pixel as belonging either to the foreground or background.
 * Connected components algorithms group and labels discrete image
 * regions as blobs.
 *
 * 2. Functions
 * The primary function for grouping and labeling blobs in a binary
 * image is VLIB_createConnectedComponentsList. The input to this
 * function is a 32-bit packed binary image, such as foreground mask,
 * and the output is a pointer to a list of 4- or 8-connected
 * components (the list is acutally a private structures,
 * VLIB_createCCMap8Bit; so that meaningful information about the
 * connected components can be extracted.
 *
 * The support function, VLIB_createCCMap8Bit, produces an 8-bin 2D
 * map that labels every pixel in the image with its corresponding
 * blob ID. Pixels associated with the background are all given
 * ID = 0. Other support functions that extract blob information
 * from the list are: VLIB_getNumCCs and VLIB_getCCFeatures. The
 * former returns the number of connected components in the list
 * while the latter reveals features of the component as defined by
 * the follow structe:
 *      typedef struct {
 *          int area;
 *          int xsum;
 *          int ysum;
 *
 *          int xmin;
 *          int ymin;
 *          int xmax;
 *          int ymax;
 *      } VLIB_CC;
 * Additional features will be added as required.
 *
 * 3. VLIB_CCHandle
 * When allocating memory for the handle to connected components,
 * be sure to use VLIB_GetSizeOfCCHandle(), which returns the size in
 * bytes. For example,
 *      int sizeOfCCHanle = VLIB_GetSizeOfCCHandle();
 *      VLIB_CCHandle *handle
 *          = (VLIB_CCHandle *) MEM_alloc(DDR2HEAP, sizeOfCCHanle, 8);
 *
 * */

/**
 * @brief Initialization function, must be called to identify the
 * available memory buffers each time the address changes, that is,
 * if persistent memory locations are available, only call the
 * initialization function once in the beginning. These functions
 * are not re-entrant.
 *
 * @param handle Function returns a pointer to the object handle,
 * which is a private structure representing the list of connected
 * components.
 * @param pPrimaryBuf1 Pointer to primary buffer 1
 * @param bytesPrimaryBuf1 Number of bytes of buffer 1
 * @param pPrimaryBuf2 Pointer to primary buffer 2
 * @param bytesPrimaryBuf2 Number of bytes of buffer 2
 * @param pOverflowBuf1 Pointer to overflow buffer 1
 * @param bytesOverflowBuf1 Number of bytes of buffer 1
 * @param pOverflowBuf2 Pointer to overflow buffer 2
 * @param bytesOverflowBuf2 Number of bytes of buffer 2
 *
 * @return Unknown.
 */
extern int
VLIB_initConnectedComponentsList(
        VLIB_CCHandle *handle,
        unsigned char *pPrimaryBuf1,
        int bytesPrimaryBuf1,
        unsigned char *pPrimaryBuf2,
        int bytesPrimaryBuf2,
        unsigned char *pOverflowBuf1,
        int bytesOverflowBuf1,
        unsigned char *pOverflowBuf2,
        int bytesOverflowBuf2);



/**
 * @brief
 * Documentation below is from TI DSP VLIB pdf, Page 36.
 * 1. For best performance, This function should use internal and
 * external memory buffers for processing the 32-bit packed binary
 * foreground image (mask) and for storing the list of connected
 * components. The amount of memory actually required is scene
 * dependent.
 *
 * 2. This function is designed to use pPrimaryBuf1 and pPrimaryBuf2
 * (both are defined in VLIB_initConnectedComponentsList function)
 * memory buffers by internal, chache-enhanced double buffering. We
 * recommand that pPrimaryBuf1 and pPrimaryBuf2 point to buffers in
 * L1 Data and/or L2 Data memory for the best performance. During
 * processing, the generated connected components are influenced by the
 * image size and content.
 *
 * 3. This function uses pPrimaryBuf1 and pPrimaryBuf2 but will also
 * consume memory in pOverflowBuf1 and pOverflowBuf2, which are also
 * defined in VLIB_initConnectedComponentsList function, as needed,
 * after the primary buffers become exhausted.
 *
 * 4. To be clear, all memory buffer assignments can use external DDR2
 * memory exclusively, but performance will be slower. It is illegal to
 * supply null pointers as arguments to pPrimaryBuf1 or pPrimaryBuf2.
 * Using the largest possible size for pPrimaryBuf1 and pPrimaryBuf2
 * is also recommanded. It is appropriate to place pOverflowBuf1 and
 * pOverflowBuf2 in external memory. Note: Make sure that the cache
 * is enabled. Also be aware that overlapping memory buffers produces
 * unknown results and is not recommanded.
 *
 * @param handle Function returns a pointer to the object handle, which
 * is a private structure representing the list of connected
 * components.
 * @param width Width of input image.
 * @param rowsInImg Height of input image.
 * @param p32BitPackedFGMask 32-bit packed binary image.
 * @param minBlobArea The minimum pixel area of each blob. Empirical
 * value for a 720 x 576 size image will be 100; but for a 352 x 288
 * size image,this value is recommended to be about 40.
 * @param connected8Flag Set to 0 for four connected(no diagonal
 * neighbors connected) or to 1 for eight connected(all eight
 * pixel neighbors).
 *
 * @return Unknown.
 */
extern int
VLIB_createConnectedComponentsList(
        VLIB_CCHandle *handle,
        unsigned short width,
        unsigned short rowsInImg,
        int *p32BitPackedFGMask,
        int minBlobArea,
        int connected8Flag);

/**
 * @brief This function is used to get the number of connected
 * components in the list.
 *
 * @param handle Function returns a pointer to the object handle,
 * which is a private structure representing the list of connected
 * components.
 * @param numCCs A integer variable pointer, representing how many
 * connected components in the list.
 *
 * @return Unknown.
 */
extern int
VLIB_getNumCCs(
        VLIB_CCHandle *handle,
        int *numCCs);

/**
 * @brief This function reveal features of the components as defined
 * by the struct VLIB_CC.
 *
 * @param handle Function returns a pointer to the object handle,
 * which is a private structure representing the list of connected
 * components.
 * @param cc A struct VLIB_CC, which will reveals features of the
 * component.
 * @param listIndex Tell the function to return the listIndex item
 * of struct VLIB_CC in the list.
 *
 * @return Unknown.
 */
extern int
VLIB_getCCFeatures(
        VLIB_CCHandle *handle,
        VLIB_CC *cc,
        short listIndex);

int
draw_zoom_rectangle (char *yuv_frame, VLIB_CC *blob,
        float hfactor, float vfactor, int color);

/**
 * @brief A initialization function which executes several init
 * function before applys detect_pedestrian function.
 * In this function, we call the initialization function of DSP VLIB
 * Connected Components Labeling: VLIB_initConnectedComponentsList,
 * which will initial several memory buffer for connected components
 * functions family.
 *
 * @return
 */
int
init_pedestrian_detection ( )
{
    int size_of_handle = VLIB_GetSizeOfCCHandle();
    handle             =
        (VLIB_CCHandle *) MEM_alloc(DDR2HEAP, size_of_handle, 8);

    VLIB_initConnectedComponentsList(
            handle,
            pPrimaryBuf1, INTERNAL_BUF_SIZE,
            pPrimaryBuf2, INTERNAL_BUF_SIZE,
            pOverflowBuf1, EXTERNAL_BUF_SIZE,
            pOverflowBuf2, EXTERNAL_BUF_SIZE);

    return 0;
}		/* -----  end of function init_pedestrian_detection  ----- */


/**
 * @brief Predict whether given ROI is a pedestrian by given hog features.
 * The predict algorithm is SVM predict.
 *
 * @param features The hog features of given ROI.
 *
 * @return 0 for Non-pedestrian; 1 for pedestrian.
 */
int
predict_pedestrian ( float *features, int distance_case )
{
    int is_pedestrian = 0;
    int size = 0;
    struct svm_model model;

    switch (distance_case)
    {
        case NEAR:
            size         = FEATURE_NSIZE + ROI_FEATURES_SIZE;
            model.weight = near_weight;
            model.bias   = near_bias;
            break;
        case MID:
            size         = FEATURE_MSIZE + ROI_FEATURES_SIZE;
            model.weight = mid_weight;
            model.bias   = mid_bias;
            break;
        case FAR:
            size         = FEATURE_FSIZE + ROI_FEATURES_SIZE;
            model.weight = far_weight;
            model.bias   = far_bias;
            break;
        default:
            // error
            break;
    }

    is_pedestrian = predict(&model, features, size);
    return is_pedestrian;
}		/* -----  end of function predict_pedestrian  ----- */


int
append_features ( float *features, int distance_case, VLIB_CC *blob)
{
    float *append_ptr = NULL;

    switch (distance_case)
    {
        case NEAR:
            append_ptr = features + FEATURE_NSIZE;
            break;
        case MID:
            append_ptr = features + FEATURE_MSIZE;
            break;
        case FAR:
            append_ptr = features + FEATURE_FSIZE;
            break;
        default:
            // error
            break;
    }

    *append_ptr++ = divsp_i(intsp_i(blob->xmin), 352.0);
    *append_ptr++ = divsp_i(intsp_i(blob->ymin), 288.0);
    *append_ptr++ = divsp_i(intsp_i(blob->xmax), 352.0);
    *append_ptr++ = divsp_i(intsp_i(blob->ymax), 288.0);

    return 0;
}		/* -----  end of function append_features  ----- */

/*
void getYfromYUV(char *yuv_frame, unsigned char* gray)
{
	int rows,cols;
	int width = DSP_IMG_WIDTH/2;

	unsigned int *pStart = (unsigned int *)yuv_frame;
	unsigned int *ptmp = NULL;
	unsigned int tmp;

	unsigned char *pYs = NULL;

	for(rows = 0; rows < DSP_IMG_HEIGHT; rows++)
	{
		ptmp = pStart + rows*width;
		pYs = gray + rows*DSP_IMG_WIDTH;
		for(cols = 0; cols < DSP_IMG_WIDTH; cols+=2)
		{
			tmp = ptmp[cols>>1];
			pYs[cols] 	= (tmp>>8)&0xff;
			pYs[cols+1] = (tmp>>24)&0xff;
		}
	}
}
*/

void getYfromYUV(char *yuv_frame, unsigned char* gray)
{
	int rows,cols;
	int width = IMG_WIDTH/2;

	unsigned int *pStart = (unsigned int *)yuv_frame;
	unsigned int *ptmp = NULL;
	unsigned int tmp;

	unsigned char *pYs = NULL;

	for(rows = 0; rows < IMG_HEIGHT; rows++)
	{
		ptmp = pStart + rows * width;
		pYs = gray + rows * IMG_WIDTH;
		for(cols = 0; cols < IMG_WIDTH; cols+=2)
		{
			tmp = ptmp[cols>>1];
			pYs[cols] 	= (tmp>>8)&0xff;
			pYs[cols+1] = (tmp>>24)&0xff;
		}
	}
}

/*
void SegByDualthresholdY(unsigned char *pYIn, unsigned char *pYout)
{
	unsigned char Yd;
	unsigned char TL, TH, T1, T2, T3;
	int rows, cols, k;
	unsigned char Iij;
	unsigned char* ptrs=NULL;
	unsigned char* ptrd=NULL;
	int sum = 0;
//	int finishCond = smallWidth - OMEGA;
//	printf("finish : %d\n", finishCond);

	for (rows=0; rows<IMG_HEIGHT; rows++)
	{
		ptrs = pYIn + rows*IMG_WIDTH;
		ptrd = pYout + rows*IMG_WIDTH;

		sum = 0.0;
		for (k = 0; k < 2*SEG_SDT_OMIGA+1; k++)
		{
			sum += ptrs[k];
		}

		for (cols = 0; cols < SEG_SDT_OMIGA + 1; cols++)
			ptrd[cols] = 0;

		for (cols = IMG_WIDTH - SEG_SDT_OMIGA; cols < IMG_WIDTH; cols++)
			ptrd[cols] = 0;

#pragma MUST_ITERATE(4, ,2)
		for (cols=SEG_SDT_OMIGA + 1; cols<IMG_WIDTH - SEG_SDT_OMIGA; cols++)
		{
			sum += ptrs[cols+SEG_SDT_OMIGA];

			sum -= ptrs[cols-SEG_SDT_OMIGA-1];

			TL =  (unsigned char)(sum/25 + SEG_SDT_ALPHA);
			T3 = max((unsigned char)_IQint(_IQmpy(_IQ(1.06), _IQ(TL-SEG_SDT_ALPHA))),TL+SEG_SDT_BETA);
			T2 = ((T3) < (TL+SEG_SDT_GAMMA) ? (T3) : (TL+SEG_SDT_GAMMA));
			T1 = ((T2) < (SEG_SDT_GRAH_H) ? (T2) : (SEG_SDT_GRAH_H));
			TH = ((T1) > (TL) ? (T1) : (TL));


			Iij = ptrs[cols];

			if (Iij > TH)
			{
				Yd = 0xff;
			}
			else if (Iij < TL)
			{
				Yd = 0;
			}
			else if ( ptrd[cols - 1] == 0xff)
			{
				Yd = 0xff;
			}
			else
			{
				Yd = 0;
			}

			ptrd[cols] = Yd;
		}
	}
}


*/

/**
 * @brief This function process a frame of yuv image to detect any pedestrians
 * in this frame, and mark the pedestrians with green edge rectangle.
 *
 * @param yuv_frame A YUV422 video frame in DDR2
 *
 * @return
 */
int
detect_pedestrian (char *origin_frame, char *yuv_frame)
//detect_pedestrian (char *origin_frame)
{
    unsigned short blob_width;
    unsigned short blob_height;
    float ratio, hfactor, vfactor;

    int distance_case = 0;
    unsigned short dw_width;
    unsigned short dw_height;
    unsigned short cell_width;
    unsigned short cell_height;

    unsigned char *gray_img     = gray_image_mem;
//	unsigned char *seg_img      = seg_image_mem;
    unsigned int  *bin_img      = bin_image_mem;
    unsigned char *blob_img     = blob_image_mem;
    unsigned char *std_blob_img = std_blob_image_mem;
    float *features             = features_mem;

    int num_of_connected_components = 0;
	int idx;

    VLIB_CC blob;

	hfactor = HFACTOR;
	vfactor = VFACTOR;
    /**
     * @brief Calling a DSP VLIB function to extract luminance from a
     * YUV422 frame.

     */
//    VLIB_extractLumaFromUYUV(
//            yuv_frame, width, pitch, height, (char *)gray_img);

	getYfromYUV(yuv_frame, gray_img);
//	getYfromYUV(origin_frame, gray_frame);
//	imresize(gray_frame, DSP_IMG_WIDTH, DSP_IMG_HEIGHT, gray_img, IMG_WIDTH, IMG_HEIGHT);

    /**
     * @brief Apply image segmentation operation on a given gray scale
     * image, and then packed the segmentation result as 32-bit int
     * packed binary image.
     */
//	rough_simple_dt_segment(gray_img, bin_img);
//    simple_dt_segment(gray_img, bin_img);
//	rough_gp_dt_segment(gray_img, bin_img);
//    gp_dt_segment(gray_img, bin_img);
	rough_gp_mean_segment(gray_img, bin_img);
//    gp_mean_segment(gray_img, bin_img);
//	SegByDualthresholdY(gray_img, seg_img);
//	pack_binary_image(seg_img, IMG_HEIGHT * BIN_IMG_WIDTH, bin_img);

    /**
     * @brief Calling DSP VLIB Connected Components function to create
     * a connected components list on 32-bit packed binary image based
     * on previous computation.
     * We use value 40 for minimum pixel area of each blob, value 1
     * for eight connected neighbors.
     */

    VLIB_createConnectedComponentsList(
            handle,
            IMG_WIDTH, IMG_HEIGHT,
            (int *)bin_img,
            30,
            1);

    /**
     * @brief Get the number of connected components in the list,
     * which is represented by struct CCHandle variable handle.
     *
     */

    VLIB_getNumCCs(handle, &num_of_connected_components);


//	draw_hline(yuv_frame, 0, IMG_HEIGHT/2, IMG_WIDTH, GREEN);
//	draw_vline(yuv_frame, IMG_WIDTH/2, 0, IMG_HEIGHT, GREEN);

    /**
     * @brief Traveling through all the connected components on the
     * list for given image.
     */
	 //num_of_connected_components
    for (idx = 0; idx < num_of_connected_components; ++idx)
    {
        /**
         * @brief Extract the blob information about the idx-th
         * connected components.
         * */

        VLIB_getCCFeatures(handle, &blob, idx);

        // minimum blob width >= 8, minimum blob height >= 11

        blob_width  = blob.xmax - blob.xmin;
        blob_height = blob.ymax - blob.ymin;
        if (blob_width < 8 || blob_height < 11)
            continue;
        // the ratio between blob width and blob height should be in [0.25, 0.75]
        ratio = divsp_i(intsp_i(blob_width), intsp_i(blob_height));
        if (ratio > 0.85F || ratio < 0.15F)
            continue;
        // maybe, the min y should be >= 0.3 * height ~ 86
//        if (blob.ymin < 86)
//            continue;
        if (blob_height < (DW_FHEIGHT + DW_MHEIGHT) / 2)
            distance_case = FAR;
        else if (blob_height < (DW_MHEIGHT + DW_NHEIGHT) / 2)
            distance_case = MID;
        else
            distance_case = NEAR;

        draw_zoom_rectangle(origin_frame, &blob, hfactor, vfactor, RED);
//        draw_rectangle(origin_frame, 2 * blob.xmin, 2 * blob.ymin, 2 * blob.xmax, 2 * blob.ymax, RED);

        // only mid distance temporary...
        if (distance_case != MID)   continue;

        switch (distance_case)
        {
            case NEAR:
                dw_width    = DW_NWIDTH;
                dw_height   = DW_NHEIGHT;
                cell_width  = CELL_NWIDTH;
                cell_height = CELL_NHEIGHT;
                break;
            case MID:
                dw_width    = DW_MWIDTH;
                dw_height   = DW_MHEIGHT;
                cell_width  = CELL_MWIDTH;
                cell_height = CELL_MHEIGHT;
                break;
            case FAR:
                dw_width    = DW_FWIDTH;
                dw_height   = DW_FHEIGHT;
                cell_width  = CELL_FWIDTH;
                cell_height = CELL_FHEIGHT;
                break;
            default:
                // error
                break;
        }

        /**
         * @brief Extract the blob image.
         */

        extract_blob(gray_img, IMG_WIDTH,
                blob.xmin, blob.ymin, blob.xmax, blob.ymax,
                blob_img);


        /**
         * @brief Resize the blob image to standard detected window size.
         */

        imresize(blob_img, blob.xmax-blob.xmin+1, blob.ymax-blob.ymin+1,
               std_blob_img, dw_width, dw_height);

        /**
         * @brief Compute the hog features based on ROI image information.
         */

        get_r_lahog_features(std_blob_img,
                dw_width, dw_height, cell_width, cell_height,
                (char *)&hog_buffer_mem, features);
//        get_hog_features(std_blob_img,
//              dw_width, dw_height, cell_width, cell_height,
//              (char *)&hog_buffer_mem, features);

        append_features (features, distance_case, &blob);
        /**
         * @brief Predict whether current ROI is a pedestrian.
         */
        if (predict_pedestrian(features, distance_case))
        {
            /**
             * Drawing a rectangle based on the origina blob information on
             * the YUV video frame.
             */
            draw_zoom_rectangle(origin_frame, &blob, hfactor, vfactor, GREEN);
//            draw_rectangle(origin_frame, 2 * blob.xmin, 2 * blob.ymin, 2 * blob.xmax, 2 * blob.ymax, GREEN);
        }
    }

    return 0;
}		/* -----  end of function detect_pedestrian  ----- */



