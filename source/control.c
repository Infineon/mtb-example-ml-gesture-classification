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
#include "control.h"

#include "nn_utils.h"
#include "nn.h"
#include "sensor.h"

#include <stdio.h>
#include <stdlib.h>

/*******************************************************************************
* Constants
*******************************************************************************/
#define MIN_CONFIDENCE 0.7

/*******************************************************************************
* Function Name: control_task
********************************************************************************
* Summary:
*   A task used to read a result from the inference engine, this prints the
*   the class and the confidence of each class.
*
* Parameters:
*     arg: not used
*
*
*******************************************************************************/
void control_task( void * arg )
{
    (void)arg;

    NN_OUT_Type  *nn_result_buffer;

    /* Allocate memory for the output buffers */
    nn_result_buffer = (NN_OUT_Type *) malloc(nn_gesture_obj.model_xx_info.n_out_classes * sizeof(NN_OUT_Type));

#if CY_ML_FIXED_POINT
    float *nn_float_buffer = (float *) malloc(nn_gesture_obj.model_xx_info.n_out_classes * sizeof(float));
#else
    float *nn_float_buffer;
#endif


    for(;;)
    {
        /* Get the result from the model */
        nn_result(&nn_gesture_obj, nn_result_buffer);

#if CY_ML_FIXED_POINT_16_IN
        /* Convert 16bit fixed-point output to floating-point for visualization */
        nn_utils_convert_int16_to_flt(nn_result_buffer,
                                      nn_float_buffer,
                                      nn_gesture_obj.model_xx_info.n_out_classes,
                                      nn_gesture_obj.in_out_fixed_point_q);
#elif CY_ML_FIXED_POINT_8_IN
        /* Convert 8bit fixed-point output to floating-point for visualization */
        nn_utils_convert_int8_to_flt(nn_result_buffer,
                                              nn_float_buffer,
                                              nn_gesture_obj.model_xx_info.n_out_classes,
                                              nn_gesture_obj.in_out_fixed_point_q);
#else
        nn_float_buffer = nn_result_buffer;
#endif

        /* Find the class with the highest confidence */
        uint8_t class_index = nn_utils_find_max_index_flt(nn_float_buffer, nn_gesture_obj.model_xx_info.n_out_classes);

        /* Clear the screen */
        printf("\x1b[2J\x1b[;H");

        printf("| Gesture     | Square | Circle | Side-To-Side |  None  | |  Detection  |\r\n");
        printf("|-------------|--------|--------|--------------|--------| |-------------|\r\n");

        /* Prints the confidence level of each class */
        printf("| Probability |  %%%-3d      %%%-3d       %%%-3d        %%%-3d       ", (int)(nn_float_buffer[0]*100), (int)(nn_float_buffer[1]*100), (int)(nn_float_buffer[2]*100), (int)(nn_float_buffer[3]*100));

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
}
