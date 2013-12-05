/*
 * @file main.cpp
 * @brief Feeder controller
 */


#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
#include <csignal>

#include "feeder.h"


using namespace std;


void signalIntHandler(int signum)
{
    cout << "\nCleanup and exit" << endl;
    vFeeder_cleanup();
    exit(signum);
}



int main(int argc, char *argv[])
{

    // Register signal and signal handler
    signal(SIGINT, signalIntHandler);

    // Setup Feeder
    if (!bFeeder_setup()) {
    	cout << "Error on Feeder setup. Exit." << endl;
    	exit(EXIT_FAILURE);
    }


    cout << "Feeder started." << endl;

    cout << "Start cycle..." << endl;
    uiFeeder_startCycle();

    while (1) {
    	sleep(10);

    	cout << "Start cycle..." << endl;
    	uiFeeder_startCycle();
    }

    return 0 ;
}
