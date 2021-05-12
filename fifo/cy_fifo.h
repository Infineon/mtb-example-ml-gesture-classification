/***********************************************************************************************//**
 * \file cy_fifo.h
 *
 * \brief
 * Provides APIs for handling a FIFO in firmware.
 *
 ***************************************************************************************************
 * \copyright
 * Copyright 2018-2021 Cypress Semiconductor Corporation
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **************************************************************************************************/

#pragma once

#include <stdio.h>
#include <stdbool.h>
#include "cy_result.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*******************************************************************************
* Constants
*******************************************************************************/
#define CY_FIFO_RESULT_ERROR        1u
#define CY_FIFO_RESULT_BAD_ARG      2u
#define CY_FIFO_RESULT_ALLOC_ERR    3u
#define CY_FIFO_RESULT_OVERFLOW     4u
#define CY_FIFO_RESULT_UNDERFLOW    5u

/** @brief FIFO structure */
typedef struct
{
    void *pool;             //!< Pointer to the pool buffer
    uint32_t pool_size;     //!< Size of the pool buffer
    uint32_t item_size;     //!< Size of one item in the FIFO
    uint32_t read_offset;   //!< Offset indicating where data is read
    uint32_t write_offset;  //!< Offset indicating where data is written
    uint32_t num_items;     //!< Number of items currently in the FIFO
    uint32_t fifo_size;     //!< Maximum number of items in the FIFO
} cy_fifo_t;

/**
 * \brief Initialize the FIFO statically. Need to provide a pointer to the pool buffer.
 *
 * \param obj FIFO object
 * \param pool Pointer to the pool buffer
 * \param pool_size Size of the pool buffer
 * \param item_size Size of one item in the FIFO
 * \returns CY_RSLT_SUCCESS if successfully initialized, else an error about
 * what went wrong
 */
cy_rslt_t cy_fifo_init_static(cy_fifo_t *obj, void *pool, uint32_t pool_size, uint32_t item_size);

/**
 * \brief Write multiple items to the FIFO.
 * \returns CY_RSLT_SUCCESS if successfully written, else an error about what went wrong
 */
cy_rslt_t cy_fifo_write(cy_fifo_t *obj, void *buffer, uint32_t count);

/**
 * \brief Update FIFO internal write offset, without writing data to the FIFO.
 * \returns CY_RSLT_SUCCESS if successfully written, else an error about what went wrong
 */
cy_rslt_t cy_fifo_write_update(cy_fifo_t *obj, uint32_t count);

/**
 * \brief Read multiple items from the FIFO.
 * \returns CY_RSLT_SUCCESS if successfully written, else an error about what went wrong
 */
cy_rslt_t cy_fifo_read(cy_fifo_t *obj, void *buffer, uint32_t count);

/**
 * \brief Read multiple items from the FIFO silently, without changing the internal read offset.
 * \returns CY_RSLT_SUCCESS if successfully written, else an error about what went wrong
 */
cy_rslt_t cy_fifo_read_silent(cy_fifo_t *obj, void *buffer, uint32_t count);

/**
 * \brief Update FIFO internal read offset, without reading data from the FIFO.
 * \returns CY_RSLT_SUCCESS if successfully written, else an error about what went wrong
 */
cy_rslt_t cy_fifo_read_update(cy_fifo_t *obj, uint32_t count);

/**
 * \brief Indicates if the FIFO is full
 * \returns TRUE if FIFO is full, else FALSE.
 */
bool cy_fifo_is_full(cy_fifo_t *obj);

/**
 * \brief Indicates if the FIFO is empty
 * \returns TRUE if FIFO is empty, else FALSE.
 */
bool cy_fifo_is_empty(cy_fifo_t *obj);

/**
 * \brief Clear FIFO.
 */
void cy_fifo_clear(cy_fifo_t *obj);

/**
 * \brief Returns the number of items in the FIFO
 * \returns Number of items in the FIFO
 */
uint32_t cy_fifo_get_count(cy_fifo_t *obj);

#if defined(__cplusplus)
}
#endif

