/******************************************************************************
* File Name:   nn.c
*
* Description: This file contains the implementation of the neural network.
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
#include "nn.h"
#include "cy_device_headers.h"

#include <stdio.h>
#include <stdlib.h>

/*******************************************************************************
* Constants
*******************************************************************************/
#define NN_QUEUE_LENGTH                 1u
#define NN_THREAD_STACK_SIZE            (4*1024u)
#define NN_QUEUE_TIMEOUT                100u

/*******************************************************************************
* Global Variables
*******************************************************************************/

/*******************************************************************************
* Local Functions
*******************************************************************************/
void nn_task(void *arg);

/*******************************************************************************
* Function Name: nn_init
********************************************************************************
* Summary:
*   Allocate internal memory for the NN. Initialize the Inference Engine. 
*   Create the NN task.
*
* Parameters:
*   nn: model info ouput
*   model_param: pointer to the model parameters
*   model_data: pointer to the model data
*
* Return:
*   The status of the initialization.
*******************************************************************************/
cy_rslt_t nn_init(nn_obj_t *nn, void *model_param, void *model_data)
{
    cy_rslt_t result;
    uint32_t item_in_size;
    uint32_t item_out_size;

    /* Extract required memory for model inference */
    result = Cy_ML_Model_Parse(model_param, &nn->model_xx_info);

    /* Allocate memory */
    if (result == CY_RSLT_SUCCESS)    /* Model Parsing successful */
    {
        nn->persisten_mem = (char *) malloc(nn->model_xx_info.persistent_mem * sizeof(char));
        if (nn->persisten_mem == NULL)
        {
            return NN_RESULT_ALLOC_ERR;
        }
        nn->scratch_mem = (char *) malloc(nn->model_xx_info.scratch_mem * sizeof(char));
        if (nn->scratch_mem == NULL)
        {
            return NN_RESULT_ALLOC_ERR;
        }
        nn->input_buffer = (char *) malloc(nn->model_xx_info.input_sz * sizeof(NN_IN_Type));
        if (nn->input_buffer == NULL)
        {
            return NN_RESULT_ALLOC_ERR;
        }
        nn->output_buffer = (char *) malloc(nn->model_xx_info.n_out_classes * sizeof(NN_OUT_Type));
        if (nn->output_buffer == NULL)
        {
            return NN_RESULT_ALLOC_ERR;
        }
    }
    else
    {
        return NN_RESULT_BAD_MODEL;
    }
    
    /* Initialize Model and get Model Container/object */
    result = Cy_ML_Model_Init(&nn->model_xx_obj, (char *) model_param, (char *) model_data,
                               nn->persisten_mem, nn->scratch_mem, &nn->model_xx_info, 1, 100);

    if (result != CY_RSLT_SUCCESS)
    {
        return NN_RESULT_ERROR;
    }

    item_in_size  = nn->model_xx_info.input_sz * sizeof(NN_IN_Type);
    item_out_size = nn->model_xx_info.n_out_classes * sizeof(NN_OUT_Type);

    /* Initialize internal queues */
    result = cy_rtos_init_queue(&nn->nn_input_queue, NN_QUEUE_LENGTH, item_in_size);
    if (result != CY_RSLT_SUCCESS)
    {
        return NN_RESULT_ERROR;
    }
    result = cy_rtos_init_queue(&nn->nn_output_queue, NN_QUEUE_LENGTH, item_out_size);
    if (result != CY_RSLT_SUCCESS)
    {
        /* DeInit the previous queue */
        cy_rtos_deinit_queue(&nn->nn_input_queue);
        return NN_RESULT_ERROR;
    }

    /* Create the RTOS task */
    result = cy_rtos_create_thread(&nn->nn_thread, nn_task, "NN_Task", NULL,
                    NN_THREAD_STACK_SIZE, CY_RTOS_PRIORITY_NORMAL, nn);
    if (result != CY_RSLT_SUCCESS)
    {
        /* DeInit the queues */
        cy_rtos_deinit_queue(&nn->nn_input_queue);
        cy_rtos_deinit_queue(&nn->nn_output_queue);
        return NN_RESULT_ERROR;
    }


    return CY_RSLT_SUCCESS;
}

/*******************************************************************************
* Function Name: nn_feed
********************************************************************************
* Summary:
*   Feed data to the neural network
*
* Parameters:
*   nn: neural network
*   input: input data
*
* Return:
*   The status of the initialization.
*******************************************************************************/
cy_rslt_t nn_feed(nn_obj_t *nn, void *input)
{
    cy_rslt_t result;
    bool in_isr = (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0;

    if (input == NULL || nn == NULL)
    {
        return NN_RESULT_BAD_ARG;
    }

    /* Send the input data to the queue */
    result = cy_rtos_put_queue(&nn->nn_input_queue, input, CY_RTOS_NEVER_TIMEOUT, in_isr);
    if (result != CY_RSLT_SUCCESS)
    {
        return NN_RESULT_ERROR;
    }

    return CY_RSLT_SUCCESS;
}

/*******************************************************************************
* Function Name: nn_result
********************************************************************************
* Summary:
*   Read the result of the neural network inference engine.
*
* Parameters:
*   nn: neural network
*   input: input data
*
* Return:
*   The status of the initialization.
*******************************************************************************/
cy_rslt_t nn_result(nn_obj_t *nn, void *output)
{
    cy_rslt_t result;
    bool in_isr = (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0;

    if (output == NULL || nn == NULL)
    {
        return NN_RESULT_BAD_ARG;
    }

    /* Read data from the queue */
    result = cy_rtos_get_queue(&nn->nn_output_queue, output, CY_RTOS_NEVER_TIMEOUT, in_isr);

    if (result != CY_RSLT_SUCCESS)
    {
        return NN_RESULT_ERROR;
    }

    return CY_RSLT_SUCCESS;
}

/*******************************************************************************
* Function Name: nn_task
********************************************************************************
* Summary:
*   NN task to process the neural network.
*
* Parameters:
*   arg: not used
*
* Return:
*   The status of the initialization.
*******************************************************************************/
void nn_task(void *arg)
{
    nn_obj_t *nn = (nn_obj_t *) arg;

#ifdef ML_PROFILER
    Cy_ML_Profile_Init();
#endif

    while (1)
    {
        /* Wait till the input queue is fill with data */
        cy_rtos_get_queue(&nn->nn_input_queue, nn->input_buffer, CY_RTOS_NEVER_TIMEOUT, false);

#ifdef ML_PROFILER
        Cy_ML_Profile_Start();
#endif

#if CY_ML_FIXED_POINT
        nn->in_out_fixed_point_q = nn->fixed_point_q;
#endif
        /* Process the inference engine */
        Cy_ML_Model_Inference(nn->model_xx_obj, 
                              nn->input_buffer, 
                              nn->output_buffer, 
#if CY_ML_FIXED_POINT
                             &nn->in_out_fixed_point_q, 
                              nn->model_xx_info.input_sz);
#else
                              NULL, 0);
#endif

#ifdef ML_PROFILER
        Cy_ML_Profile_Update();
#endif

        /* Send out result to the output queue */
        cy_rtos_put_queue(&nn->nn_output_queue, nn->output_buffer, CY_RTOS_NEVER_TIMEOUT, false);
    }
}

