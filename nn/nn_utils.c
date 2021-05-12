/******************************************************************************
* File Name:   nn_utils.c
*
* Description: This file contains the implementation of some support functions
*  for the neural network profiler.
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
#include "nn_utils.h"

/*******************************************************************************
* Function Name: nn_utils_find_max_index_flt
********************************************************************************
* Summary:
*   Return the highest value index in the input array
*
* Parameters:
*   in: input array in floating-point
*   len: array size
*
* Return:
*   Index of the maximum value
*******************************************************************************/
int nn_utils_find_max_index_flt(float *in, int len)
{
    float cur_val;
    float max_val;
    int   out_idx;
    int   blk_cnt;

    /* Initialize index value to zero */
    out_idx = 0;

    /* Load first input value as the max value from now */
    max_val = *in++;

    /* Initialize the number of samples */
    blk_cnt = (len -1);

    while (blk_cnt > 0)
    {
        /* Set the max val to the next value in the input array */
        cur_val = *in++;

        /* Compare against the max value */
        if (max_val < cur_val)
        {
            /* Update the max value and the output index */
            max_val = cur_val;
            out_idx = len - blk_cnt;
        }

        /* Decrement the block counter */
        blk_cnt--;
    }

    return out_idx;
}

/*******************************************************************************
* Function Name: nn_utils_find_max_index_int
********************************************************************************
* Summary:
*   Return the highest value index in the input array
*
* Parameters:
*   in: input array in fixed-point
*   len: array size
*
* Return:
*   Index of the maximum value
*******************************************************************************/
int nn_utils_find_max_index_int(int *in, int len)
{
    int cur_val;
    int max_val;
    int out_idx;
    int blk_cnt;

    /* Initialize index value to zero */
    out_idx = 0;

    /* Load first input value as the max value from now */
    max_val = *in++;

    /* Initialize the number of samples */
    blk_cnt = (len -1);

    while (blk_cnt > 0)
    {
        /* Set the max val to the next value in the input array */
        cur_val = *in++;

        /* Compare against the max value */
        if (max_val < cur_val)
        {
            /* Update the max value and the output index */
            max_val = cur_val;
            out_idx = len - blk_cnt;
        }

        /* Decrement the block counter */
        blk_cnt--;
    }

    return out_idx;
}

/*******************************************************************************
* Function Name: nn_utils_convert_int16_to_flt
********************************************************************************
* Summary:
*   Convert an fixed-point (integer) value to floating-point based on the 
*   q_norm.
*
* Parameters:
*   in: input array in fixed-point
*   out: output array in floating-point
*   len: array size
*   q: Q value for the input array
*
*******************************************************************************/
void nn_utils_convert_int16_to_flt(int16_t *in, float *out, int len, int q)
{
    int blk_cnt;

    /* Calculate the floating Q normalization value */
    float q_norm = 1.0f / (float) (1 << q);
    
    blk_cnt = len;

    while (blk_cnt > 0)
    {
        *out++ = ((float) (*in++)) * q_norm;
        blk_cnt--;
    }
}

/*******************************************************************************
* Function Name: nn_utils_convert_int8_to_flt
********************************************************************************
* Summary:
*   Convert an fixed-point (integer) value to floating-point based on the
*   q_norm.
*
* Parameters:
*   in: input array in fixed-point
*   out: output array in floating-point
*   len: array size
*   q: Q value for the input array
*
*******************************************************************************/
void nn_utils_convert_int8_to_flt(int8_t *in, float *out, int len, int q)
{
    int blk_cnt;

    /* Calculate the floating Q normalization value */
    float q_norm = 1.0f / (float) (1 << q);

    blk_cnt = len;

    while (blk_cnt > 0)
    {
        *out++ = ((float) (*in++)) * q_norm;
        blk_cnt--;
    }
}

/*******************************************************************************
* Function Name: nn_utils_convert_flt_to_int16
********************************************************************************
* Summary:
*   Convert a floating point value to a fixed-point(integer) based on the
*   q_norm.
*
* Parameters:
*   in: input array in floating-point
*   out: output array in fixed-point
*   len: array size
*   q: Q value for the input array
*
*******************************************************************************/
void nn_utils_convert_flt_to_int16(float *in, int16_t *out, int len, float q)
{
    int blk_cnt = 0;

    /* Convert to floating-point int16 */
    while(len > blk_cnt)
    {
        if(0 <= in[blk_cnt])
        {
            *out++ = (int16_t)(*in++ * q + 0.5);
        }
        else
        {
            *out++ = (int16_t)(*in++ * q - 0.5);
        }
        blk_cnt++;
    }
}

/*******************************************************************************
* Function Name: nn_utils_convert_flt_to_int8
********************************************************************************
* Summary:
*   Convert a floating point value to a fixed-point(integer) based on the
*   q_norm.
*
* Parameters:
*   in: input array in floating-point
*   out: output array in fixed-point
*   len: array size
*   q: Q value for the input array
*
*******************************************************************************/
void nn_utils_convert_flt_to_int8(float *in, int8_t *out, int len, float q)
{
    int blk_cnt = 0;

    /* Convert to floating-point int16 */
    while(len > blk_cnt)
    {
        if(0 <= in[blk_cnt])
        {
            *out++ = (int8_t)(*in++ * q + 0.5);
        }
        else
        {
            *out++ = (int8_t)(*in++ * q - 0.5);
        }
        blk_cnt++;
    }
}
