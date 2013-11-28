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

static char btATCommandString[ BT_MAX_COMMAND_SIZE + sizeof(btATCommandTermination) + 1 ];

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

static SerialConfig btDefaultSerialConfigAT = { 38400,0,0,0 };

static WORKING_AREA(btSendThreadWa, 128);
static msg_t btSendThread(void *instance) {

    if (!instance)
        chThdSleep(TIME_INFINITE);

    BluetoothDriver *drv = (BluetoothDriver *) instance;

    chRegSetThreadName("btSendThread");

    while (TRUE) {
        if ( chOQIsEmptyI(drv->bluetoothConfig->btInputQueue) ){
            //queue empty, we have nothing to send
            chThdSleepMilliseconds(drv->bluetoothConfig->commSleepTimeMs);
            continue;
        }
        else{


        }
      }

  return (msg_t) 0;
}

static WORKING_AREA(btRecieveThreadWa, 128);
static msg_t btRecieveThread(void *instance) {

    (void)instance;
    chRegSetThreadName("btRecieveThread");

    while (TRUE) {
        palClearPad(GPIOD, GPIOD_LED3);
        chThdSleepMilliseconds(500);
        palSetPad(GPIOD, GPIOD_LED3);
        chThdSleepMilliseconds(500);
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

    //switch to AT mode
    drv->vmt->btSetModeAt(drv, 5000);
    //start the serial driver, then configure the module

    /**
    TODO: Serial driver defines
    */
    palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(7));
    palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(7));
    sdStart(drv->serialDriver, &btDefaultSerialConfigAT);

    //test AT mode

    if(btTestAT(drv))
        chprintf((BaseSequentialStream *)&SDU1, "\r\nERROR: AT test FAILED.\r\n");
    else
        chprintf((BaseSequentialStream *)&SDU1, "\r\nINFO: AT test SUCCESSFUL.\r\n");


    //we should do the predefined module configuration here, eg.: name, communication baud rate, PIN code, etc.




    //here we should switch to communications mode and be ready for connections

    drv->vmt->btSetModeComm(drv, 5000);

    btStartReceive(drv);

};


/**
*   Send char through the bluetooth module
*/
int btSendChar(void *instance, uint8_t ch){

    if(!instance)
        return EXIT_FAILURE;

    BluetoothDriver *drv = (BluetoothDriver *) instance;

    if ( chOQIsEmptyI(drv->bluetoothConfig->btInputQueue) )
        ;



    return EXIT_SUCCESS;
};

int btSendBuffer(void *instance, uint16_t len, uint8_t *buffer){
    BluetoothDriver *drv = (BluetoothDriver *) instance;

    return 0;
};



/**
    Test AT command mode by sending "AT\r\n" , the response should be "OK\r\n"

    debug: response is printed to console

    @returns    EXIT_SUCCESS if OK
                EXIT_FAILURE on error
*/
int btTestAT(void *instance){

    BluetoothDriver *drv = (BluetoothDriver *) instance;

    if(!drv || !(drv->serialDriver))
        return EXIT_FAILURE;



    uint8_t btAtTestString[] = { 'A', 'T', '\r', '\n', '\0' };
    uint8_t dataCounter = 0;
    uint8_t btTestIncomingData[5];

    btEmptyIncomingSerial(drv);
    sdWrite(drv->serialDriver, (uint8_t *) btAtTestString, 4);
    chThdSleepMilliseconds(100);

    do{
        btTestIncomingData[dataCounter] = sdGetTimeout(drv->serialDriver, TIME_IMMEDIATE);
        dataCounter++;
    }while (dataCounter < 4 && btTestIncomingData[dataCounter-1] != (uint8_t) Q_TIMEOUT);

    btTestIncomingData[4] = '\0';

    //debug:
    chprintf((BaseSequentialStream *)&SDU1, "\r\nDEBUG btTestIncomingData: %s \r\n", (char *) btTestIncomingData);

    return (strcmp((char *) "OK\r\n", (char *) btTestIncomingData) == 0) ? EXIT_SUCCESS : EXIT_FAILURE;

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

void btRxChar(void *instance, uint8_t ch){

    BluetoothDriver *drv = (BluetoothDriver *) instance;

    return;
};

void btSetDeviceName(void *instance, char *newname){

    if ( !instance || !newname )
        return;


    int newNameLen = strlen(newname);


    if( newNameLen > BT_MAX_NAME_LENGTH )
        return;


    BluetoothDriver *drv = (BluetoothDriver *) instance;


    if(drv->currentWorkMode != atMode)
        btSetModeAt(drv, 5000);


    char btPrefix[] = "AT+NAME=";



    chMtxLock(&btCommandMutex);


    strncpy( btATCommandString,
            btPrefix,
            sizeof(btPrefix)/sizeof(btPrefix[0]) );

    strncpy( btATCommandString + sizeof(btPrefix)/sizeof(btPrefix[0]),
            newname,
            newNameLen );

    strncpy( btATCommandString + sizeof(btPrefix)/sizeof(btPrefix[0]) + newNameLen,
            btATCommandTermination,
            sizeof(btATCommandTermination) );



    chnWrite(drv->serialDriver,
             btATCommandString,
             sizeof(btPrefix)/sizeof(btPrefix[0]) + newNameLen + sizeof(btATCommandTermination)/sizeof(btATCommandTermination[0]));


    chMtxUnlock();

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
    palClearPad(BT_RESET_PORT, BT_RESET_PIN);
    palSetPad(BT_MODE_KEY_PORT, BT_MODE_KEY_PIN);
    chSysUnlock();
    chThdSleepMilliseconds(timeout);   //wait for reset
    chSysLock();
    palSetPad(BT_RESET_PORT, BT_RESET_PIN);
    palClearPad(BT_MODE_KEY_PORT, BT_MODE_KEY_PIN);
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
    palClearPad(BT_RESET_PORT, BT_RESET_PIN);
    palClearPad(BT_MODE_KEY_PORT, BT_MODE_KEY_PIN);
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
