/*!
 * @file hc05.h
 * @brief Header file for console commands for HC-05 SPP device for bluetooth module in ChibiosRT.
 *
 * @addtogroup BLUETOOTH
 * @{
 */

#ifndef HC05CONSOLE_H_INCLUDED
#define HC05CONSOLE_H_INCLUDED

#include "hal.h"
#include <ch.h>
#include <chthreads.h>
#include <chmemcore.h>
#include "bluetooth.h"
#include "hc05.h"
#include <chprintf.h>
#include <shell.h>


#if HAL_USE_HC_05_BLUETOOTH || defined(__DOXYGEN__)





#ifdef __cplusplus
extern "C" {
#endif
    void cmd_hc05SetModeAT(BaseSequentialStream *chp, int argc, char *argv[]);
    void cmd_hc05SetModeComm(BaseSequentialStream *chp, int argc, char *argv[]);
    void cmd_hc05GetIQSize(BaseSequentialStream *chp, int argc, char *argv[]);
    void cmd_hc05GetOQSize(BaseSequentialStream *chp, int argc, char *argv[]);
    void cmd_hc05SetPinIMC(BaseSequentialStream *chp, int argc, char *argv[]);
    void cmd_hc05SetNameIMC(BaseSequentialStream *chp, int argc, char *argv[]);
    void cmd_hc05SendBuffer(BaseSequentialStream *chp, int argc, char *argv[]);
    void cmd_hc05GetBuffer(BaseSequentialStream *chp, int argc, char *argv[]);



#ifdef __cplusplus
}
#endif

#endif  //HAL_USE_HC05 || defined(__DOXYGEN__)
#endif // HC05CONSOLE_H_INCLUDED


/** @} */
