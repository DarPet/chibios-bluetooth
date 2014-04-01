/*!
 * @file testbluetooth.h
 * @brief Header file for testing bluetooth module in ChibiosRT.
 *
 * @addtogroup BLUETOOTH
 * @{
 */

#ifndef TESTBLUETOOTH_H_INCLUDED
#define TESTBLUETOOTH_H_INCLUDED

#include "hal.h"
#include "bluetooth.h"
#include "stdlib.h"

#if HAL_USE_BLUETOOTH || defined(__DOXYGEN__)

#if (HAL_USE_HC_05_BLUETOOTH || defined(__DOXYGEN__))
#include "hc05.h"
#endif

#endif // HAL_USE_BLUETOOTH || defined(__DOXYGEN__)

#endif // TESTBLUETOOTH_H_INCLUDED
/** @} */
