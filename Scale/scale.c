/**
 * @file scale.c
 *
 * @brief Scale controller
 *
 * @author Benjamin Balga
 */


#include <stdlib.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <softPwm.h>

#include "scale.h"


/* **************************************************************** *
 *  Local defines
 * **************************************************************** */

#define STATUS_LED_PIN  6   /*! Pin # of status LED */

#define SPI_CHANNEL 0       /*! SPI channel # of ADC converter */
#define SPI_CLK_HZ  1500000 /*! SPI clock in Hz, must be > 10kHz */


/* **************************************************************** *
 *  Private variables
 * **************************************************************** */


/* **************************************************************** *
 *  Private functions
 * **************************************************************** */


/* **************************************************************** *
 *  Fonctions
 * **************************************************************** */

bool bScale_setup()
{
    // Setup WiringPi
    if (wiringPiSetup() == -1) {
        // Error occured
        return false;
    }

    if (wiringPiSPISetup(SPI_CHANNEL, SPI_CLK_HZ) == -1) {
        // Error while initializing SPI
        return false;
    }

    // Setup LED to 1Hz
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, 0);

    return true;
}



bool bScale_getWeight(int * const iMeasuredWeight)
{
    unsigned char data[2] = {0};
    int weight = 0;

    if (!iMeasuredWeight) return false;

    // Turn on status led to indicate activity
    digitalWrite(STATUS_LED_PIN, 1);

    // Get ADC measurement
    if (wiringPiSPIDataRW(SPI_CHANNEL, &data[0], 2) < 0) {
        // Error occured in SPI transfert
        return false;
    }

    //cout << "Buffer: " << (int)dataBuffer[0] << ", " << (int)dataBuffer[1] << endl;

    // Compute raw value from ADC
    int rawValue = ((data[1]&0xFE) >> 1) | (( (int)data[0]&0x1F) << 7);

    // Compute voltage from raw
    double voltValue = (double)rawValue * 5.0/4096.0;

    cout << "Raw: " << rawValue << "  = " << voltValue << " V" << endl;

    // Compute weight
    weight = voltValue;

    // Turn off status led
    digitalWrite(STATUS_LED_PIN, 0);

    *iMeasuredWeight = weight;

    return true;
}



void vScale_cleanup()
{
    digitalWrite(STATUS_LED_PIN, 0);
    pinMode(STATUS_LED_PIN, INPUT);
}
