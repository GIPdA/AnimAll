/*
 * @file scale.cpp
 * @brief Scale controller
 */

#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cerrno>
#include <iostream>
#include <fstream>
//#include <sys/inotify.h>
#include <signal.h>
#include <ctime>
#include <cstring>
#include <sys/types.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <softPwm.h>

using namespace std;


#define STATUS_LED_PIN  6

#define SPI_CHANNEL 0
#define SPI_CLK_HZ  1500000

#define DEFAULT_LOG_FILE_NAME "/home/pi/scale.log"
#define CHOWN_USER "pi"


fstream xCmdFile;
string logFilePath;


void cleanup()
{
    digitalWrite(STATUS_LED_PIN, 0);
    pinMode(STATUS_LED_PIN, INPUT);

    // Close file
    xCmdFile.close();
}


void signalIntHandler(int signum)
{
    cout << "\nCleanup and exit" << endl;
    cleanup();
    exit(signum);
}



void writeDateTimestamp()
{
    // Compute timestamp
    time_t now = time(NULL);
    char *dt = asctime(localtime(&now));
    dt[strlen(dt)-1]='\0';

    // Write new session
    xCmdFile << "<" << dt << ">";
}

void logWeight(int weight)
{
    xCmdFile.open(logFilePath.c_str(), ios::out | ios::app);

    if (xCmdFile.is_open()) {
        // Write weight

        //xCmdFile << weight << " " << writeDateTimestamp();
        xCmdFile << weight << " ";
        writeDateTimestamp();

        xCmdFile.close();
    } else {
        // Unable to write to file !
    }
}

void computeWeight(int *weight, unsigned char *data)
{
    // Compute raw value from ADC
    int rawValue = ((data[1]&0xFE) >> 1) | (( (int)data[0]&0x1F) << 7);

    double voltValue = (double)rawValue * 5.0/4096.0;

    cout << "Raw: " << rawValue << "  = " << voltValue << " V" << endl;

    *weight = rawValue;
}



/*
 *********************************************************************************
 * main
 *********************************************************************************
 */

int main(int argc, char *argv[])
{

    cout << "Starting Distrib..." << endl;

// * Log file *********************************
    if (argc != 2) {
        // No file path specified, set default
        logFilePath = DEFAULT_LOG_FILE_NAME;
        cout << "No log file specified, use default at: " DEFAULT_LOG_FILE_NAME << endl;
    } else {
        cout << "Using log file: " << argv[1] << endl;
        logFilePath = argv[1];
    }

    // Test if file exists and try to create it if not
    xCmdFile.open(logFilePath.c_str(), ios::out | ios::app);

    if (!xCmdFile.is_open()) {
        // Unable to open log file
        cerr << "Unable to write " << logFilePath << endl;
        exit(EXIT_FAILURE);
    }

    // Chown log file to USER
    struct passwd *pwd = getpwnam(CHOWN_USER);
    if (!pwd || chown(logFilePath.c_str(), pwd->pw_uid, pwd->pw_gid) == -1) {
        cout << "chown error: " << logFilePath << endl;
        exit(EXIT_FAILURE);
    }

    // Compute timestamp
    time_t now = time(NULL);
    char *dt = asctime(localtime(&now));
    dt[strlen(dt)-1]='\0';

    // Write new session
    xCmdFile << "<" << dt << ">" << endl;

    xCmdFile.close();
// * Log file *********************************


    // Register signal and signal handler
    signal(SIGINT, signalIntHandler);



// * WiringPi *********************************
    // Setup WiringPi
    if (wiringPiSetup() == -1) {
        // Error while initializing SPI
        cout << "Error while initializing WiringPi (" << errno << ")" << endl;
        exit(EXIT_FAILURE);
    }

    if (wiringPiSPISetup(SPI_CHANNEL, SPI_CLK_HZ) == -1) {
        // Error while initializing SPI
        cout << "Error while initializing SPI (" << errno << ")" << endl;
        exit(EXIT_FAILURE);
    }

    // Setup I/O
    
    // Setup LED
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, 0);

    // Setup ISR

// * WiringPi *********************************


    cout << "Distrib setup done. Waiting..." << endl;

    unsigned char dataBuffer[2] = {0};
    int weight = 0;

    // Loop, measure values
    for (;;) {
        
        xCmdFile.open(logFilePath.c_str(), ios::out | ios::app);

        if (xCmdFile.is_open()) {
            // Turn on status led
            digitalWrite(STATUS_LED_PIN, 1);

            // Get measure
            wiringPiSPIDataRW(SPI_CHANNEL, &dataBuffer[0], 2);

            //cout << "Buffer: " << (int)dataBuffer[0] << ", " << (int)dataBuffer[1] << endl;

            computeWeight(&weight, &dataBuffer[0]);

            // Log weight with timestamp
            logWeight(weight);

            xCmdFile.close();

            // Turn off status led
            digitalWrite(STATUS_LED_PIN, 0);
        } else {
            // Unable to write to file !
        }

        sleep(1);
    }

    cleanup();

    return 0 ;
}