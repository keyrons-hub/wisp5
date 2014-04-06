/**
 * @file accel.c
 *
 * Driver for the ADXL362 accelerometer
 *
 * @author Aaron Parks
 * @date Aug 2013
 */
#include "accel.h"
#include "accel_registers.h"
#include "../globals.h"
#include "../timing/timer.h"
#include "../wired/spi.h"

///////////////////////////////////////////////////////////////////////////////

// TODO Translate these to a better format once SPI driver is fleshed out. Make them CONST.
uint8_t const ADXL_READ_PARTID[] = {ADXL_CMD_READ_REG,ADXL_REG_PARTID,0x00};
uint8_t const ADXL_READ_DEVID[] = {ADXL_CMD_READ_REG,ADXL_REG_DEVID_AD,0x00};
uint8_t const ADXL_REAsxD_STATUS[] = {ADXL_CMD_READ_REG,ADXL_REG_STATUS,0x00};
uint8_t const ADXL_READ_XYZ_8BIT[] = {ADXL_CMD_READ_REG,ADXL_REG_XDATA,0x00,0x00,0x00};
uint8_t const ADXL_READ_XYZ_16BIT[] = {ADXL_CMD_READ_REG,ADXL_REG_XDATA_L,0x00,0x00,0x00,0x00,0x00,0x00};
uint8_t const ADXL_CONFIG_MEAS[] = {ADXL_CMD_WRITE_REG,ADXL_REG_POWER_CTL,0x02}; // Put the ADXL into measurement mode
uint8_t const ADXL_CONFIG_STBY[] = {ADXL_CMD_WRITE_REG,ADXL_REG_POWER_CTL,0x00}; // Put the ADXL into standby mode
///////////////////////////////////////////////////////////////////////////////

/**
 * Turn on and start up the ADXL362 accelerometer. This leaves the ADXL running.
 */
BOOL ACCEL_initialize() {

    // TODO Figure out optimal ADXL configuration for single measurement
    while(!SPI_acquirePort());
    BITSET(POUT_ACCEL_EN, PIN_ACCEL_EN);
    BITSET(POUT_ACCEL_CS, PIN_ACCEL_CS);

    //TODO find the proper length of delay
    __delay_cycles(100);
    BITCLR(POUT_ACCEL_CS, PIN_ACCEL_CS);
    __delay_cycles(20);
    SPI_transaction(gpRxBuf, (uint8_t*)ADXL_CONFIG_MEAS, sizeof(ADXL_CONFIG_MEAS));
    BITSET(POUT_ACCEL_CS, PIN_ACCEL_CS);

    SPI_releasePort();

    //Timer_LooseDelay(LP_LSDLY_200MS);// To let ADXL start measuring? How much time is actually required here, if any?

    // TODO Use a pin interrupt to let us know when first measurement is ready
    return SUCCESS;
}

/**
 * Put the ADXL362 into a lower power standby state without gating power
 *
 * @todo Implement this function
 */
void ACCEL_standby() {


}

/**
 * Grab one sample from the ADXL362 accelerometer
 */
BOOL ACCEL_singleSample(threeAxis_t* result) {

    while(!SPI_acquirePort());

    BITCLR(POUT_ACCEL_CS, PIN_ACCEL_CS);
    __delay_cycles(20);
    SPI_transaction(gpRxBuf, (uint8_t*)ADXL_READ_XYZ_16BIT, sizeof(ADXL_READ_XYZ_16BIT));
    BITSET(POUT_ACCEL_CS, PIN_ACCEL_CS);

    SPI_releasePort();

    result->x = gpRxBuf[2] + __swap_bytes(gpRxBuf[3]);
    result->y = gpRxBuf[4] + __swap_bytes(gpRxBuf[5]);
    result->z = gpRxBuf[6] + __swap_bytes(gpRxBuf[7]);

    return SUCCESS;
}