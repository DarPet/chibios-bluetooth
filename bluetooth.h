/*!
 * @file bluetooth.h
 * @brief Header file for bluetooth module in ChibiosRT.
 *
 * @addtogroup BLUETOOTH
 * @{
 */

#include "hal.h"
#include <sys/queue.h>

//bluetooth module driver headers from here
#include "hc05.h"


//bluetooth module driver headers till here

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
 *
 *  Using these instead of writing the bit rate to a 32 bit int should help with 8 and 16 bit compatibility and portability
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

/**
 * @brief Available bluetooth modules to use
 *
 * The "nomodule" entry is reserved, so there is no 0 in the enum
 */
typedef enum {
  nomodule = 0,     //this should not be used
  hc05 = 1
} btmodule_t;


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
 *
 *  This struct contains multiple pointers to different bluetooth module config structs.
 *  Only one should be non-NULL. The unused ones shuld be set to NULL.
 *  The usedmodule variable shows which config pointer to use.
 *
 */
typedef struct BluetoothConfig{
    char name[BLUETOOTH_MAX_NAME_LENGTH];
    int pincode[BLUETOOTH_MAX_PINCODE_LENGTH];
    uint32_t baudrate;
    btmodule_t usedmodule;

    //config pointers from here
    hc05config *myhcconfig;

    //config pointers end here

}BluetoothConfig;

/**
 * @brief BluetoothDriver object
 */
typedef struct BluetoothDriver{
    const struct BluetoothDeviceVMT *vmt;
    BluetoothConfig config;
    InputQueue *btInputQueue;
    OutputQueue *btOutputQueue;
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
int btOpen(BluetoothDriver *instance, BluetoothConfig *config);
int btClose(BluetoothDriver *instance);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_BLUETOOTH */
#endif // BLUETOOTH_H_INCLUDED
/** @} */
