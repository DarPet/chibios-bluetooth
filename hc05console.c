/*!
 * @file hc05.c
 * @brief Source file for console commands for HC-05 SPP device for bluetooth module in ChibiosRT.
 *
 * @addtogroup BLUETOOTH
 * @{
 */

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "shell.h"
#include "chqueues.h"
#include "bluetooth.h"
#include "hc05.h"
#include "serial.h"
#include "serial_lld.h"
#include "mcuconf.h"
#include <string.h>

#if HAL_USE_HC_05_BLUETOOTH || defined(__DOXYGEN__)


extern SerialUSBDriver SDU1;
extern struct BluetoothDriver* BluetoothDriverForConsole;

/*! \brief set HC 05 to AT mode
*
*/
void cmd_hc05SetModeAT(BaseSequentialStream *chp, int argc, char *argv[])
{
    (void)argv;

    chprintf(chp, "Switching to AT mode\r\n");
    hc05SetModeAt(BluetoothDriverForConsole->config,200);
    chprintf(chp, "Switched to AT mode\r\n");

}


/*! \brief set HC 05 to communication mode
*
*/
void cmd_hc05SetModeComm(BaseSequentialStream *chp, int argc, char *argv[])
{
    (void)argv;

    chprintf(chp, "Switching to communication mode\r\n");
    hc05SetModeComm(BluetoothDriverForConsole->config,200);
    chprintf(chp, "Switched to communication mode\r\n");
}


/*! \brief get the number of byte in the input queue
*
*/
void cmd_hc05GetIQSize(BaseSequentialStream *chp, int argc, char *argv[]);


/*! \brief get the number of bytes in the output queue
*
*/
void cmd_hc05GetOQSize(BaseSequentialStream *chp, int argc, char *argv[]);


/*! \brief set the new PIN code with mode change
*
*/
int cmd_hc05SetPinIMC(BaseSequentialStream *chp, int argc, char *argv[]);

/*! \brief set the new NAME with mode change
*
*/
void cmd_hc05SetNameIMC(BaseSequentialStream *chp, int argc, char *argv[])
{
 //   (void)argv;

    if( argc != 1)
    {
        chprintf(chp, "Usage: btsetnamen string \r\n");
    }
    else
    {
        chprintf(chp, "Setting new NAME: %s\r\n", argv[0]);

        chprintf(chp, "commandLength: %i\r\n", strlen(argv[0]));

        chnWrite((BaseChannel *)BluetoothDriverForConsole->config->myhc05config->hc05serialpointer,argv[0],strlen(argv[0]));
        chnWrite((BaseChannel *)BluetoothDriverForConsole->config->myhc05config->hc05serialpointer,"\r\n",2);

        chprintf(chp, "New name is %s\r\n", argv[0]);
    }
}



/*! \brief send buffer of data
*
*/
void cmd_hc05SendBuffer(BaseSequentialStream *chp, int argc, char *argv[])
{
    //(void)argv;

    chprintf(chp, "Sending data buffer\r\n");
    //hc05sendBuffer(BluetoothDriverForConsole,argv[1], 4);
    chnPutTimeout(&SD2, 'a', TIME_IMMEDIATE);
    chprintf(chp, "Switched to communication mode\r\n");
}

/*! \brief get buffer of data
*
*/
void cmd_hc05GetBuffer(BaseSequentialStream *chp, int argc, char *argv[]);


#endif //HAL_USE_HC_05_BLUETOOTH || defined(__DOXYGEN__)
 /** @} */
