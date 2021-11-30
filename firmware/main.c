#include <string.h>
#include "nrf.h"
#include "nrf_timer.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_drv_spi.h"
#include "app_util_platform.h"
#include "app_error.h"

#define PIN_SPI_SS          30
#define PIN_SPI_MISO        27
#define PIN_SPI_MOSI        28
#define PIN_SPI_SCK         29

#define LSM6DSM_WRITE_MASK          0x7F
#define LSM6DSM_READ_MASK           0x80

/* ----- LSM6DSM Register Addresses ---- */
#define LSM6DSM_REG_CTRL1_XL    0x10
#define LSM6DSM_REG_CTRL2_G     0x11
#define LSM6DSM_REG_CTRL3_C     0x12
#define LSM6DSM_REG_CTRL4_C     0x13
#define LSM6DSM_REG_CTRL5_C     0x14
#define LSM6DSM_REG_CTRL6_C     0x15
#define LSM6DSM_REG_CTRL7_G     0x16
#define LSM6DSM_REG_CTRL8_XL    0x17
#define LSM6DSM_REG_CTRL9_XL    0x18
#define LSM6DSM_REG_CTRL10_C    0x19

#define LSM6DSM_REG_OUT_TEMP_L  0x20
#define LSM6DSM_REG_OUT_TEMP_H  0x21
#define LSM6DSM_REG_OUTX_L_G    0x22
#define LSM6DSM_REG_OUTX_H_G    0x23
#define LSM6DSM_REG_OUTY_L_G    0x24
#define LSM6DSM_REG_OUTY_H_G    0x25
#define LSM6DSM_REG_OUTZ_L_G    0x26
#define LSM6DSM_REG_OUTZ_H_G    0x27
#define LSM6DSM_REG_OUTX_L_XL   0x28
#define LSM6DSM_REG_OUTX_H_XL   0x29
#define LSM6DSM_REG_OUTY_L_XL   0x2A
#define LSM6DSM_REG_OUTY_H_XL   0x2B
#define LSM6DSM_REG_OUTZ_L_XL   0x2C
#define LSM6DSM_REG_OUTZ_H_XL   0x2D

#define MAX_BUF_LEN     10

#define SPI_INSTANCE  0 /**< SPI instance index. */
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */
static volatile bool sample_timeout = false;  /**< Flags when sampling timer timeout*/

typedef struct
{
    uint16_t x;
    uint16_t y;
    uint16_t z;
} sensor_data_t;

uint8_t LSM6DSM_write (uint8_t addr, uint8_t * data, uint8_t len)
{
    uint8_t err_code = 0;
    uint8_t tx_buf[MAX_BUF_LEN];
    uint8_t rx_buf[MAX_BUF_LEN];
    memset(tx_buf, 0, MAX_BUF_LEN);
    memset(rx_buf, 0, MAX_BUF_LEN);

    if (MAX_BUF_LEN - 1 < len)
    {
        err_code = 1;
    }
    else
    {
        tx_buf[0] = addr & LSM6DSM_WRITE_MASK;
        memcpy(&tx_buf[1], data, len);

        APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, tx_buf, len, rx_buf, 0));
    }

    return err_code;
}

uint8_t LSM6DSM_read (uint8_t addr, uint8_t * data, uint8_t len)
{
    uint8_t err_code = 0;
    uint8_t tx_buf[MAX_BUF_LEN];
    uint8_t rx_buf[MAX_BUF_LEN];
    memset(tx_buf, 0, MAX_BUF_LEN);
    memset(rx_buf, 0, MAX_BUF_LEN);

    if (MAX_BUF_LEN - 1 < len)
    {
        err_code = 1;
    }
    else
    {
        tx_buf[0] = addr | LSM6DSM_READ_MASK;

        APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, tx_buf, 1, rx_buf, len));
    }

    return err_code;
}

void LSM6DSM_settings (void)
{
    // set accelerometer output data rate to 833Hz
    // set accelerometer range to +-16g
    uint8_t accelerometer_settings = 0b01110100; // 0111 for 833hz, 01 for +-16g,
    LSM6DSM_write(LSM6DSM_REG_CTRL1_XL, &accelerometer_settings, 1);

    // set gyroscope output data rate to  833Hz
    // set gyroscope range to +-2000dps
    uint8_t gyroscope_settings = 0b01111100; // 0111 for 833hz, 11 for 2000dps
    LSM6DSM_write(LSM6DSM_REG_CTRL2_G, &gyroscope_settings, 1);

    // disable i2c
    uint8_t control_settings_4 = 0b00000100;
    LSM6DSM_write(LSM6DSM_REG_CTRL4_C, &control_settings_4, 1);
}

uint16_t LSM6DSM_get_output (uint8_t addr)
{
    uint8_t data_buf[2] = { 0x00, 0x00 };
    uint16_t data = 0x0000;
    LSM6DSM_read(addr, &data_buf[0], 2);
    data |= data_buf[1];
    data = (data << 8) | data_buf[0];
    return data;
}

sensor_data_t LSM6DSM_get_gyroscope_output (void)
{
    sensor_data_t data;
    data.x = LSM6DSM_get_output(LSM6DSM_REG_OUTX_L_G);
    data.y = LSM6DSM_get_output(LSM6DSM_REG_OUTY_L_G);
    data.z = LSM6DSM_get_output(LSM6DSM_REG_OUTZ_L_G);
    return data;
}

sensor_data_t LSM6DSM_get_accelerometer_output (void)
{
    sensor_data_t data;
    data.x = LSM6DSM_get_output(LSM6DSM_REG_OUTX_L_XL);
    data.y = LSM6DSM_get_output(LSM6DSM_REG_OUTY_L_XL);
    data.z = LSM6DSM_get_output(LSM6DSM_REG_OUTZ_L_XL);
    return data;
}

void log_info_sensor_data (sensor_data_t data)
{
    uint8_t buf[6] = {
        (data.x >> 8) & 0x00FF, data.x & 0x00FF,
        (data.y >> 8) & 0x00FF, data.y & 0x00FF,
        (data.z >> 8) & 0x00FF, data.z & 0x00FF,
    };
    NRF_LOG_HEXDUMP_INFO(buf, 6);
}

void TIMER2_IRQHandler(void) {
    if ( (NRF_TIMER2->INTENSET & NRF_TIMER_INT_COMPARE0_MASK)
         && (NRF_TIMER2->EVENTS_COMPARE[0] != 0)
       ) {
        NRF_TIMER2->EVENTS_COMPARE[0] = 0;
        NRF_TIMER2->TASKS_CLEAR = 1; /* clear counter */

        sample_timeout = true;
    }
}

int main(void)
{
    sensor_data_t gyroscope_output = { .x = 0, .y = 0, .z = 0 };
    sensor_data_t accelerometer_output = { .x = 0, .y = 0, .z = 0 };

    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    // enable spi
    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    spi_config.ss_pin   = PIN_SPI_SS;
    spi_config.miso_pin = PIN_SPI_MISO;
    spi_config.mosi_pin = PIN_SPI_MOSI;
    spi_config.sck_pin  = PIN_SPI_SCK;
    APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, NULL, NULL));

    // configure sampling timer
    NRF_TIMER2->MODE = NRF_TIMER_MODE_TIMER;
    NRF_TIMER2->TASKS_CLEAR = 1;
    NRF_TIMER2->PRESCALER = 4; /* div by 2^x； = 1 MHz */
    NRF_TIMER2->BITMODE = NRF_TIMER_BIT_WIDTH_16;
    NRF_TIMER2->CC[0] = 833; /* 833 Hz */
    NRF_TIMER2->INTENSET = NRF_TIMER_INT_COMPARE0_MASK;
    NVIC_SetPriority(TIMER2_IRQn, 3);
    NVIC_EnableIRQ(TIMER2_IRQn);

    // set LSM6DSM
    LSM6DSM_settings();

    // enable sampling timer
    sample_timeout = false;
    NRF_TIMER2->TASKS_CLEAR = 1;
	NRF_TIMER2->TASKS_START = 1;

    NRF_LOG_INFO("LSM6DSM sensor started.");

    while (1)
    {
        if (sample_timeout)
        {
            gyroscope_output = LSM6DSM_get_gyroscope_output();
            accelerometer_output = LSM6DSM_get_accelerometer_output();

            NRF_LOG_INFO("Gyroscope: ");
            log_info_sensor_data(gyroscope_output);

            NRF_LOG_INFO("Accelerometer: ");
            log_info_sensor_data(accelerometer_output);

            sample_timeout = false;
        }

        NRF_LOG_FLUSH();
    }
}
