/*!
 * @file testbluetooth.c
 * @brief Source file for testing bluetooth module in ChibiosRT.
 *
 * @addtogroup BLUETOOTH
 * @{
 */
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "shell.h"

#include "testbluetooth.h"

#include "usbcfg.h"



#if HAL_USE_BLUETOOTH || defined(__DOXYGEN__)

#define TESTBT_BUFFERLEN 50


SerialUSBDriver SDU1;

static const ShellCommand commands[] = {
 //   {"mem", cmd_mem},
 //   {"threads", cmd_threads},
    {NULL,NULL}
};

#define SHELL_WA_SIZE THD_WA_SIZE(2048)

extern SerialUSBDriver SDU1;

static const ShellConfig shell_cfg1 = {
    (BaseSequentialStream *) &SDU1,
    commands
};

void connectConsole(void) {
    shellInit();
    sduObjectInit(&SDU1);
    sduStart(&SDU1, &serusbcfg);
    usbDisconnectBus(serusbcfg.usbp);
    chThdSleepMilliseconds(1000);
    usbStart(serusbcfg.usbp, &usbcfg);
    usbConnectBus(serusbcfg.usbp);
}

static char myBtInBuffer[BLUETOOTH_INPUT_BUFFER_SIZE+1];
static char myBtOutBuffer[BLUETOOTH_OUTPUT_BUFFER_SIZE+1];

extern struct BluetoothDeviceVMT hc05BtDevVMT;

int main(void){

    Thread *shelltp=NULL;

    halInit();
    chSysInit();
    connectConsole();


    static INPUTQUEUE_DECL (myBtInputQueue, myBtInBuffer, BLUETOOTH_INPUT_BUFFER_SIZE, NULL, NULL);
    static OUTPUTQUEUE_DECL (myBtOutputQueue, myBtOutBuffer, BLUETOOTH_OUTPUT_BUFFER_SIZE, NULL, NULL);

    static struct hc05_config_t myhc05_config = {
        .txport = gpioa_port,
        .txpin = 2,
        .txalternatefunction = 7,
        .rxport = gpioa_port,
        .rxpin = 3,
        .rxalternatefunction = 7,
        .resetport = gpioe_port,
        .resetpin = 5,
        .keyport = gpioe_port,
        .keypin = 4,
        .serialdriver = sd2
    };

    static struct BluetoothConfig myTestBluetoothConfig ={
        .name = "Pumukli",
        .pincode = "1234",
        .baudrate = b38400,
        .usedmodule = hc05,
        .myhc05config = &myhc05_config

        };

    static struct BluetoothDriver myTestBluetoothDriver ={
        .vmt = &hc05BtDevVMT,
        .config = &myTestBluetoothConfig,
        .btInputQueue = &myBtInputQueue,
        .btOutputQueue = &myBtOutputQueue,
        .driverIsReady = 0,
        .commSleepTimeMs = 100
        };

    btOpen(&myTestBluetoothDriver, &myTestBluetoothConfig);

    static char myTestBuffer[TESTBT_BUFFERLEN];
    memset(&myTestBuffer, '\0' , TESTBT_BUFFERLEN);


    while(TRUE) {
        if (!shelltp) {
            if(SDU1.config->usbp->state==USB_ACTIVE)
                shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
        }
        else {
            if(chThdTerminated(shelltp)) {
                chThdRelease(shelltp);
                shelltp=NULL;
            }
        }


        if (btRead(&myTestBluetoothDriver, myTestBuffer, TESTBT_BUFFERLEN))
            btSend(&myTestBluetoothDriver, 1, myTestBuffer, TESTBT_BUFFERLEN);

        chThdSleepMilliseconds(500);
    }


    return 0;
}






#endif //HAL_USE_BLUETOOTH || defined(__DOXYGEN__)

 /** @} */
