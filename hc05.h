/*!
 * @file hc05.h
 * @brief Header file for HC-05 SPP device for bluetooth module in ChibiosRT.
 *
 * @addtogroup BLUETOOTH
 * @{
 */

#include "hal.h"
#include "bluetooth.h"

#ifndef HC05_H_INCLUDED
#define HC05_H_INCLUDED

#if HAL_USE_HC05 || defined(__DOXYGEN__)


/**
 * @brief HC-05 BluetoothDriver configuration struct.
 */
typedef struct hc05config{
    int txport;
    int txpin;
    int rxport;
    int rxpin;
    int rtsport;
    int rtspin;
    int ctsport;
    int ctspin;
    int resetport;
    int resetpin;
    int keyport;
    int keypin;
}hc05config;


#endif  //HAL_USE_HC05 || defined(__DOXYGEN__)
#endif // HC05_H_INCLUDED


/** @} */
