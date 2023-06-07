/******************************************************************************
* File Name:   processing.h
*
* Description: This file contains the function prototypes and constants used
*   in processing.c.
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
#ifndef PROCESSING_H
#define PROCESSING_H

#include <stdio.h>
#include <stdint.h>
#include <math.h>

/******************************************************************************
 * Defines
 *****************************************************************************/
#define MAX_STATES 10

/******************************************************************************
 * Typedefs
 *****************************************************************************/
typedef struct iir_filter_struct_t
{
    const float* b;
    const float* a;
    float x_states[MAX_STATES];
    float y_states[MAX_STATES];
    int n_order;
}iir_filter_struct;

/* Coefficients for 3rd order butter-worth filter */
#define IIR_FILTER_BUTTER_WORTH_COEFF_B { 0.01809893f, 0.0542968f , 0.0542968f , 0.01809893f }
#define IIR_FILTER_BUTTER_WORTH_COEFF_A { 1.0f        , -1.76004188f,  1.18289326f, -0.27805992f }

/*******************************************************************************
* Functions
*******************************************************************************/
int iir_filter_init(iir_filter_struct* st, const float* b, const float* a, int n_order);
void iir_filter(iir_filter_struct *st, float *buf, int length, uint16_t cur_dimension, uint16_t total_dimensions);
void normalization_min_max(float *buf, uint16_t length, uint16_t dimension,float sensor_min, float sensor_max);
void cast_int16_to_float(int16_t *int_buf, float *float_buf, uint16_t length);
void column_swap(float *buf, uint16_t length, uint16_t dimension, uint16_t column_one, uint16_t column_two);
void column_inverse(float *buf, uint16_t length, uint16_t dimension, uint16_t column);

#endif /* PROCESSING_H */
