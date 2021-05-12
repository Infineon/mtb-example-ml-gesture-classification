/******************************************************************************
* File Name:   sensor.c
*
* Description: This file contains the implementation to read data from the IMU
*                 and process it in chunks of 128 samples. The data is then fed
*                 to the neural network.
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
#include "processing.h"
#include "sensor.h"
#include "nn_utils.h"

#include "cyhal.h"
#include "cybsp.h"

#include "mtb_bmx160.h"
#include "bmm150.h"

#include "cy_fifo.h"

/* Include model files */
#include INCLUDE_FILE(MODEL_NAME, MODEL_FILE_EXT)
#include INCLUDE_FILE(MODEL_NAME, X_DATA_FILE_EXT)
#include INCLUDE_FILE(MODEL_NAME, Y_DATA_FILE_EXT)

/*******************************************************************************
* Typedefs
*******************************************************************************/
typedef struct
{
    const char          *name;
    const unsigned char *weights;
    const unsigned char *parameters;
    const unsigned char *x_regression_data;
    const unsigned char *y_regression_data;
} nn_model_t;

/*******************************************************************************
* Constants
*******************************************************************************/
#define SENSOR_EVENT_BIT 1u<<2

#define IMU_SPI_FREQUENCY 10000000

#define SENSOR_DATA_WIDTH 2
#define SENSOR_NUM_AXIS   6
#define SENSOR_SAMPLE_SIZE (SENSOR_DATA_WIDTH * SENSOR_NUM_AXIS)
#define SENSOR_BATCH_SIZE  128u

#define SENSOR_FIFO_ITEM_SIZE (SENSOR_SAMPLE_SIZE / 2)
#define SENSOR_FIFO_POOL_SIZE (2*SENSOR_BATCH_SIZE * SENSOR_SAMPLE_SIZE)

#define SENSOR_SCAN_RATE       128
#define SENSOR_TIMER_FREQUENCY 100000
#define SENSOR_TIMER_PERIOD (SENSOR_TIMER_FREQUENCY/SENSOR_SCAN_RATE)
#define SENSOR_TIMER_PRIORITY  3

#define MAX_DATA_SAMPLE 32768
#define MIN_DATA_SAMPLE -32768

#if CY_ML_FIXED_POINT_16_IN
    #define QFORMAT_SCALE    32767
    #define QFORMAT_VALUE    15
#endif
#if CY_ML_FIXED_POINT_8_IN
    #define QFORMAT_SCALE    127
    #define QFORMAT_VALUE    7
#endif

/*******************************************************************************
* Global Variables
*******************************************************************************/
/* Model information */
nn_model_t nn_gesture_model =
{
    .name              = EXPAND_AND_STRINGIFY(MODEL_NAME),
    .weights           = MODEL_DATA(MODEL_NAME, MODEL_PTR_EXT),
    .parameters        = MODEL_DATA(MODEL_NAME, MODEL_PARAM_EXT),
    .x_regression_data = MODEL_DATA(MODEL_NAME, X_DATA_PTR_EXT),
    .y_regression_data = MODEL_DATA(MODEL_NAME, Y_DATA_PTR_EXT)
};

/* Global timer used for getting data */
cyhal_timer_t sensor_timer;

/* Event set when data is done collecting */
cy_event_t sensor_event;
uint32_t sensor_event_bits  = SENSOR_EVENT_BIT;


/* Circle buffer to store IMU data */
static cy_fifo_t sensor_fifo;
int8_t sensor_fifo_pool[SENSOR_FIFO_POOL_SIZE];

/* IMU driver structures */
mtb_bmx160_data_t data;
mtb_bmx160_t sensor_bmx160;

/* SPI object for data transmission */
cyhal_spi_t spi;

/* Neural Network Object */
nn_obj_t nn_gesture_obj;

/*******************************************************************************
* Local Functions
*******************************************************************************/
void sensor_interrupt_handler(void *callback_arg, cyhal_timer_event_t event);
cy_rslt_t sensor_timer_init(void);

/*******************************************************************************
* Function Name: sensor_init
********************************************************************************
* Summary:
*   Initialize the Neural Network based on the magic wand model. Initializes the
*   IMU to collect data, a SPI for IMU communication, a timer, and circular buffer.
*
* Parameters:
* None
*
* Return:
*   The status of the initialization.
*******************************************************************************/
cy_rslt_t sensor_init(void)
{
    cy_rslt_t result;

    /* Initialize the Neural Network */
    result = nn_init(&nn_gesture_obj, (void *) nn_gesture_model.parameters, (void *) nn_gesture_model.weights);
    if(CY_RSLT_SUCCESS != result)
    {
        return result;
    }

    /* Setup the circle buffer for data storage */
    cy_fifo_init_static(&sensor_fifo, sensor_fifo_pool, sizeof(sensor_fifo_pool), SENSOR_FIFO_ITEM_SIZE);

    /* Initialize SPI for IMU communication */
    result = cyhal_spi_init(&spi, CYBSP_SPI_MOSI, CYBSP_SPI_MISO, CYBSP_SPI_CLK, NC, NULL, 8, CYHAL_SPI_MODE_00_MSB, false);
    if(CY_RSLT_SUCCESS != result)
    {
        return result;
    }

    /* Set SPI frequency to 10MHz */
    result = cyhal_spi_set_frequency(&spi, IMU_SPI_FREQUENCY);
    if(CY_RSLT_SUCCESS != result)
    {
        return result;
    }

    /* Initialize the IMU */
    result = mtb_bmx160_init_spi(&sensor_bmx160, &spi, CYBSP_SPI_CS);
    if(CY_RSLT_SUCCESS != result)
    {
        return result;
    }

    /* Timer for data collection */
    sensor_timer_init();

    /* Create an event that will be set when data collection is done */
    cy_rtos_init_event(&sensor_event);

    return CY_RSLT_SUCCESS;
}

/*******************************************************************************
* Function Name: sensor_run
********************************************************************************
* Summary:
*   A task used to pre-process data before it is fed to the inference engine.
*
* Parameters:
*     arg: not used
*
*
*******************************************************************************/
void sensor_task(void *arg)
{
    (void)arg;

    /* Regression pointers */
    NN_IN_Type  *input_reference;

    /* Data processed in floating point */
    float data_feed[SENSOR_BATCH_SIZE][SENSOR_NUM_AXIS];

    /* Initialize butter-worth filter variables */
    int n_order = 3;
    /* Coefficients for 3rd order butter-worth filter */
    const float coeff_b[] = IIR_FILTER_BUTTER_WORTH_COEFF_B;
    const float coeff_a[] = IIR_FILTER_BUTTER_WORTH_COEFF_A;
    iir_filter_struct butter_lp_fil;


    for(;;)
    {
        /* Wait until there is 128 samples from the accelerometer and the
         * gyroscope in the circular buffer */
        cy_rtos_waitbits_event(&sensor_event, &sensor_event_bits, true, true, CY_RTOS_NEVER_TIMEOUT);

        uint16_t cur = 0;

        /* Cast the data from an int16 to a float for pre-processing */
        int16_t temp_buffer[SENSOR_BATCH_SIZE][SENSOR_NUM_AXIS];
        cy_fifo_read(&sensor_fifo, &temp_buffer, SENSOR_BATCH_SIZE * SENSOR_DATA_WIDTH);
        cast_int16_to_float(&temp_buffer[0][0], &data_feed[0][0], SENSOR_BATCH_SIZE*SENSOR_NUM_AXIS);

        /* Third order butter-worth filter */
        while(cur < SENSOR_NUM_AXIS)
        {
            /* Initialize and run the filter */
            iir_filter_init(&butter_lp_fil, coeff_b, coeff_a, n_order);
            iir_filter(&butter_lp_fil, &data_feed[0][0], SENSOR_BATCH_SIZE, cur, SENSOR_NUM_AXIS);
            cur++;
        }

        /* Do a min max normalization to get all data between -1 and 1 */
        normalization_min_max(&data_feed[0][0], SENSOR_BATCH_SIZE, SENSOR_NUM_AXIS, MIN_DATA_SAMPLE, MAX_DATA_SAMPLE);

        /*  */
#if CY_ML_FIXED_POINT_16_IN
        /* Convert to int16 based on the q format */
        float format = QFORMAT_SCALE;
        NN_IN_Type data_feed_int[SENSOR_BATCH_SIZE][SENSOR_NUM_AXIS];
        nn_utils_convert_flt_to_int16(&data_feed[0][0], &data_feed_int[0][0], SENSOR_BATCH_SIZE*SENSOR_NUM_AXIS, format);
#elif CY_ML_FIXED_POINT_8_IN
        /* Convert to int8 based on the q format */
        float format = QFORMAT_SCALE;
        NN_IN_Type data_feed_int[SENSOR_BATCH_SIZE][SENSOR_NUM_AXIS];
        nn_utils_convert_flt_to_int8(&data_feed[0][0], &data_feed_int[0][0], SENSOR_BATCH_SIZE*SENSOR_NUM_AXIS, format);
#endif

        /* Feed the Model */
#if CY_ML_FIXED_POINT
        nn_gesture_obj.fixed_point_q = QFORMAT_VALUE;
        input_reference = (NN_IN_Type *) data_feed_int;
        nn_feed(&nn_gesture_obj, input_reference);
#else
        input_reference = (NN_IN_Type *) data_feed;
        nn_feed(&nn_gesture_obj, input_reference);
#endif


    }
}

/*******************************************************************************
* Function Name: sensor_interrupt_handler
********************************************************************************
* Summary:
*   Reads accelerometer and gyroscope data at 128 HZ. When there are 128 samples
*   an event is set so the data can be processed.
*
* Parameters:
*     callback_arg: not used
*     event: not used
*
*
*******************************************************************************/
void sensor_interrupt_handler(void *callback_arg, cyhal_timer_event_t event)
{
    (void) callback_arg;
    (void) event;
    static int location = 0;

    /* Read data from IMU sensor */
    cy_rslt_t result;
    result = mtb_bmx160_read(&sensor_bmx160, &data);
    if (CY_RSLT_SUCCESS != result)
    {
        CY_ASSERT(0);
    }

    /* Write data to the circle buffer */
    cy_fifo_write(&sensor_fifo, &data.accel, 1);
    cy_fifo_write(&sensor_fifo, &data.gyro, 1);

    /* Once there is enough data to feed the inference, run pre-processing */
    location++;
    if(location == SENSOR_BATCH_SIZE)
    {
        /* Reset the counter */
        location = 0;

        /* Once the event is set the data is processed */
        cy_rtos_setbits_event(&sensor_event, SENSOR_EVENT_BIT, true);
    }
}

/*******************************************************************************
* Function Name: sensor_timer_init
********************************************************************************
* Summary:
*   Sets up an interrupt that triggers at 128Hz.
*
* Parameters:
*     None
*
*
*******************************************************************************/
cy_rslt_t sensor_timer_init(void)
{
    cy_rslt_t rslt;
    const cyhal_timer_cfg_t timer_cfg =
    {
        .compare_value = 0,                 /* Timer compare value, not used */
        .period = SENSOR_TIMER_PERIOD,      /* Defines the timer period */
        .direction = CYHAL_TIMER_DIR_UP,    /* Timer counts up */
        .is_compare = false,                /* Don't use compare mode */
        .is_continuous = true,              /* Run the timer indefinitely */
        .value = 0                          /* Initial value of counter */
    };

    /* Initialize the timer object. Does not use pin output ('pin' is NC) and
     * does not use a pre-configured clock source ('clk' is NULL). */
    rslt = cyhal_timer_init(&sensor_timer, NC, NULL);
    if (CY_RSLT_SUCCESS != rslt)
    {
        return rslt;
    }

    /* Apply timer configuration such as period, count direction, run mode, etc. */
    rslt = cyhal_timer_configure(&sensor_timer, &timer_cfg);
    if (CY_RSLT_SUCCESS != rslt)
    {
        return rslt;
    }

    /* Set the frequency of timer to 100KHz */
    rslt = cyhal_timer_set_frequency(&sensor_timer, SENSOR_TIMER_FREQUENCY);
    if (CY_RSLT_SUCCESS != rslt)
    {
        return rslt;
    }

    /* Assign the ISR to execute on timer interrupt */
    cyhal_timer_register_callback(&sensor_timer, sensor_interrupt_handler, NULL);
    /* Set the event on which timer interrupt occurs and enable it */
    cyhal_timer_enable_event(&sensor_timer, CYHAL_TIMER_IRQ_TERMINAL_COUNT, SENSOR_TIMER_PRIORITY, true);
    /* Start the timer with the configured settings */
    rslt = cyhal_timer_start(&sensor_timer);
    if (CY_RSLT_SUCCESS != rslt)
    {
        return rslt;
    }

    return CY_RSLT_SUCCESS;
}
