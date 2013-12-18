#include <stdlib.h>
#include <stdint.h>
#include <sys/queue.h>
#include <ch.h>
#include <hal.h>
#include <chthreads.h>
#include <chmemcore.h>
#include <string.h>
#include <btmodule.h>
#include <serial.h>
#include "chprintf.h"
#include "shell.h"
#include "lis302dl.h"

#include "usbcfg.h"


#define HAL_USE_BLUETOOTH



#if defined(HAL_USE_BLUETOOTH) || defined(__DOXYGEN__)

extern SerialUSBDriver SDU1;

static Mutex btCommandMutex;

static const char btATCommandTermination[] = {'\r','\n'};

const struct BluetoothDeviceVMT bluetoothDeviceVMT = {
    .btStart=btStart,
    .btStartReceive=btStartReceive,
    .btStopReceive=btStopReceive,
    .btSendAtCommand=btSendAtCommand,
    .btSetModeAt=btSetModeAt,
    .btSetModeComm=btSetModeComm,
};

static SerialConfig btDefaultSerialConfigAT = { 38400,0,0,0 };

static WORKING_AREA(btSendThreadWa, 128);
static msg_t btSendThread(void *instance) {

    if (!instance)
        chThdSleep(TIME_INFINITE);

    BluetoothDriver *drv = (BluetoothDriver *) instance;

    chRegSetThreadName("btSendThread");

    while (TRUE) {
        if ( !chIQIsEmptyI(drv->bluetoothConfig->btInputQueue) ){

            chnPutTimeout((BaseChannel *)drv->serialDriver, chIQGetTimeout(drv->bluetoothConfig->btInputQueue, TIME_IMMEDIATE), TIME_INFINITE);
        }
        chThdSleepMilliseconds(drv->bluetoothConfig->commSleepTimeMs);
      }

  return (msg_t) 0;
}

static WORKING_AREA(btRecieveThreadWa, 128);
static msg_t btRecieveThread(void *instance) {

    if (!instance)
        chThdSleep(TIME_INFINITE);

    BluetoothDriver *drv = (BluetoothDriver *) instance;

    (void)instance;
    chRegSetThreadName("btRecieveThread");

    while (TRUE) {
        if ( !chOQIsFullI(drv->bluetoothConfig->btOutputQueue) ){

            chOQPut(drv->bluetoothConfig->btOutputQueue, chnGetTimeout((BaseChannel *)drv->serialDriver, TIME_INFINITE));
        }
        chThdSleepMilliseconds(drv->bluetoothConfig->commSleepTimeMs);
    }


  return (msg_t) 0;
}

void btInit(void *instance, BluetoothConfig *config){


    BluetoothDriver *drv = (BluetoothDriver *) instance;
    //null pointer check
    if (!drv || !config)
        return;

    chMtxInit(&btCommandMutex);

    palSetPadMode(BT_MODE_KEY_PORT, BT_MODE_KEY_PIN, PAL_MODE_OUTPUT_PUSHPULL
                | PAL_STM32_OSPEED_HIGHEST);
    palSetPadMode(BT_RESET_PORT, BT_RESET_PIN, PAL_MODE_OUTPUT_PUSHPULL
                | PAL_STM32_OSPEED_HIGHEST);



    drv->bluetoothConfig = config;
    drv->serialDriver = (config->btSerialDriver != NULL) ? config->btSerialDriver : BT_DEFAULT_SERIAL_DRIVER_ADDRESS;
    drv->serialConfig = (config->btSerialConfig != NULL) ? config->btSerialConfig : &btDefaultSerialConfigAT;
    drv->vmt = &bluetoothDeviceVMT;

    //create driverThread, but do not start it yet
    drv->sendThread=chThdCreateI(btSendThreadWa, sizeof(btSendThreadWa), NORMALPRIO, btSendThread, drv);
    drv->recieveThread=chThdCreateI(btRecieveThreadWa, sizeof(btRecieveThreadWa), NORMALPRIO, btRecieveThread, drv);


};

void btStart(void *instance){


    if(!instance)
        return;

    BluetoothDriver *drv = (BluetoothDriver *) instance;

    /**
    TODO: Serial driver defines
    */
    palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(7));
    palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(7));
    sdStart(drv->serialDriver, &btDefaultSerialConfigAT);

    chThdSleepMilliseconds(5000);

    //switch to AT mode
    drv->vmt->btSetModeAt(drv, 5000);
    //start the serial driver, then configure the module



    //we should do the predefined module configuration here, eg.: name, communication baud rate, PIN code, etc.

    BT_SET_NAME(drv,"hapci1");

    BT_RESET(drv);
    BT_SET_PASSKEY(drv, "4321");

    //here we should switch to communications mode and be ready for connections

    drv->vmt->btSetModeComm(drv, 5000);

    btStartReceive(drv);

};



//start communication
void btStartReceive(void *instance){

    BluetoothDriver *drv = (BluetoothDriver *) instance;

    chThdResume(drv->sendThread);
    chThdResume(drv->recieveThread);

    return;
};

// stop communication
void btStopReceive(void *instance){

    BluetoothDriver *drv = (BluetoothDriver *) instance;

    chThdSleep(TIME_INFINITE);

    return;
};



void btSendAtCommand(void *instance, char *command){

    if ( !instance || !command )
        return;


    int commandLen = strlen(command);


    if( commandLen > BT_MAX_COMMAND_SIZE )
        return;


    BluetoothDriver *drv = (BluetoothDriver *) instance;


    if(drv->currentWorkMode != atMode)
        btSetModeAt(drv, 5000);


    chnWrite(drv->serialDriver,
             (uint8_t *)command,
             commandLen);



};


/**
    @parameters uint16_t timeout: time to wait between reset port togling (milliseconds)
*/

void btSetModeAt(void * instance, uint16_t timeout){

    if(!instance)
        return;

    BluetoothDriver *drv = (BluetoothDriver *) instance;
    //reset module (low), pull key high
    chSysLock();
    palSetPad(BT_MODE_KEY_PORT, BT_MODE_KEY_PIN);
    chSysUnlock();

    chThdSleepMilliseconds(100);

    chSysLock();
    palClearPad(BT_RESET_PORT, BT_RESET_PIN);
    chSysUnlock();


    chThdSleepMilliseconds(timeout);   //wait for reset
    chSysLock();
    palSetPad(BT_RESET_PORT, BT_RESET_PIN);
    chSysUnlock();

    chThdSleepMilliseconds(timeout);   //wait for module recovery
    //we should be in AT mode, with 38400 baud
    drv->currentWorkMode=atMode;
};

/**
    @parameters uint16_t timeout: time to wait between reset port togling (milliseconds)
*/

void btSetModeComm(void * instance, uint16_t timeout){

    if(!instance)
        return;

    BluetoothDriver *drv = (BluetoothDriver *) instance;
    //reset module (low), pull key low
    chSysLock();
    palClearPad(BT_MODE_KEY_PORT, BT_MODE_KEY_PIN);
    chSysUnlock();

    chThdSleepMilliseconds(100);

    chSysLock();
    palClearPad(BT_RESET_PORT, BT_RESET_PIN);
    chSysUnlock();
    chThdSleepMilliseconds(timeout);   //wait for reset
    chSysLock();
    palSetPad(BT_RESET_PORT, BT_RESET_PIN);
    chSysUnlock();
    chThdSleepMilliseconds(timeout);   //wait for module recovery
    //we should be in communication mode, with configured baud
    drv->currentWorkMode=communicationMode;
};





/**
    Function to trash not needed incoming serial data
*/
void btEmptyIncomingSerial(void *instance){

    BluetoothDriver *drv = (BluetoothDriver *) instance;

    if(!drv || !(drv->serialDriver))
        return;



    while(Q_TIMEOUT != sdGetTimeout(drv->serialDriver, TIME_IMMEDIATE))
        ;
    return;


};





#endif //#if HAL_USE_BLUETOOTH || defined(__DOXYGEN__)
