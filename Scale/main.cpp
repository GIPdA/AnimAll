/*
 * @file main.cpp
 * @brief Scale controller
 */


#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
#include <csignal>

#include "scale.h"


using namespace std;


void signalIntHandler(int signum)
{
    cout << "\nCleanup and exit" << endl;
    vScale_cleanup();
    exit(signum);
}



int main(int argc, char *argv[])
{
    int weight = 0;

    // Register signal and signal handler
    signal(SIGINT, signalIntHandler);

    // Setup Feeder
    if (!bScale_setup()) {
    	cout << "Error on Scale setup. Exit." << endl;
    	exit(EXIT_FAILURE);
    }


    cout << "Scale started." << endl;

    while (1) {
    	sleep(1);

        if (bScale_getWeight(&weight)) {
            cout << "Weight: " << weight << "g" << endl;
        } else {
            cout << "Reading error !" << endl;
        }
    }

    return 0 ;
}
