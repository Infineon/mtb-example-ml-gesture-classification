/******************************************************************************
* File Name:   nn_utils.h
*
* Description: This file contains the function prototypes and constants used
*   in nn_utils.c.
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
#ifndef NN_UTILS_H
#define NN_UTILS_H

#include <stdint.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Defines
 *****************************************************************************/

/******************************************************************************
 * Typedefs
 *****************************************************************************/

/*******************************************************************************
* Constants
*******************************************************************************/

/*******************************************************************************
* Functions
*******************************************************************************/
int  nn_utils_find_max_index_flt(float *in, int len);
int  nn_utils_find_max_index_int(int *in, int len);
void nn_utils_convert_int16_to_flt(int16_t *in, float *out, int len, int q);
void nn_utils_convert_int8_to_flt(int8_t *in, float *out, int len, int q);
void nn_utils_convert_flt_to_int16(float *in, int16_t *out, int len, float q);
void nn_utils_convert_flt_to_int8(float *in, int8_t *out, int len, float q);

#endif /* NN_UTILS_H */

/* [] END OF FILE */
