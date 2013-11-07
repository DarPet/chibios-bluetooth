#include "bluetooth.h"
#include "ch.h"
#include "hal.h"
#include <serial_lld.h>

static bluetoothConfig myBluetoothConfig ={
DEFAULT_BT_BAUD,
DEFAULT_BT_PARITY,
DEFAULT_BT_STOP,
DEFAULT_BT_NAME,
DEFAULT_BT_MODE,
&SD2
};

static const uint8_t btAtTerminationString[] = "\r\n";

static SerialConfig btAtSerialDriverConfig ={
9600,
};

static SerialConfig btCommSerialDriverConfig ={
9600,
};

void btReset(){
    // pin must be low for at least 5ms to cause a reset
    // RESET PIN (HC-05 pin 11) is connected to Discovery board GPIOD PIN2 (PD2)
    palClearPad(GPIOD, 2);

    palSetPad(GPIOD, GPIOD_LED3);
    chThdSleepMilliseconds(500);

    palSetPad(GPIOD, 2);

    palClearPad(GPIOD, GPIOD_LED3);
};

void btSetCommandMode(btCommandMode commandMode){
    // sets the bluetooth module to the given working mode
    // KEY PIN (HC-05 pin 34) is connected to Discovery board GPIOD PIN1 (PD1)
    switch (commandMode){
    case atMode:
	// stop current bluetooth communication
        btStop();
        // supply power to pin 34, then supply power to the bluetooth module (reset)
        palSetPad(GPIOD, 1);
        chThdSleepMilliseconds(100);
        btReset();
        chThdSleepMilliseconds(100);
	//now we must communicate using a 38400 baud rate
    //initialize the serial link of the module
        btStartInAt();
	//when finish, we exit
        break;
    case commMode:
        // ?? we could jump from the at mode to here using an at command to be ready for connections

        // stop current bluetooth communication
        btStop();
        // set pin 34 low, then supply power to the bluetooth module (reset)
        palSetPad(GPIOD, 0);
        chThdSleepMilliseconds(100);
        btReset();
        chThdSleepMilliseconds(100);
	//now we must communicate using the configured baud rate
    //initialize the serial link of the module
        btStart();
	//when finish, we exit
        break;
    default:
        //it is not a good thing if we get here
        break;
    };

};

void btStart()
{
    sdStart(&SD2, &btCommSerialDriverConfig);
};

void btStartInAt(){

    sdStart(&SD2, &btAtSerialDriverConfig);

};


void btStop()
{
    sdStop(myBluetoothConfig.sdp);
};

void btPut();

char btGet();

void btConfigInit();


//at commands

void btAtGetBaud();

void btAtSetBaud();

void btAtGetName();

void btAtSetName(const uint8_t *newName, size_t newNameLength){
    static const uint8_t btStringRestoreDefaults[] = "AT+NAME=";
    //Send the commands first part
    sdWrite(&SD2, &btStringRestoreDefaults[0], 7);
    //Now add the new name
    sdWrite(&SD2, newName, newNameLength);
    //And finally write the termination \r\n
    sdWrite(&SD2, &btAtTerminationString[0], 2);

};

void btAtResetDefaults(){
    static const uint8_t btStringRestoreDefaults[] = "AT+ORGL\r\n";

    sdWrite(&SD2, &btStringRestoreDefaults[0], 9);
};
