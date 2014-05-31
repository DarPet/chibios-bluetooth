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
char buffer[64];

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

        hc05setName(BluetoothDriverForConsole, argv[0], strlen(argv[0]));

        chprintf(chp, "New name is %s\r\n", argv[0]);
    }
}

/*! \brief send AT command directly with this command
*
*/
void cmd_hc05SendATCommand(BaseSequentialStream *chp, int argc, char *argv[])
{
 //   (void)argv;

    if( argc != 1)
    {
        chprintf(chp, "Usage: btsendat string \r\n");
    }
    else
    {
        chprintf(chp, "Sending command: %s\r\n", argv[0]);

        chprintf(chp, "commandLength: %i\r\n", strlen(argv[0]));

        hc05sendAtCommand(BluetoothDriverForConsole, argv[0]);

        chprintf(chp, "Command sent: %s\r\n", argv[0]);
    }
}



/*! \brief send buffer of data
*
*/
void cmd_hc05SendBuffer(BaseSequentialStream *chp, int argc, char *argv[])
{
    //(void)argv;

    chprintf(chp, "Sending data buffer\r\n");
    chnWrite(BluetoothDriverForConsole->config->myhc05config->hc05serialpointer, argv[0], strlen(argv[0]));
    chnWrite(BluetoothDriverForConsole->config->myhc05config->hc05serialpointer,"\r\n",2);
    chprintf(chp, "Switched to communication mode\r\n");
}

/*! \brief get buffer of data
*
*/
void cmd_hc05GetBuffer(BaseSequentialStream *chp, int argc, char *argv[])
{
    chprintf(chp, "Getting data buffer\r\n");
    memset(buffer,'\0', 40);

    sdReadTimeout(BluetoothDriverForConsole->config->myhc05config->hc05serialpointer, buffer, 32, TIME_IMMEDIATE);
    //chnWrite(BluetoothDriverForConsole->config->myhc05config->hc05serialpointer,"\r\n",2);
    chprintf(chp, "Buffer: %s\r\n", buffer);
}


#endif //HAL_USE_HC_05_BLUETOOTH || defined(__DOXYGEN__)
 /** @} */
