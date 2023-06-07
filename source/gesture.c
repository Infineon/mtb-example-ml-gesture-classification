/******************************************************************************
* File Name:   gesture.c
*
* Description: This file contains the implementation to read data from the IMU
*                 and process it in chunks of 128 samples. The data is then fed
*                 to the neural network.
*
* Related Document: See README.md
*
*
*******************************************************************************
* Copyright 2021-2023, Cypress Semiconductor Corporation (an Infineon company) or
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
#include "gesture.h"

#include "processing.h"
#include "control.h"
#include "sensor.h"

#include "cyhal.h"
#include "cybsp.h"
#include "mtb_ml_utils.h"
#include "mtb_ml_common.h"

#include <stdlib.h>

/* Include model files */
#include MTB_ML_INCLUDE_MODEL_FILE(MODEL_NAME)

/*******************************************************************************
* Constants
*******************************************************************************/
#define MAX_DATA_SAMPLE 32768
#define MIN_DATA_SAMPLE -32768

#if (COMPONENT_ML_INT16x16 || COMPONENT_ML_INT16x8)
    #define QFORMAT_VALUE    15
#endif
#if (COMPONENT_ML_INT8x8)
    #define QFORMAT_VALUE    7
#endif
/*******************************************************************************
* Global Variables
*******************************************************************************/
/* Model information */
mtb_ml_model_t *magic_wand_obj;

/* Output/result buffers for the inference engine */
static MTB_ML_DATA_T *result_buffer;

/* Model Output Size */
static int model_output_size;

/*******************************************************************************
* Function Name: gesture_init
********************************************************************************
* Summary:
*   Initialize the Neural Network based on the magic wand model. Initializes the
*   IMU sensor to collect data.
*
* Parameters:
*     None
*
* Return:
*   The status of the initialization.
*******************************************************************************/
cy_rslt_t gesture_init(void)
{
    cy_rslt_t result;

    mtb_ml_model_bin_t magic_wand_bin = {MTB_ML_MODEL_BIN_DATA(MODEL_NAME)};

    /* Initialize the Neural Network */
    result = mtb_ml_model_init(&magic_wand_bin, NULL, &magic_wand_obj);
    if(CY_RSLT_SUCCESS != result)
    {
        return result;
    }

#if !COMPONENT_ML_FLOAT32
    /* Set the q-factor */
    mtb_ml_model_set_input_q_fraction_bits(magic_wand_obj, QFORMAT_VALUE);
#endif

    mtb_ml_model_get_output(magic_wand_obj, &result_buffer, &model_output_size);

    /* Initialize the IMU sensor */
    result = sensor_init();

    return result;
}

/*******************************************************************************
* Function Name: gesture_task
********************************************************************************
* Summary:
*   A task used to pre-process data before it is fed to the inference engine.
*
* Parameters:
*     arg: not used
*
*
*******************************************************************************/
void gesture_task(void *arg)
{
#if !GESTURE_DATA_COLLECTION_MODE
    /* Regression pointers */
    MTB_ML_DATA_T *input_reference;

#endif

    /* Data processed in floating point */
    float data_feed[SENSOR_BATCH_SIZE][SENSOR_NUM_AXIS];

    (void)arg;

    /* Initialize the butter-worth filter variables */
    int n_order = 3;
    /* Coefficients for 3rd order butter-worth filter */
    const float coeff_b[] = IIR_FILTER_BUTTER_WORTH_COEFF_B;
    const float coeff_a[] = IIR_FILTER_BUTTER_WORTH_COEFF_A;
    iir_filter_struct butter_lp_fil;

    for(;;)
    {
        uint16_t cur = 0;
        int16_t temp_buffer[SENSOR_BATCH_SIZE][SENSOR_NUM_AXIS];

        /* Get sensor data */
        sensor_get_data((void *) temp_buffer);

        /* Cast the data from an int16 to a float for pre-processing */
        cast_int16_to_float(&temp_buffer[0][0], &data_feed[0][0], SENSOR_BATCH_SIZE*SENSOR_NUM_AXIS);

        /* Third order butter-worth filter */
        while(cur < SENSOR_NUM_AXIS)
        {
            /* Initialize and run the filter */
            iir_filter_init(&butter_lp_fil, coeff_b, coeff_a, n_order);
            iir_filter(&butter_lp_fil, &data_feed[0][0], SENSOR_BATCH_SIZE, cur, SENSOR_NUM_AXIS);
            cur++;
        }

        /* A min max normalization to get all data between -1 and 1 */
        normalization_min_max(&data_feed[0][0], SENSOR_BATCH_SIZE, SENSOR_NUM_AXIS, MIN_DATA_SAMPLE, MAX_DATA_SAMPLE);

#ifdef CY_BMI_160_IMU_I2C
        /* Swap axis for BMI_160 so board orientation stays the same */
        column_inverse(&data_feed[0][0], SENSOR_BATCH_SIZE, SENSOR_NUM_AXIS, 2);
        column_swap(&data_feed[0][0], SENSOR_BATCH_SIZE, SENSOR_NUM_AXIS, 0, 1);
        column_inverse(&data_feed[0][0], SENSOR_BATCH_SIZE, SENSOR_NUM_AXIS, 5);
        column_swap(&data_feed[0][0], SENSOR_BATCH_SIZE, SENSOR_NUM_AXIS, 3, 4);
#endif

#if GESTURE_DATA_COLLECTION_MODE
    cur = 0;
    printf("-,-,-,-,-,-\r\n");
    while (cur < SENSOR_BATCH_SIZE)
    {
        printf("%6f,%6f,%6f,%6f,%6f,%6f\r\n", data_feed[cur][0],
                                              data_feed[cur][1],
                                              data_feed[cur][2],
                                              data_feed[cur][3],
                                              data_feed[cur][4],
                                              data_feed[cur][5]);
        cur++;
    }
#else

#if !COMPONENT_ML_FLOAT32
        /* Quantize data before feeding model */
        MTB_ML_DATA_T data_feed_int[SENSOR_BATCH_SIZE][SENSOR_NUM_AXIS];
        mtb_ml_utils_model_quantize(magic_wand_obj, &data_feed[0][0], &data_feed_int[0][0]);

        /* Feed the Model */
        input_reference = (MTB_ML_DATA_T *) data_feed_int;
        mtb_ml_model_run(magic_wand_obj, input_reference);
        control(result_buffer, model_output_size);

#else
        input_reference = (MTB_ML_DATA_T *) data_feed;
        mtb_ml_model_run(magic_wand_obj, input_reference);
        control(result_buffer, model_output_size);
#endif

#endif /* #if GESTURE_DATA_COLLECTION */
    }
}
