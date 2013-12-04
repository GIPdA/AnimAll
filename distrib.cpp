/*
 * @file distrib.cpp
 * @brief Distrib controller
 */

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sys/inotify.h>
#include <signal.h>
#include <wiringPi.h>
#include <softPwm.h>

using namespace std;


#define MOTOR_PIN   0
#define SWITCH_PIN  7
#define START_MOTOR_PIN 2
#define STATUS_LED_PIN  3

#define CMD_FILE_NAME "/home/pi/distributeur/distrib.ctl"


enum DistribState { Running, Waiting };

static DistribState eDistribState = Waiting;

fstream xCmdFile;
int fd, wd;


void cleanup()
{
    digitalWrite(MOTOR_PIN, 0);
    pinMode(MOTOR_PIN, INPUT);
    digitalWrite(STATUS_LED_PIN, 0);
    pinMode(STATUS_LED_PIN, INPUT);

    // Removing the directory from the watch list.
    inotify_rm_watch(fd, wd);

    // Closing the INOTIFY instance
    close(fd);

    xCmdFile.close();
}


void signalIntHandler(int signum)
{
    cout << "\nCleanup and exit" << endl;
    cleanup();
    exit(signum);
}




void startMotor()
{
    eDistribState = Running;

    softPwmWrite(STATUS_LED_PIN, 8000);
    //softPwmCreate(STATUS_LED_PIN, 1000, 2000);

    digitalWrite(MOTOR_PIN, 1);
}

void stopMotor()
{
    digitalWrite(MOTOR_PIN, 0);

    eDistribState = Waiting;

    softPwmWrite(STATUS_LED_PIN, 1000);
    //softPwmCreate(STATUS_LED_PIN, 1000, 10000);
}


void switchInterrupt()
{
    stopMotor();
}

void startInterrupt()
{
    startMotor();
}


bool processCommand(string cmdStr)
{
    cout << "Process " << cmdStr << endl;
    if (cmdStr.find("start") == 0)
    {
        if (eDistribState == Running) {
            // Motor is running, discard commands
            xCmdFile << "<run>\n";
            return false;
        }
        cout << "Starting motor" << endl;
        startMotor();
        xCmdFile << "<ok>\n";
        return true;
    }

    if (cmdStr.find("stop") == 0)
    {
        cout << "Stopping motor" << endl;
        stopMotor();
        xCmdFile << "<ok>\n";
        return true;
    }

    xCmdFile << "<err>\n";
    return false;
}



/*
 *********************************************************************************
 * main
 *********************************************************************************
 */

int main (void)
{
    int length;
    struct inotify_event inotifyEvt;

    cout << "Starting Distrib... ";


    // Register signal and signal handler
    signal(SIGINT, signalIntHandler);



// * inotify *********************************
    // Creating the INOTIFY instance
    fd = inotify_init();

    if (fd < 0) {
        cerr << "Unable to init inotify" << endl;
        cout << "failed." << endl;
        exit(EXIT_FAILURE);
    }

    // Adding the command file into watch list.
    // Watch for closing writes
    wd = inotify_add_watch(fd, CMD_FILE_NAME, IN_MODIFY | IN_CLOSE_WRITE);

    if (wd < 0) {
        cerr << "inotify watch failed" << endl;
        cout << "failed." << endl;
        close(fd);
        exit(EXIT_FAILURE);
    }
// * inotify *********************************


// * WiringPi *********************************
    // Setup WiringPi
    wiringPiSetup();

    // Setup I/O
    pinMode(MOTOR_PIN, OUTPUT);
    pinMode(SWITCH_PIN, INPUT);
    pullUpDnControl(SWITCH_PIN, PUD_UP);
    pinMode(START_MOTOR_PIN, INPUT);
    pullUpDnControl(START_MOTOR_PIN, PUD_UP);
    
    // Setup LED to 1Hz
    pinMode(STATUS_LED_PIN, OUTPUT);
    softPwmCreate(STATUS_LED_PIN, 1000, 10000);

    // Setup ISR
    wiringPiISR(SWITCH_PIN, INT_EDGE_RISING, &switchInterrupt);
    wiringPiISR(START_MOTOR_PIN, INT_EDGE_RISING, &startInterrupt);
// * WiringPi *********************************


    cout << "ok." << endl;



    
    unsigned char was_modified = 0, skip_next = 0;
    int last_pos;
    string line, line_temp;

    // Loop, wait for new line in file
    for (;;)
    {
        //cout << "Wait..." << endl;

        // Wait for file change
        length = read(fd, &inotifyEvt, sizeof(inotifyEvt)); 

        // Check for error
        if (length < 0) {
            cerr << "inotify read error" << endl;
        }
        
        //cout << "Event " << (inotifyEvt.mask) << endl;


        if (    (inotifyEvt.mask & IN_MODIFY)
            && !(inotifyEvt.mask & IN_ISDIR) )
        {
            was_modified = !skip_next;
            skip_next = 0;
        }


        // Check event: should be IN_CLOSE_WRITE and not a dir
        if (    (inotifyEvt.mask & IN_CLOSE_WRITE) 
            && was_modified
            && !(inotifyEvt.mask & IN_ISDIR))
        {
            // Get lines
            was_modified = 0;

            xCmdFile.open(CMD_FILE_NAME, ios::in | ios::out);

            if (xCmdFile.is_open())
            {
                // Go to last pos (get)
                xCmdFile.seekg(last_pos);

                // Get last line
                do {
                    line = line_temp;
                    getline(xCmdFile, line_temp);
                } while (!line_temp.empty());


                // Seek to end of line before \n
                //xCmdFile.seekp((int)xCmdFile.tellg() - 1);
                xCmdFile.seekp(-1, ios::end);

                // Process command
                processCommand(line);


                // Update last pos
                xCmdFile.seekg(0, ios::end);
                last_pos = xCmdFile.tellg();

                skip_next = 1;

                xCmdFile.close();
            }
        }
    }

    cleanup();

    return 0 ;
}