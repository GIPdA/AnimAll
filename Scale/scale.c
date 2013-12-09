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

#define DEFAULT_WEIGHT_MULTIPLE (700.0/(2133.0-1140.0))

#define STATUS_LED_PIN  6   /*! Pin # of status LED */

#define SPI_CHANNEL 0       /*! SPI channel # of ADC converter */
#define SPI_CLK_HZ  1500000 /*! SPI clock in Hz, must be > 10kHz */

#define FILTER_TAP_NUM 128

#define SCALE_PRECISION 5   /*! in gr */

/* **************************************************************** *
 *  Private variables
 * **************************************************************** */

typedef struct {
  double history[FILTER_TAP_NUM];
  unsigned int last_index;
} Filter;

static Filter filter;

static double prv_rawToWeightMultiple = 1.0;
static double prv_referenceValue = 0.0;
static unsigned char prv_wasTare = 0;


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

    prv_rawToWeightMultiple = DEFAULT_WEIGHT_MULTIPLE;

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


void vScale_tareScale()
{
    prv_referenceValue = prv_filter_get(&filter);
    prv_wasTare = 1;
}



bool bScale_getWeight(int * const iMeasuredWeight)
{
    if (!iMeasuredWeight) return false;

    // Turn on status led to indicate activity
    digitalWrite(STATUS_LED_PIN, 1);

    double v = prv_filter_get(&filter);

    if (!prv_wasTare) {
        prv_wasTare = 1;
        vScale_tareScale();
    }

    // Compute weight
    int weight = roundUp((int)((v-prv_referenceValue)*prv_rawToWeightMultiple), SCALE_PRECISION);

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
