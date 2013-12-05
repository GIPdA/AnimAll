
#ifndef _FEEDER_H_
#define _FEEDER_H_

#define MOTOR_PIN   	0
#define SWITCH_PIN  	7
#define START_MOTOR_PIN 2
#define STATUS_LED_PIN  3


bool bFeeder_setup();

unsigned int uiFeeder_startCycle();
void vFeeder_stop();

void vFeeder_cleanup();


#endif _FEEDER_H_
