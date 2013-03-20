/**
 * =====================================================================================
 *
 *       @file     svm.c
 *       @brief    A file contain Support Vector Machine algorithms.
 *
 *       @author   dengos (W), dengos.w@gmail.com, 
 *       @version  1.0
 *       @date     09/02/2012 04:36:58 PM
 *
 * =====================================================================================
 */
#include "parameters.h"
//#include "fastrts_i.h"
#include "svm.h"
static float svm_result_my = 0.0F;
static int svm_features_size = 0;

/**
 * @brief A SVM predict function, which only work with one
 * feature set a time.
 *
 * @param model A pointer of struct svm_model, which provide
 * parameters for svm predict function.
 * @param features A feature set of one sample.
 * @param size The size of given features.
 *
 * @return 1 for predict true, 0 for false.
 */
int
predict ( struct svm_model *model, float *features, int feature_size )
{
    int idx;
    int is_true = 0;
    float accumulation = 0.0F;

    float *weight  = NULL;
    float *feature = NULL;

    weight  = model->weight;
    feature = features;
    for (idx = 0; idx < feature_size; idx++)
    {
        accumulation = addsp_i(accumulation, mpysp_i(weight[idx], feature[idx]));
//        weight++;
//        feature++;
    }

    accumulation = addsp_i(accumulation, model->bias);
	svm_result_my = accumulation;
	svm_features_size = feature_size;
	if (accumulation > 1e-6)
		return 1;
	else
		return 0;
 //   is_true = accumulation > 1e-6 ? 1: 0;
//    return is_true;
}		/* -----  end of function predict  ----- */



