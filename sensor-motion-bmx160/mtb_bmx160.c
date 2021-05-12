/**************************************************************************//**
 * \file mtb_bmx160.c
 *
 * Description: This file contains the functions for interacting with the
 *              motion sensor.
 *
 *******************************************************************************
 * \copyright
 * Copyright 2018-2020 Cypress Semiconductor Corporation
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

#include "mtb_bmx160.h"
#include "cyhal_i2c.h"
#include "cyhal_spi.h"
#include "cyhal_gpio.h"
#include "cyhal_system.h"

#if defined(__cplusplus)
extern "C"
{
#endif

#define I2C_TIMEOUT         10 /* 10 msec */
#define BMX160_ERROR(x)     (CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_BOARD_HARDWARE_BMI160, x))
#define I2C_WRITE_BUFFER_LENGTH   32

static cyhal_i2c_t *i2c = NULL;
static cyhal_spi_t *spi = NULL;
static cyhal_gpio_t spi_ss = NC;

static mtb_bmx160_t *ptr;

static int8_t i2c_write_bytes(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    CY_ASSERT(len + 1 < I2C_WRITE_BUFFER_LENGTH);
    uint8_t buf[I2C_WRITE_BUFFER_LENGTH];
    buf[0] = reg_addr;
    for(uint16_t i=0; i<len; i++)
    {
        buf[i+1] = data[i];
    }

    cy_rslt_t result = cyhal_i2c_master_write(i2c, dev_addr, buf, len+1, I2C_TIMEOUT, true);

    return (CY_RSLT_SUCCESS == result)
        ? BMI160_OK
        : BMI160_E_COM_FAIL;
}

static int8_t i2c_read_bytes(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    cy_rslt_t result = cyhal_i2c_master_write(i2c, dev_addr, &reg_addr, 1, I2C_TIMEOUT, false);

    if (CY_RSLT_SUCCESS == result)
        result = cyhal_i2c_master_read(i2c, dev_addr, data, len, I2C_TIMEOUT, true);

    return (CY_RSLT_SUCCESS == result)
        ? BMI160_OK
        : BMI160_E_COM_FAIL;
}

static int8_t spi_write_bytes(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    cyhal_gpio_write(spi_ss, 0);
    result |= cyhal_spi_send(spi, (uint32_t) reg_addr);

    for (int i = 0; i < len; i++)
    {
        result |= cyhal_spi_send(spi, (uint32_t) data[i]);
    }
    cyhal_gpio_write(spi_ss, 1);

    return (CY_RSLT_SUCCESS == result)
        ? BMI160_OK
        : BMI160_E_COM_FAIL;
}

static int8_t spi_read_bytes(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    uint8_t value = reg_addr | 0x80;

    cyhal_gpio_write(spi_ss, 0);
    result |= cyhal_spi_send(spi, (uint32_t) value);

    for (int i = 0; i < len; i++)
    {
        result |= cyhal_spi_recv(spi, (uint32_t *) &data[i]);
    }
    cyhal_gpio_write(spi_ss, 1);

    return (CY_RSLT_SUCCESS == result)
        ? BMI160_OK
        : BMI160_E_COM_FAIL;
}

int8_t bmm150_aux_read(uint8_t id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len) {
    (void) id;
    return bmi160_aux_read(reg_addr, reg_data, len, &(ptr->sensor));
}

int8_t bmm150_aux_write(uint8_t id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len) {
    (void) id;
    return bmi160_aux_write(reg_addr, reg_data, len, &(ptr->sensor));
}

static void delay_wrapper(uint32_t ms)
{
    (void)cyhal_system_delay_ms(ms);
}

cy_rslt_t mtb_bmx160_init_i2c(mtb_bmx160_t *obj, cyhal_i2c_t *inst, mtb_bmx160_address_t address)
{
    CY_ASSERT(inst != NULL);
    i2c = inst;
    obj->mag_enable = false;

    /* Configure the BMI160 structure */
    obj->sensor.id           = address;
    obj->sensor.interface    = BMI160_I2C_INTF;
    obj->sensor.read         = (bmi160_com_fptr_t)i2c_read_bytes;
    obj->sensor.write        = (bmi160_com_fptr_t)i2c_write_bytes;
    obj->sensor.delay_ms     = delay_wrapper;
    obj->intpin1             = NC;
    obj->intpin2             = NC;

    /* Initialize BNI160 sensor */
    int8_t status = bmi160_init(&(obj->sensor));

    return (BMI160_OK == status) /* BMI160 initialization successful */
        ? mtb_bmx160_config_default(obj)
        : BMX160_ERROR(status);
}

cy_rslt_t mtb_bmx160_init_spi(mtb_bmx160_t *obj, cyhal_spi_t *inst, cyhal_gpio_t ss)
{
    CY_ASSERT(inst != NULL);
    spi = inst;
    obj->mag_enable = false;
    
    /* Configure the BMI160 structure */
    obj->sensor.id           = 0;
    obj->sensor.interface    = BMI160_SPI_INTF;
    obj->sensor.read         = (bmi160_com_fptr_t)spi_read_bytes;
    obj->sensor.write        = (bmi160_com_fptr_t)spi_write_bytes;
    obj->sensor.delay_ms     = delay_wrapper;
    obj->intpin1             = NC;
    obj->intpin2             = NC;

    spi_ss = ss;

    /* Initialize CS pin */
    cy_rslt_t result = cyhal_gpio_init(spi_ss, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0);
    if (result != CY_RSLT_SUCCESS) return result;
    cyhal_gpio_write(spi_ss, 1);

    /* Initialize BNI160 sensor */
    int8_t status = bmi160_init(&(obj->sensor));

    return (BMI160_OK == status) /* BMI160 initialization successful */
        ? mtb_bmx160_config_default(obj)
        : BMX160_ERROR(status);
}

cy_rslt_t mtb_bmx160_init_mag(mtb_bmx160_t *obj)
{
    ptr = obj;
    obj->mag_enable = true;

    obj->magnometer.dev_id = BMM150_DEFAULT_I2C_ADDRESS;
    obj->magnometer.intf = BMM150_I2C_INTF;
    obj->magnometer.read = bmm150_aux_read;
    obj->magnometer.write = bmm150_aux_write;
    obj->magnometer.delay_ms = delay_wrapper;

    obj->sensor.aux_cfg.aux_i2c_addr = obj->magnometer.dev_id;
    obj->sensor.aux_cfg.aux_sensor_enable = BMI160_ENABLE;
    obj->sensor.aux_cfg.manual_enable = BMI160_ENABLE;
    obj->sensor.aux_cfg.aux_rd_burst_len = BMI160_AUX_READ_LEN_2;

    /* Initialize BMI160 auxiliar sensor */
    int8_t status = 0;

    status |= bmi160_aux_init(&(obj->sensor));
    status |= bmm150_init(&obj->magnometer);

    obj->magnometer.settings.preset_mode = BMM150_PRESETMODE_REGULAR;
    bmm150_set_presetmode(&obj->magnometer);

    obj->magnometer.settings.pwr_mode = BMM150_FORCED_MODE;
    bmm150_set_op_mode(&obj->magnometer);

    uint8_t aux_addr = 0x42;       

    obj->sensor.aux_cfg.aux_odr = 8;
    status |= bmi160_config_aux_mode(&(obj->sensor));
    status |= bmi160_set_aux_auto_mode(&aux_addr, &(obj->sensor));

    return (BMI160_OK == status)
        ? CY_RSLT_SUCCESS
        : BMX160_ERROR(status);
}

cy_rslt_t mtb_bmx160_config_default(mtb_bmx160_t * obj)
{
    /* Select the Output data rate, range of accelerometer sensor */
    obj->sensor.accel_cfg.odr = BMI160_ACCEL_ODR_200HZ;
    obj->sensor.accel_cfg.range = BMI160_ACCEL_RANGE_4G;
    obj->sensor.accel_cfg.bw = BMI160_ACCEL_BW_NORMAL_AVG4;

    /* Select the power mode of accelerometer sensor */
    obj->sensor.accel_cfg.power = BMI160_ACCEL_NORMAL_MODE;

    /* Select the Output data rate, range of gyroscope sensor */
    obj->sensor.gyro_cfg.odr = BMI160_GYRO_ODR_200HZ;
    obj->sensor.gyro_cfg.range = BMI160_GYRO_RANGE_2000_DPS;
    obj->sensor.gyro_cfg.bw = BMI160_GYRO_BW_NORMAL_MODE;

    /* Select the power mode of gyroscope sensor */
    obj->sensor.gyro_cfg.power = BMI160_GYRO_NORMAL_MODE;

    /* Set the sensor configuration */
    int8_t status = bmi160_set_sens_conf(&(obj->sensor));

    return (BMI160_OK == status)
        ? CY_RSLT_SUCCESS
        : BMX160_ERROR(status);
}

cy_rslt_t mtb_bmx160_read(mtb_bmx160_t *obj, mtb_bmx160_data_t *sensor_data)
{
    /* To read both Accel and Gyro data along with time*/
    int8_t status = bmi160_get_sensor_data((BMI160_ACCEL_SEL | BMI160_GYRO_SEL | BMI160_TIME_SEL),
        &(sensor_data->accel), &(sensor_data->gyro), &(obj->sensor));

    /* Read Mag sensor data */
    if (status == BMI160_OK)
    {
        if (obj->mag_enable)
        {
            status = bmm150_read_mag_data(&(obj->magnometer));
            if (status == BMI160_OK)
            {
                sensor_data->mag.x = obj->magnometer.data.x;
                sensor_data->mag.y = obj->magnometer.data.y;
                sensor_data->mag.z = obj->magnometer.data.z;
            }
        }
    }

    return (BMI160_OK == status)
        ? CY_RSLT_SUCCESS
        : BMX160_ERROR(status);
}

struct bmi160_dev *mtb_bmx160_get(mtb_bmx160_t *obj)
{
    return &(obj->sensor);
}

cy_rslt_t mtb_bmx160_selftest(mtb_bmx160_t *obj)
{
    int8_t status = bmi160_perform_self_test(BMI160_ACCEL_SEL, &(obj->sensor));

    if (status == BMI160_OK)
    {
        status = bmi160_perform_self_test(BMI160_GYRO_SEL, &(obj->sensor));
    }

    return (BMI160_OK == status)
        ? CY_RSLT_SUCCESS
        : BMX160_ERROR(status);
}

cy_rslt_t _mtb_bmx160_config_int(cyhal_gpio_t *intpin, cyhal_gpio_t pin, bool init, uint8_t intr_priority, cyhal_gpio_event_t event, cyhal_gpio_event_callback_t callback, void *callback_arg)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    if (NULL == callback)
    {
        cyhal_gpio_free(pin);
        *intpin = NC;
    }
    else
    {
        if (init)
        {
            result = cyhal_gpio_init(pin, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, 0);
        }
        if (CY_RSLT_SUCCESS == result)
        {
            *intpin = pin;
            cyhal_gpio_register_callback(pin, callback, callback_arg);
            cyhal_gpio_enable_event(pin, event, intr_priority, 1);
        }
    }

    return result;
}

cy_rslt_t mtb_bmx160_config_int(mtb_bmx160_t *obj, struct bmi160_int_settg *intsettings, cyhal_gpio_t pin, uint8_t intr_priority, cyhal_gpio_event_t event, cyhal_gpio_event_callback_t callback, void *callback_arg)
{
    cy_rslt_t result;

    if (obj->intpin1 == pin)
    {
        result = _mtb_bmx160_config_int(&(obj->intpin1), pin, false, intr_priority, event, callback, callback_arg);
    }
    else if (obj->intpin2 == pin)
    {
        result = _mtb_bmx160_config_int(&(obj->intpin2), pin, false, intr_priority, event, callback, callback_arg);
    }
    else if (obj->intpin1 == NC)
    {
        result = _mtb_bmx160_config_int(&(obj->intpin1), pin, true, intr_priority, event, callback, callback_arg);
    }
    else if (obj->intpin2 == NC)
    {
        result = _mtb_bmx160_config_int(&(obj->intpin2), pin, true, intr_priority, event, callback, callback_arg);
    }
    else
    {
        result = CYHAL_BMX160_RSLT_ERR_ADDITIONAL_INT_PIN;
    }

    if (result == CY_RSLT_SUCCESS)
    {
        int8_t status = bmi160_set_int_config(intsettings, &(obj->sensor));
        if (status != BMI160_OK) result = BMX160_ERROR(status);
    }

    return result;
}

void mtb_bmx160_free(mtb_bmx160_t * obj)
{
    if(obj->intpin1 != NC)
    {
        cyhal_gpio_free(obj->intpin1);
    }

    if(obj->intpin2 != NC)
    {
        cyhal_gpio_free(obj->intpin2);
    }
    obj->mag_enable = false;
    i2c = NULL;
    spi = NULL;
    spi_ss = NC;
}

#if defined(__cplusplus)
}
#endif
