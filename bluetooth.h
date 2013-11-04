#ifndef BLUETOOTH_H_INCLUDED
#define BLUETOOTH_H_INCLUDED

#include "hal.h"


#define DEFAULT_BT_BAUD 38400
#define DEFAULT_BT_PARITY none
#define DEFAULT_BT_STOP 1
#define DEFAULT_BT_NAME chibiblue
#define DEFAULT_BT_MODE slave
#define MAX_NAME_SIZE 32
//#define DEFAULT_BT_RESET_PAD GPIOD_PIN2
//#define DEFAULT_BT_RESET_PORT GPIOD
//#define DEFAULT_BT_KEY_PAD GPIOD_PIN1
//#define DEFAULT_BT_KEY_PORT GPIOD

typedef enum bt_modes{
    slave = 0,
    master = 1,
}bt_modes;

typedef enum btparity{
    even = 0,
    mark,
    none,
    odd,
    space,
}btparity;

typedef struct bluetoothConfig{

    int baud;
    btparity parity;
    int stop;
    char name[MAX_NAME_SIZE];
    bt_modes mode;
    SerialDriver *sdp;
}bluetoothConfig;

void btReset(bluetoothConfig *btconf);

void btSetMode(bluetoothConfig *btconf);

void btStart(bluetoothConfig *btconf);

void btStop(bluetoothConfig *btConf);

void btPut(bluetoothConfig *btConf);

char btGet(bluetoothConfig *btConf);






#endif // BLUETOOTH_H_INCLUDED
