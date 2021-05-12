/*******************************************************************************
* File Name: cy_fifo.c
*
* Description:
* FIFO implementation.
*
********************************************************************************
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
*******************************************************************************/

#include "cy_fifo.h"
#include <string.h>

#if defined(__cplusplus)
extern "C"
{
#endif

/*******************************************************************************
*       Functions
*******************************************************************************/
cy_rslt_t cy_fifo_init_static(cy_fifo_t *obj, void *pool, uint32_t pool_size, uint32_t item_size)
{
    /* Check if arguments are correct */
    if ((obj != NULL) && (pool != NULL) && (pool_size != 0) && (item_size != 0))
    {
        obj->fifo_size = pool_size / item_size;
        if (obj->fifo_size == 0)
        {
            return CY_FIFO_RESULT_BAD_ARG;
        }
        obj->pool = pool;
        obj->pool_size = pool_size;
        obj->item_size = item_size;
        obj->read_offset = 0;
        obj->write_offset = 0;
        obj->num_items = 0;
        
        return CY_RSLT_SUCCESS;
    }
    
    return CY_FIFO_RESULT_BAD_ARG;
}


cy_rslt_t cy_fifo_write(cy_fifo_t *obj, void *buffer, uint32_t count)
{
    uint8_t *buf_ptr = buffer;
    if ((obj->fifo_size - obj->num_items) >= count)
    {
        while (count > 0)
        {
            memcpy((void *) ((uint32_t) obj->pool + (obj->write_offset*obj->item_size)), buf_ptr, obj->item_size);
            buf_ptr += obj->item_size;
            count--;
            obj->write_offset++;
            if (obj->write_offset >= obj->fifo_size)
            {
                obj->write_offset = 0;
            }
            obj->num_items++;            
        }

        return CY_RSLT_SUCCESS;
    }

    return CY_FIFO_RESULT_OVERFLOW;
}

cy_rslt_t cy_fifo_write_update(cy_fifo_t *obj, uint32_t count)
{
    if ((obj->fifo_size - obj->num_items) >= count)
    {
        obj->write_offset = (obj->write_offset + count) % obj->fifo_size;
        obj->num_items += count;
        
        return CY_RSLT_SUCCESS;
    }

    return CY_FIFO_RESULT_OVERFLOW;
}

cy_rslt_t cy_fifo_read(cy_fifo_t *obj, void *buffer, uint32_t count)
{
    uint8_t *buf_ptr = buffer;
    if (obj->num_items >= count)
    {
        while (count > 0)
        {
            memcpy(buf_ptr, (const void *) ((uint32_t) obj->pool + (obj->read_offset*obj->item_size)), obj->item_size);
            buf_ptr += obj->item_size;
            count--;
            obj->read_offset++;
            if (obj->read_offset >= obj->fifo_size)
            {
                obj->read_offset = 0;
            }
            obj->num_items--;
        }

        return CY_RSLT_SUCCESS;
    }

    return CY_FIFO_RESULT_UNDERFLOW;
}

cy_rslt_t cy_fifo_read_silent(cy_fifo_t *obj, void *buffer, uint32_t count)
{
    uint8_t *buf_ptr = buffer;
    uint32_t read_offset = obj->read_offset;
    if (obj->num_items >= count)
    {
        while (count > 0)
        {
            memcpy(buf_ptr, (const void *) ((uint32_t) obj->pool + (read_offset*obj->item_size)), obj->item_size);
            buf_ptr += obj->item_size;
            count--;
            read_offset++;
            if (read_offset >= obj->fifo_size)
            {
                read_offset = 0;
            }
        }

        return CY_RSLT_SUCCESS;
    }

    return CY_FIFO_RESULT_UNDERFLOW;
}

cy_rslt_t cy_fifo_read_update(cy_fifo_t *obj, uint32_t count)
{
    if (obj->num_items >= count)
    {
        obj->read_offset = (obj->read_offset + count) % obj->fifo_size;
        obj->num_items -= count;
        
        return CY_RSLT_SUCCESS;
    }

    return CY_FIFO_RESULT_UNDERFLOW;    
}

bool cy_fifo_is_full(cy_fifo_t *obj)
{
    return (obj->fifo_size == obj->num_items);
}

bool cy_fifo_is_empty(cy_fifo_t *obj)
{
    return (obj->num_items == 0);
}

void cy_fifo_clear(cy_fifo_t *obj)
{
    obj->write_offset = 0;
    obj->read_offset = 0;
    obj->num_items = 0;
}

uint32_t cy_fifo_get_count(cy_fifo_t *obj)
{
    return (obj->num_items);
}

#if defined(__cplusplus)
}
#endif