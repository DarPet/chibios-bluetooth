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

static uint8_t btATCommandString[BT_MAX_COMMAND_SIZE+3];

// only send the first two characters, the '\0' is for strcmp compatibility
static uint8_t btATCommandTermination[] = {'\r','\n', '\0'};

const struct BluetoothDeviceVMT bluetoothDeviceVMT = {
    .btStart=btStart,
    .btSendChar=btSendChar,
    .btSendBuffer=btSendBuffer,
    .btStartReceive=btStartReceive,
    .btStopReceive=btStopReceive,
    .btRxChar=btRxChar,
    .btSetDeviceName=btSetDeviceName,
    .btSetModeAt=btSetModeAt,
    .btSetModeComm=btSetModeComm,
};

static SerialConfig btDefaultSerialConfigAT = { 38400, };

static WORKING_AREA(btThreadWa, 128);
static msg_t btThread(BluetoothDriver *instance) {

  chRegSetThreadName("btThread");

  while (TRUE) {
    palClearPad(GPIOD, GPIOD_LED3);
    chThdSleepMilliseconds(500);
    palSetPad(GPIOD, GPIOD_LED3);
    chThdSleepMilliseconds(500);
  }
}

void btInit(BluetoothDriver *instance, BluetoothConfig *config){
    //null pointer check
    if (!instance || !config)
        return;

    instance->bluetoothConfig = config;
    instance->serialDriver = (config->btSerialDriver != NULL) ? config->btSerialDriver : BT_DEFAULT_SERIAL_DRIVER_ADDRESS;
    instance->serialConfig = (config->btSerialConfig != NULL) ? config->btSerialConfig : &btDefaultSerialConfigAT;
    instance->vmt = &bluetoothDeviceVMT;

    //create thread, but do not start it yet
    instance->thread=chThdCreateI(btThreadWa, sizeof(btThreadWa), NORMALPRIO, btThread, instance);


};

void btStart(BluetoothDriver *instance){

    if(!instance || instance->thread)
        return;

    //switch to AT mode
    btSetModeAt(5000);
    //start the serial driver, then configure the module

    /**
    TODO: Serial driver defines
    */
    palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(7));
    palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(7));
    sdStart(instance->serialDriver, &btDefaultSerialConfigAT);

    //test AT mode

    if(btTestAT(instance))
        chprintf(&SDU1, "\r\nERROR: AT test FAILED.\r\n");
    else
        chprintf(&SDU1, "\r\nINFO: AT test SUCCESSFUL.\r\n");


    //we should do the predefined module configuration here, eg.: name, communication baud rate, PIN code, etc.




    //here we should switch to communications mode and be ready for connections

    instance->vmt->btSetModeComm(5000);

    chThdResume(instance->thread);

};

int btSendChar(BluetoothDriver *instance, uint8_t ch){

    return;
};

int btSendBuffer(BluetoothDriver *instance, uint16_t len, uint8_t *buffer){

    return;
};



/**
    Test AT command mode by sending "AT\r\n" , the response should be "OK\r\n"

    debug: response is printed to console

    @returns    0 if OK
                1 if there is an error
                -1 with null pointer
*/
int btTestAT(BluetoothDriver *instance){
    if(!instance || !(instance->serialDriver))
        return -1;

    uint8_t btAtTestString[] = { 'A', 'T', '\r', '\n', '\0' };
    uint8_t dataCounter = 0;
    uint8_t btTestIncomingData[5];

    btEmptyIncomingSerial(instance);
    sdWrite(instance->serialDriver, (uint8_t *) btAtTestString, 4);
    chThdSleepMilliseconds(100);

    do{
        btTestIncomingData[dataCounter] = sdGetTimeout(instance->serialDriver, TIME_IMMEDIATE);
        dataCounter++;
    }while (dataCounter < 4 && btTestIncomingData[dataCounter-1] != (uint8_t) Q_TIMEOUT);

    btTestIncomingData[4] = '\0';

    //debug:
    chprintf(&SDU1, "\r\nDEBUG btTestIncomingData: %s \r\n", (char *) btTestIncomingData);

    if( strcmp((char *) btAtTestString, (char *) btTestIncomingData) == 0)
        return 0;
    else
        return 1;
};

//start communication
void btStartReceive(BluetoothDriver *instance){



    return;
};

// stop communication
void btStopReceive(BluetoothDriver *instance){



    return;
};

void btRxChar(BluetoothDriver *instance, uint8_t ch){



    return;
};

void btSetDeviceName(BluetoothDriver *instance, uint8_t *newname){



    return;
};


/**
    @parameters uint16_t timeout: time to wait between reset port togling (milliseconds)
*/

void btSetModeAt(uint16_t timeout){

    //reset module (low), pull key high
    chSysLock();
    palClearPad(BT_RESET_PORT, BT_RESET_PIN);
    palSetPad(BT_MODE_KEY_PORT, BT_MODE_KEY_PIN);
    chSysUnlock();
    chThdSleepMilliseconds(timeout);   //wait for reset
    chSysLock();
    palSetPad(BT_RESET_PORT, BT_RESET_PIN);
    chSysUnlock();
    chThdSleepMilliseconds(timeout);   //wait for module recovery
    //we should be in AT mode, with 38400 baud

};

/**
    @parameters uint16_t timeout: time to wait between reset port togling (milliseconds)
*/

void btSetModeComm(uint16_t timeout){

    //reset module (low), pull key low
    chSysLock();
    palClearPad(BT_RESET_PORT, BT_RESET_PIN);
    palClearPad(BT_MODE_KEY_PORT, BT_MODE_KEY_PIN);
    chSysUnlock();
    chThdSleepMilliseconds(timeout);   //wait for reset
    chSysLock();
    palSetPad(BT_RESET_PORT, BT_RESET_PIN);
    chSysUnlock();
    chThdSleepMilliseconds(timeout);   //wait for module recovery
    //we should be in communication mode, with configured baud
};

/**
    Function to trash not needed incoming serial data
*/
void btEmptyIncomingSerial(BluetoothDriver *instance){
    if(!instance || !(instance->serialDriver))
        return;

    while(Q_TIMEOUT != sdGetTimeout(instance->serialDriver, TIME_IMMEDIATE))
        ;
    return;


};





#endif //#if HAL_USE_BLUETOOTH || defined(__DOXYGEN__)
