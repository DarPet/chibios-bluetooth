#include "bluetooth.h"
#include "ch.h"
#include "hal.h"


void btReset(bluetoothConfig *btconf)
{
    //pin must be low for at least 5ms to cause a reset
    palClearPad(GPIOD, 1);
    chThdSleepMilliseconds(10);
    palSetPad(GPIOD, 1);
};

void btSetMode(bluetoothConfig *btconf);

void btStart(bluetoothConfig *btconf);

void btStop(bluetoothConfig *btConf);

void btPut(bluetoothConfig *btConf);

char btGet(bluetoothConfig *btConf);
