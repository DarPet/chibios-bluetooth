/*!
 * @file bluetooth.h
 * @brief Header file for bluetooth module in ChibiosRT.
 *
 * @addtogroup BLUETOOTH
 * @{
 */
#ifndef BLUETOOTH_H_INCLUDED
#define BLUETOOTH_H_INCLUDED

#include <hal.h>
#include <sys/queue.h>
#include <stdlib.h>


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
/**
 * @brief   Input buffer size.
 * @details Configuration parameter, this is the input buffer size
 */
#if !defined(BLUETOOTH_INPUT_BUFFER_LENGTH) || defined(__DOXYGEN__)
#define BLUETOOTH_INPUT_BUFFER_SIZE 128
#endif
/**
 * @brief   Output buffer size.
 * @details Configuration parameter, this is the output buffer size
 */
#if !defined(BLUETOOTH_OUTPUT_BUFFER_LENGTH) || defined(__DOXYGEN__)
#define BLUETOOTH_OUTPUT_BUFFER_SIZE 128
#endif

/** @} */

/*===========================================================================*/
/* Forward declarations                                                      */
/*===========================================================================*/
//typedef struct BluetoothConfig BluetoothConfig;
//typedef struct BluetoothDriver BluetoothDriver;
//typedef struct hc05_config hc05_config;

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




/**
 * @brief BluetoothDriver configuration struct.
 *
 *  This struct contains multiple pointers to different bluetooth module config structs.
 *  Only one should be non-NULL. The unused ones shuld be set to NULL.
 *  If the unused pointers are a problem, #ifdef modulname method could be used to remove at coompile time
 *  The usedmodule variable shows which config pointer to use.
 *
 */
typedef struct BluetoothConfig{
    char name[BLUETOOTH_MAX_NAME_LENGTH+1];
    char pincode[BLUETOOTH_MAX_PINCODE_LENGTH+1];
    btbitrate_t baudrate;
    Thread *sendThread;
    Thread *recieveThread;
    btmodule_t usedmodule;

    //config pointers from here
    void *myhc05config;

    //config pointers end here

}BluetoothConfig;

/**
 * @brief BluetoothDriver object
 */
typedef struct BluetoothDriver{
    const struct BluetoothDeviceVMT *vmt;
    BluetoothConfig *config;
    InputQueue *btInputQueue;
    OutputQueue *btOutputQueue;
    int driverIsReady;
    int commSleepTimeMs;
}BluetoothDriver;


/**
 * @brief BluetoothDriver virtual methods table.
 */

typedef struct BluetoothDeviceVMT {
    int (*sendBuffer)(BluetoothDriver *instance, char *buffer, int bufferlength);
    int (*sendCommandByte)(BluetoothDriver *instance, int commandByte);
    int (*canRecieve)(BluetoothDriver *instance);
    int (*readBuffer)(BluetoothDriver *instance, char *buffer, int maxlength);
    int (*setPinCode)(BluetoothDriver *instance, char *pin, int pinlength);
    int (*setName)(BluetoothDriver *instance, char *newname, int namelength);
    int (*open)(BluetoothDriver *instance, BluetoothConfig *config);
    int (*close)(BluetoothDriver *instance);
    int (*resetModuleSettings) (BluetoothDriver * instance);
}BluetoothDeviceVMT;







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
