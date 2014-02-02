/*!
 * @file bluetooth.h
 * @brief Header file for bluetooth module in ChibiosRT.
 *
 * @addtogroup BLUETOOTH
 * @{
 */

#include "hal.h"

#ifndef BLUETOOTH_H_INCLUDED
#define BLUETOOTH_H_INCLUDED

#if HAL_USE_BLUETOOTH || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Bluetooth configuration options
 * @{
 */
/**
 * @brief   Default bit rate.
 * @details Configuration parameter, this is the baud rate selected for the
 *          default configuration.
 */
#if !defined(BLUETOOTH_DEFAULT_BITRATE) || defined(__DOXYGEN__)
#define BLUETOOTH_DEFAULT_BITRATE      38400
#endif
/**
 * @brief   Maximum module name length.
 * @details Configuration parameter, this is the maximum name length for the bluetooth module.
 */
#if !defined(BLUETOOTH_MAX_NAME_LENGTH) || defined(__DOXYGEN__)
#define BLUETOOTH_MAX_NAME_LENGTH 32
#endif
/**
 * @brief   Maximum module pin length.
 * @details Configuration parameter, this is the maximum pin code length for the bluetooth module.
 */
#if !defined(BLUETOOTH_MAX_PINCODE_LENGTH) || defined(__DOXYGEN__)
#define BLUETOOTH_MAX_PINCODE_LENGTH 4
#endif

/** @} */


/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief Communication standard bit rates.
 */
typedef enum {
  b1200 = 0,
  b2400 = 1,
  b4800 = 2,
  b9600 = 3,
  b19200 = 4,
  b38400 = 5,
  b57600 = 6,
  b115200 = 7
} btbitrate_t;


typedef struct BluetoothConfig BluetoothConfig;

/**
 * @brief BluetoothDriver virtual methods table.
 */

struct BluetoothDeviceVMT {
    int (*sendBuffer)(void *instance, char *buffer, int bufferlength);
    int (*sendCommandByte)(void *instance, int commandByte);
    int (*canRecieve)(void *instance);
    int (*readBuffer)(void *instance, char *buffer, int maxlength);
    int (*setPinCode)(void *instance, char *pin, int pinlength);
    int (*setName)(void *instance, char *newname, int namelength);
    int (*open)(void *instance);
    int (*close)(void *isntance);
}BluetoothDeviceVMT;

/**
 * @brief BluetoothDriver configuration struct.
 */
typedef struct BluetoothConfig{
    char name[BLUETOOTH_MAX_NAME_LENGTH];
    uint32_t baudrate;
    int pincode[BLUETOOTH_MAX_PINCODE_LENGTH];
}BluetoothConfig;

/**
 * @brief BluetoothDriver object
 */
typedef struct BluetoothDriver{
    const struct BluetoothDeviceVMT *vmt;
    BluetoothConfig config;
}BluetoothDriver;



/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
int btSend(BluetoothDriver *instance, int commandByte, char *buffer, int bufferlength);
int btIsFrame(BluetoothDriver *instance);
int btRead(BluetoothDriver *instance, char *buffer, int maxlen);
int btOpen();
int btClose();
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_BLUETOOTH */
#endif // BLUETOOTH_H_INCLUDED
/** @} */
