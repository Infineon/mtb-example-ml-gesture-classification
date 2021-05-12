/******************************************************************************
* File Name:   nn.h
*
* Description: This file contains the function prototypes and constants used
*   in nn.c.
*
* Related Document: See README.md
*
*
*******************************************************************************
* Copyright 2021, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*******************************************************************************/
#ifndef NN_H
#define NN_H

#include "cy_result.h"

#include "cyabs_rtos.h"

#include "cy_ml_inference.h"

/******************************************************************************
 * Macros
 *****************************************************************************/
#define EXPANDSTR(x) x
#define STRINGIFY(x) #x
#define EXPAND_AND_STRINGIFY(x) STRINGIFY(x)
#define CONCAT(X,Y) X##Y

#define INCLUDE_FILE(x,y)    EXPAND_AND_STRINGIFY(CONCAT(x,y))
#define MODEL_DATA(x,y)      EXPANDSTR(CONCAT(x,y))
#define MODEL_DATA_FILE(x,y) EXPAND_AND_STRINGIFY(CONCAT(x,y))

/******************************************************************************
 * Defines
 *****************************************************************************/
#define MODEL_FILE_EXT     _model_all.h

#define MODEL_PARAM_EXT    _model_prms_bin

#if defined(COMPONENT_ML_FLOAT32) 
    #define X_DATA_FILE_EXT     _x_data_flt.h
    #define Y_DATA_FILE_EXT     _y_data_flt.h
#else
    #define X_DATA_FILE_EXT     _x_data_fixed.h
    #define Y_DATA_FILE_EXT     _y_data_fixed.h
#endif

#if defined(COMPONENT_ML_FLOAT32) 
    #define X_DATA_PTR_EXT      _x_data_flt_bin
    #define Y_DATA_PTR_EXT      _y_data_flt_bin
    #define MODEL_PTR_EXT       _model_flt_bin
#endif
#if defined(COMPONENT_ML_INT16x8) 
    #define X_DATA_PTR_EXT      _x_data_fixed16_bin
    #define Y_DATA_PTR_EXT      _y_data_fixed_bin
    #define MODEL_PTR_EXT       _model_fixed8_bin
#endif
#if defined(COMPONENT_ML_INT16x16)
    #define X_DATA_PTR_EXT      _x_data_fixed16_bin
    #define Y_DATA_PTR_EXT      _y_data_fixed_bin
    #define MODEL_PTR_EXT       _model_fixed16_bin
#endif
#if defined(COMPONENT_ML_INT8x16)
    #define X_DATA_PTR_EXT      _x_data_fixed8_bin
    #define Y_DATA_PTR_EXT      _y_data_fixed_bin
    #define MODEL_PTR_EXT       _model_fixed16_bin
#endif
#if defined(COMPONENT_ML_INT8x8)
    #define X_DATA_PTR_EXT      _x_data_fixed8_bin
    #define Y_DATA_PTR_EXT      _y_data_fixed_bin
    #define MODEL_PTR_EXT       _model_fixed8_bin
#endif

/******************************************************************************
 * Typedefs
 *****************************************************************************/
#if defined(COMPONENT_ML_FLOAT32) 
    typedef float               NN_IN_Type;
    typedef float               NN_OUT_Type;
    typedef float               NN_WGT_Type;
#endif
#if defined(COMPONENT_ML_INT16x8) 
    typedef int16_t             NN_IN_Type;
    typedef int16_t             NN_OUT_Type;
    typedef int8_t              NN_WGT_Type;
#endif
#if defined(COMPONENT_ML_INT16x16)
    typedef int16_t             NN_IN_Type;
    typedef int16_t             NN_OUT_Type;
    typedef int16_t             NN_WGT_Type;
#endif
#if defined(COMPONENT_ML_INT8x16)
    typedef int8_t              NN_IN_Type;
    typedef int8_t              NN_OUT_Type;
    typedef int16_t             NN_WGT_Type;
#endif
#if defined(COMPONENT_ML_INT8x8)
    typedef int8_t              NN_IN_Type;
    typedef int8_t              NN_OUT_Type;
    typedef int8_t              NN_WGT_Type;
#endif

typedef struct 
{
    void *model_xx_obj;
    cy_stc_ml_model_info_t model_xx_info;
    char *persisten_mem;
    char *scratch_mem;
    char *input_buffer;
    char *output_buffer;
    int fixed_point_q;
    int in_out_fixed_point_q;
    cy_queue_t  nn_input_queue;
    cy_queue_t  nn_output_queue;
    cy_thread_t nn_thread;
} nn_obj_t;

/*******************************************************************************
* Constants
*******************************************************************************/
#define NN_RESULT_ERROR         1u
#define NN_RESULT_TIMEOUT       2u
#define NN_RESULT_BAD_ARG       3u
#define NN_RESULT_ALLOC_ERR     4u
#define NN_RESULT_BAD_MODEL     5u

/*******************************************************************************
* Functions
*******************************************************************************/
cy_rslt_t nn_init(nn_obj_t *nn, void *model_param, void *model_data);
cy_rslt_t nn_feed(nn_obj_t *nn, void *input);
cy_rslt_t nn_result(nn_obj_t *nn, void *output);

#endif /* CONSOLE_H */

/* [] END OF FILE */
