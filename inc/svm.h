/**
 * =====================================================================================
 *
 *          @file  svm.h
 *         @brief  
 *
 *        @author  dengos (W), dengos.w@gmail.com, 
 *       @version  1.0
 *          @date  09/02/2012 04:43:23 PM
 *
 * =====================================================================================
 */


#ifndef  svm_INC
#define  svm_INC



struct svm_model 
{
    float *weight;
    float  bias;
};				
/* ----------  end of struct svm_model  ---------- */


int
predict ( struct svm_model *model, float *features, int size );



#endif   /* ----- #ifndef svm_INC  ----- */


