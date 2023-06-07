/******************************************************************************
* File Name:   processing.c
*
* Description: This file contains implementation for processing data before
*              passing it to the inference engine.
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
#include "processing.h"

/******************************************************************************
 * Defines
 *****************************************************************************/
#define MAX_NORMALIZATION_VALUE  1
#define MIN_NORMALIZATION_VALUE -1


/*******************************************************************************
* Function Name: iir_filter_init
********************************************************************************
* Summary:
*   Initialize the X and Y states for a butter-worth filter.
*
* Parameters:
*   st: Structure for filter data
*   a: A coefficients for filtration
*   b: B coefficients for filtration
*   n_order: The filter order
*
* Return:
*   The status of the initialization.
*******************************************************************************/
int iir_filter_init(iir_filter_struct* st, const float* b, const float* a, int n_order)
{
    /* Store passed in values in the struct */
    st->b = b;
    st->a = a;
    st->n_order = n_order;

    /* Check the number of states */
    if (n_order >= MAX_STATES)
    {
        printf("ERROR: exceeded max states!!\n");
        return -1;
    }

    /* Prepare the X and Y states */
    for (int i = 0; i < n_order+1; i++)
    {
        st->x_states[i] = 0.0;
        st->y_states[i] = 0.0;
    }
    return 0;
}

/*******************************************************************************
* Function Name: iir_filter
********************************************************************************
* Summary:
*   Implements a butter-worth filter on the passed in data
*
* Parameters:
*   st: Structure for filter data
*   buf: pointer the stored data
*   length: The total number values
*   cur_dimension: Current dimension of the 2D array
*   total_dimension: Total dimensions of the 2D array
*
* Return:
*   The status of the initialization.
*******************************************************************************/
void iir_filter(iir_filter_struct *st, float *buf, int length, uint16_t cur_dimension, uint16_t total_dimensions)
{
    /* Store the previous states */
    float* x_states = st->x_states;
    float* y_states = st->y_states;
    const float* b = st->b;
    const float* a = st->a;
    int n_order = st->n_order;

    /* Filter the data based on the passed in length */
    for (int i = 0; i < length; i++)
    {
        x_states[0] = buf[total_dimensions*i + cur_dimension];
        buf[total_dimensions*i + cur_dimension] = b[0] * x_states[0];
        for (int j = 1; j < n_order + 1; j++)
        {
            /* Store filtered values in original buffer */
            buf[total_dimensions*i + cur_dimension] += b[j] * x_states[j];
            buf[total_dimensions*i + cur_dimension] -= a[j] * y_states[j];
        }

        /* Prepare the next states */
        y_states[0] = buf[total_dimensions*i + cur_dimension];
        for (int j = n_order - 1; j > -1; j--)
        {
            x_states[j + 1] = x_states[j];
            y_states[j + 1] = y_states[j];
        }
    }
}

/*******************************************************************************
* Function Name: normalization_min_max
********************************************************************************
* Summary:
*   Normalizes the input data between max and minimum values. The normalized data
*   is stored in the passed in buffer.
*
* Parameters:
*   buf: A buffer with data to normalize
*   length: The number of rows in the passed in buffer
*   dimension: The number of columns in the passed in buffer
*   sensor_min: Minimum data in the data set
*   sensor_max: Maximum value in the data set
*
*******************************************************************************/
void normalization_min_max(float *buf, uint16_t length, uint16_t dimension, float sensor_min, float sensor_max)
{
    uint16_t cur_row = 0;
    uint8_t cur_col = 0;

    /* Scaler used for scaling data to the max min size */
    float scaler = (MAX_NORMALIZATION_VALUE - MIN_NORMALIZATION_VALUE)/(sensor_max - sensor_min);

    /* Completes calculations for each row */
    while(length > cur_row)
    {
        /* Completes calculations for each column */
        while(dimension > cur_col)
        {
            /* Check the bounds of the data */
            if(buf[cur_row*dimension+cur_col] > sensor_max)
            {
                /* If data is larger than the max, set to max value */
                buf[cur_row*dimension+cur_col] = MAX_NORMALIZATION_VALUE;
            }
            else if(buf[cur_row*dimension+cur_col] < sensor_min)
            {
                /* If data is smaller than the minimum, set to minimum value */
                buf[cur_row*dimension+cur_col] = MIN_NORMALIZATION_VALUE;
            }
            /* Translate data into values between -1 and 1 */
            else
            {
                buf[cur_row*dimension+cur_col] = MAX_NORMALIZATION_VALUE-((sensor_max - buf[cur_row*dimension+cur_col]) * scaler);
            }
            cur_col++;
        }
        cur_col = 0;
        cur_row++;
    }
}

/*******************************************************************************
* Function Name: cast_int16_to_float
********************************************************************************
* Summary:
*   Casts data from an int16 to a float. Data is transfered to a new buffer.
*
* Parameters:
*   int_buf: A buffer with int16 values
*   float_buf: A buffer to store the casted float values
*   length: The number of rows in the passed in buffer
*
*******************************************************************************/
void cast_int16_to_float(int16_t *int_buf, float *float_buf, uint16_t length)
{
    uint16_t cur = 0;

    /* Cast all data */
    while(length > cur)
    {
        float_buf[cur] = (float)int_buf[cur];
        cur++;
    }
}

/*******************************************************************************
* Function Name: column_swap
********************************************************************************
* Summary:
*   Swaps two columns in an array.
*
* Parameters:
*   buf: A buffer with data to swap
*   length: The number of rows in the passed in buffer
*   dimension: The number of columns in the passed in buffer
*   column_one: The column that will be swapped with column_two
*   column_two: The column that will be swapped with column_one
*
*******************************************************************************/
void column_swap(float *buf, uint16_t length, uint16_t dimension, uint16_t column_one, uint16_t column_two)
{
    uint16_t cur_row = 0;
    float temp = 0;

    /* Completes calculations for each row */
    while(length > cur_row)
    {
        /* Swap the specified columns */
        temp = buf[cur_row*dimension+column_one];
        buf[cur_row*dimension+column_one] = buf[cur_row*dimension+column_two];
        buf[cur_row*dimension+column_two] = temp;
        cur_row++;
    }
}

/*******************************************************************************
* Function Name: column_inverse
********************************************************************************
* Summary:
*   Inverts of each element of a given column in an array.
*
* Parameters:
*   buf: A buffer with data invert
*   length: The number of rows in the passed in buffer
*   dimension: The number of columns in the passed in buffer
*   column: The column that will be inverted
*
*******************************************************************************/
void column_inverse(float *buf, uint16_t length, uint16_t dimension, uint16_t column)
{
    uint16_t cur_row = 0;

    /* Completes calculations for each row */
    while(length > cur_row)
    {
        /* Swap the specified columns */
        buf[cur_row*dimension+column] = buf[cur_row*dimension+column] * -1;
        cur_row++;
    }
}

