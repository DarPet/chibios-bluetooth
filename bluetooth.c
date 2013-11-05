#include "bluetooth.h"
#include "ch.h"
#include "hal.h"

static bluetoothConfig myBluetoothConfig ={
DEFAULT_BT_BAUD,
DEFAULT_BT_PARITY,
DEFAULT_BT_STOP,
DEFAULT_BT_NAME,
DEFAULT_BT_MODE,
&SD2
};

void btReset(){
    // pin must be low for at least 5ms to cause a reset
    // RESET PIN (HC-05 pin 11) is connected to Discovery board GPIOD PIN2 (PD2)
    palClearPad(GPIOD, 2);
    chThdSleepMilliseconds(10);
    palSetPad(GPIOD, 2);
};

void btSetCommandMode(btCommandMode commandMode){
    // sets the bluetooth module to the given working mode
    // KEY PIN (HC-05 pin 34) is connected to Discovery board GPIOD PIN1 (PD1)
    switch (commandMode){
    case atMode:
	// stop current bluetooth communication

        // supply power to pin 34, then supply power to the bluetooth module (reset)
        palSetPad(GPIOD, 1);
        chThdSleepMilliseconds(10);
        btReset();
	//now we must communicate using a 38400 baud rate


	//when finish, we exit
	break;
    }

};

void btStart();

void btStop();

void btPut();

char btGet();

void btConfigInit();
