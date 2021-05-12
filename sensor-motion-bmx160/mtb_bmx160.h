/**************************************************************************//**
 * \file mtb_bmx160.h
 *
 * Description: This file is the public interface of the BMI160 motion sensor.
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

#pragma once

/**
* \addtogroup group_board_libs Motion Sensor
* \{
* Basic set of APIs for interacting with the BMI160 motion sensor. This
* provides basic initialization and access to to the basic accelerometer &
* gyroscope data. It also provides access to the base BMI160 driver for full
* control. For more information about the motion sensor, see:
* https://github.com/BoschSensortec/BMI160_driver
*
* \note Currently, this library only supports being used for a single instance
* of this device.
*
* \note BMI160 support requires delays. If the RTOS_AWARE component is set or
* CY_RTOS_AWARE is defined, the HAL driver will defer to the RTOS for delays.
* Because of this, it is not safe to call any functions other than
* \ref mtb_bmi160_init_i2c until after the RTOS scheduler has started.
*
* \note There is a known issue with the BMI160 endianness detection. Any code
* referencing the structures defined in the BMI160 driver should have this header
* file, mtb_bmi160.h, first in any includes.
*
* \section subsection_board_libs_snippets Code snippets
* \subsection subsection_board_libs_snippet_1 Snippet 1: Simple initialization with I2C.
* The following snippet initializes an I2C instance and the BMI160, then reads
* from the BMI160.
* \snippet mtb_imu_bmi160_example.c snippet_bmi160_i2c_init
*
* \subsection subsection_board_libs_snippet_2 Snippet 2: BMI160 interrupt configuration.
* The following snippet demonstrates how to configure a BMI160 interrupt.
* \snippet mtb_imu_bmi160_example.c snippet_bmi160_configure_interrupt
*/

#include "bmi160.h"
#include "bmm150.h"
#include "cy_result.h"
#include "cyhal_gpio.h"
#include "cyhal_i2c.h"

#if defined(__cplusplus)
extern "C"
{
#endif

/** Structure holding the IMU instance specific information. */
typedef struct
{
    struct bmi160_dev sensor;
    struct bmm150_dev magnometer;
    bool   mag_enable;
    cyhal_gpio_t intpin1;
    cyhal_gpio_t intpin2;
} mtb_bmx160_t;

/** Structure holding the accelerometer and gyroscope data read from the device. */
typedef struct
{
    /** Accelerometer data */
    struct bmi160_sensor_data accel;
    /** Gyroscope data */
    struct bmi160_sensor_data gyro;
    /** Magnometer data */
    struct bmm150_mag_data mag;
} mtb_bmx160_data_t;

/** Enumeration used for selecting I2C address. */
typedef enum
{
    MTB_BMX160_DEFAULT_ADDRESS = BMI160_I2C_ADDR,
    MTB_BMX160_SECONDARY_ADDRESS = BMI160_AUX_BMM150_I2C_ADDR
}mtb_bmx160_address_t;

/** An attempt was made to configure too many gpio pins as interrupts. */
#define CYHAL_BMX160_RSLT_ERR_ADDITIONAL_INT_PIN                 \
    (CYHAL_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_BOARD_HARDWARE_BMI160, 0x100))

/**
 * Initialize the IMU for I2C communication. Then applies the default
 * configuration settings for both the accelerometer & gyroscope. Known maximum
 * I2C frequency of 1MHz; refer to manufacturer's datasheet for confirmation.
 * See: \ref mtb_bmx160_config_default()
 * @param[in] obj       Pointer to a BMX160 object. The caller must allocate the memory
 *  for this object but the init function will initialize its contents.
 * @param[in] inst      I2C instance to use for communicating with the BMI160 sensor.
 * @param[in] address   BMI160 I2C address, set by hardware implementation.
 * @return CY_RSLT_SUCCESS if properly initialized, else an error indicating what went wrong.
 */
cy_rslt_t mtb_bmx160_init_i2c(mtb_bmx160_t *obj, cyhal_i2c_t *inst, mtb_bmx160_address_t address);

/**
 * Initialize the IMU for SPI communication. Then applies the default
 * configuration settings for both the accelerometer & gyroscope. Known maximum
 * SPI frequency of 10MHz; refer to manufacturer's datasheet for confirmation.
 * See: \ref mtb_bmx160_config_default()
 * @param[in] obj       Pointer to a BMX160 object. The caller must allocate the memory
 *  for this object but the init function will initialize its contents.
 * @param[in] inst      SPI instance to use for communicating with the BMI160 sensor.
 * @param[in] ss        BMX160 SPI slave select pin
 * @return CY_RSLT_SUCCESS if properly initialized, else an error indicating what went wrong.
 */
cy_rslt_t mtb_bmx160_init_spi(mtb_bmx160_t *obj, cyhal_spi_t *inst, cyhal_gpio_t ss);

/**
 * Initialize the internal magnometer. 
 * @param[in] obj       Pointer to a BMX160 object. The caller must allocate the memory
 *  for this object but the init function will initialize its contents.
 * @return CY_RSLT_SUCCESS if properly initialized, else an error indicating what went wrong.
 */
cy_rslt_t mtb_bmx160_init_mag(mtb_bmx160_t *obj);

/**
 * Configure the motion sensor to a default mode with both accelerometer & gyroscope enabled
 * with a nominal output data rate. The default values used are from the example in the BMI160
 * driver repository, see https://github.com/BoschSensortec/BMI160_driver
 * @param[in] obj  Pointer to a BMX160 object.
 * @return CY_RSLT_SUCCESS if properly initialized, else an error indicating what went wrong.
 */
cy_rslt_t mtb_bmx160_config_default(mtb_bmx160_t *obj);

/**
 * Reads the current accelerometer & gyroscope data from the motion sensor.
 * @param[in] obj  Pointer to a BMI160 object.
 * @param[out] sensor_data The accelerometer & gyroscope data read from the motion sensor
 * @return CY_RSLT_SUCCESS if properly initialized, else an error indicating what went wrong.
 */
cy_rslt_t mtb_bmx160_read(mtb_bmx160_t *obj, mtb_bmx160_data_t *sensor_data);

/**
 * Gets access to the base motion sensor data. This allows for direct manipulation of the
 * sensor for any desired behavior. See https://github.com/BoschSensortec/BMI160_driver for
 * more details on the sensor.
 * @param[in] obj  Pointer to a BMX160 object.
 * @return pointer to the BMI160 configuration structure.
 */
struct bmi160_dev *mtb_bmx160_get(mtb_bmx160_t *obj);

/**
 * Performs both accelerometer and gyro self tests. Note these tests cause a soft reset
 * of the device and device should be reconfigured after a test.
 * See https://github.com/BoschSensortec/BMI160_driver for more details.
 * @param[in] obj  Pointer to a BMX160 object.
 * @return CY_RSLT_SUCCESS if self tests pass, else an error indicating what went wrong.
 */
cy_rslt_t mtb_bmx160_selftest(mtb_bmx160_t *obj);

/**
 * Configure a GPIO pin as an interrupt for the BMI160.
 * This configures the pin as an interrupt, and calls the BMI160 interrupt configuration API
 * with the application supplied settings structure.
 * See https://github.com/BoschSensortec/BMI160_driver for more details.
 * @param[in] obj           Pointer to a BMX160 object.
 * @param[in] intsettings   Pointer to a BMX160 interrupt settings structure.
 * @param[in] pin           Which pin to configure as interrupt
 * @param[in] intr_priority The priority for NVIC interrupt events
 * @param[in] event         The type of interrupt event
 * @param[in] callback      The function to call when the specified event happens. Pass NULL to unregister the handler.
 * @param[in] callback_arg  Generic argument that will be provided to the callback when called, can be NULL
 * @return CY_RSLT_SUCCESS if interrupt was succesffuly enabled.
 */
cy_rslt_t mtb_bmx160_config_int(mtb_bmx160_t *obj, struct bmi160_int_settg *intsettings, cyhal_gpio_t pin, uint8_t intr_priority, cyhal_gpio_event_t event, cyhal_gpio_event_callback_t callback, void *callback_arg);

/**
 * Frees up any resources allocated by the motion_sensor as part of \ref mtb_bmi160_init_i2c().
 * @param[in] obj  Pointer to a BMI160 object.
*/
void mtb_bmi160_free(mtb_bmx160_t * obj);

#if defined(__cplusplus)
}
#endif

/** \} group_board_libs */
