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
#include "shell.h"
#include "chprintf.h"

#include "testbluetooth.h"
#include "hc05console.h"

#include "usbcfg.h"



#if HAL_USE_BLUETOOTH || defined(__DOXYGEN__)

#define TESTBT_BUFFERLEN 50


SerialUSBDriver SDU1;
struct BluetoothDriver* BluetoothDriverForConsole;

void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[]) {
  size_t n, size;

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: mem\r\n");
    chprintf(chp, "Strlen argv0: %i", strlen(argv[0]));
    return;
  }
  n = chHeapStatus(NULL, &size);
  chprintf(chp, "core free memory : %u bytes\r\n", chCoreStatus());
  chprintf(chp, "heap fragments   : %u\r\n", n);
  chprintf(chp, "heap free total  : %u bytes\r\n", size);
}

void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[])
{
  static const char *states[] = {THD_STATE_NAMES};
  Thread *tp;

  (void)argv;
  if (argc > 0)
  {
    chprintf(chp, "Usage: threads\r\n");
    return;
  }
  chprintf(chp, "%20s %10s %10s %6s %6s %11s %7s\r\n",
           "name", "add", "stack", "prio", "refs", "state", "time");
  tp = chRegFirstThread();
  do
  {
    chprintf(chp, "%20s %.10lx %.10lx %6lu %6lu %11s %7lu\r\n",
             (uint32_t)tp->p_name, (uint32_t)tp, (uint32_t)tp->p_ctx.r13,
             (uint32_t)tp->p_prio, (uint32_t)(tp->p_refs - 1),
             states[tp->p_state], (uint32_t)tp->p_time);
    tp = chRegNextThread(tp);
  } while (tp != NULL);
}

static const ShellCommand commands[] = {
    {"mem", cmd_mem},
    {"threads", cmd_threads},
    {"modeat", cmd_hc05SetModeAT},
    {"modecomm", cmd_hc05SetModeComm},
    {"btsetname", cmd_hc05SetNameIMC},
    {"btsendbuff", cmd_hc05SendBuffer},
    {"btsendat", cmd_hc05SendATCommand},
    {"btread", cmd_hc05GetBuffer},
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

    //set the test driver struct as the consolestruct
    BluetoothDriverForConsole = &myTestBluetoothDriver;


    btOpen(&myTestBluetoothDriver, &myTestBluetoothConfig);

    static char myTestBuffer[TESTBT_BUFFERLEN+1];
    memset(&myTestBuffer, '\0' , TESTBT_BUFFERLEN+1);



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


        if (btCanRecieve(&myTestBluetoothDriver))
        {
            memset(&myTestBuffer, '\0' , TESTBT_BUFFERLEN+1);
            hc05readBuffer(&myTestBluetoothDriver, myTestBuffer, TESTBT_BUFFERLEN);
            hc05sendBuffer(&myTestBluetoothDriver, myTestBuffer, TESTBT_BUFFERLEN);
        }


        chThdSleepMilliseconds(500);
    }


    return 0;
}






#endif //HAL_USE_BLUETOOTH || defined(__DOXYGEN__)

 /** @} */
