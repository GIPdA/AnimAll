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
#include <pthread.h>

#include "scale.h"


/* **************************************************************** *
 *  Local defines
 * **************************************************************** */

#define STATUS_LED_PIN  6   /*! Pin # of status LED */

#define SPI_CHANNEL 0       /*! SPI channel # of ADC converter */
#define SPI_CLK_HZ  1500000 /*! SPI clock in Hz, must be > 10kHz */

#define FILTER_TAP_NUM 128

/* **************************************************************** *
 *  Private variables
 * **************************************************************** */

typedef struct {
  double history[FILTER_TAP_NUM];
  unsigned int last_index;
} Filter;

Filter filter;


/* **************************************************************** *
 *  Private functions
 * **************************************************************** */

static void prv_filter_init(Filter* f);
static void prv_filter_put(Filter* f, double input);
static double prv_filter_get(Filter* f);
static int roundUp(int numToRound, int multiple);


/* **************************************************************** *
 *  Fonctions
 * **************************************************************** */


static PI_THREAD (scaleThread)
{
    unsigned char data[2] = {0};
    int rawValue = 0;

    piHiPri (50);

    for (;;) {
        // Get ADC measurement
        if (wiringPiSPIDataRW(SPI_CHANNEL, &data[0], 2) < 0) {
            // Error occured in SPI transfert
            return false;
        }

        // Compute raw value from ADC
        rawValue = ((data[1]&0xFE) >> 1) | (( (int)data[0]&0x1F) << 7);

        prv_filter_put(&filter, (double)rawValue);

        delayMicroseconds(1000);
    }

    return NULL;
}


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

    prv_filter_init(&filter);

    int res = piThreadCreate(scaleThread);

    if (res < 0) {
        return false;
    }

    return true;
}


static void prv_filter_init(Filter* f)
{
    int i;
    for(i = 0; i < FILTER_TAP_NUM; ++i) {
        f->history[i] = 0;
    }
    f->last_index = 0;
}

static void prv_filter_put(Filter* f, double input)
{
    f->history[(f->last_index++) & (FILTER_TAP_NUM-1)] = input;
}

static double prv_filter_get(Filter* f)
{
    double acc = 0;
    int index = f->last_index, i;

    for(i = 0; i < FILTER_TAP_NUM; ++i) {
        acc += f->history[(index--) & (FILTER_TAP_NUM-1)];// * filter_taps[i];
    }
    acc /= FILTER_TAP_NUM;

    return acc;
}


static int roundUp(int numToRound, int multiple)
{
    if(multiple == 0) {
        return numToRound;
    }

    int remainder = numToRound % multiple;
    if (remainder == 0) {
        return numToRound;
    }

    return numToRound + multiple - remainder;
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
