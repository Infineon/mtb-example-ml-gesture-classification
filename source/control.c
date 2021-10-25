/******************************************************************************
* File Name:   control.c
*
* Description: This file contains the implementation to read a result from the
*                 inference engine.
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
#include <gesture.h>
#include "control.h"
#include "mtb_ml_utils.h"

#include <stdio.h>
#include <stdlib.h>

/*******************************************************************************
* Constants
*******************************************************************************/
#define MIN_CONFIDENCE 0.97

/*******************************************************************************
* Function Name: control
********************************************************************************
* Summary:
*   A function used to read a result from the inference engine, this prints the
*   the class and the confidence of each class.
*
* Parameters:
*     result_buffer: The result buffer from the inference engine
*     model_output_size: Output size from the inference engine
*
*
*******************************************************************************/
void control(MTB_ML_DATA_T* result_buffer, int model_output_size)
{
    /* Get the q-format from the model */
#if !COMPONENT_ML_FLOAT32
    uint8_t q_format = mtb_ml_model_get_output_q_fraction_bits(magic_wand_obj);
#endif

    int class_index = mtb_ml_utils_find_max(result_buffer, model_output_size);

#if CY_ML_FIXED_POINT_16_IN
    /* Convert 16bit fixed-point output to floating-point for visualization */
    float *nn_float_buffer = (float *) malloc(model_output_size * sizeof(float));
    mtb_ml_utils_convert_int16_to_flt(result_buffer, nn_float_buffer, model_output_size, q_format);
#elif CY_ML_FIXED_POINT_8_IN
    /* Convert 8bit fixed-point output to floating-point for visualization */
    float *nn_float_buffer = (float *) malloc(model_output_size * sizeof(float));
    mtb_ml_utils_convert_int8_to_flt(result_buffer, nn_float_buffer, model_output_size, q_format);
#else
    float *nn_float_buffer = result_buffer;
#endif

    /* Clear the screen */
    printf("\x1b[2J\x1b[;H");

    printf("| Gesture     | Square | Circle | Side-To-Side |  None  | |  Detection  |\r\n");
    printf("|-------------|--------|--------|--------------|--------| |-------------|\r\n");

    /* Prints the confidence level of each class */
    printf("| Confidence  |  %%%-3d      %%%-3d       %%%-3d        %%%-3d       ", (int)(nn_float_buffer[0]*100), (int)(nn_float_buffer[1]*100), (int)(nn_float_buffer[2]*100), (int)(nn_float_buffer[3]*100));

    /* Check the confidence for the selected class */
    if(MIN_CONFIDENCE < nn_float_buffer[class_index])
    {
        /* Switch statement for the selected class */
        switch (class_index)
        {
            case 0:
                printf("Square\r\n");
                break;
            case 1:
                printf("Circle\r\n");
                break;
            case 2:
                printf("Side-To-Side\r\n");
                break;
            case 3:
                printf("None\r\n");
                break;
        }
    }
    /* If the confidence is not high, no gesture detected */
    else
    {
        printf("None\r\n");
    }
}
