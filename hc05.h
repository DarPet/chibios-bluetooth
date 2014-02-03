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
 * @brief SerialDrivers that can be used by the HC-05
 */

typedef enum{
    sd1 = 0,
    sd2 = 1,
    sd3 = 2,
    sd4 = 4,
    sd5 = 5
}hc05_seriald_t;

/**
 * @brief Possible states of the HC-05 module
 */
typedef enum{
    unknown = 0,
    initializing = 1,
    ready_communication = 2,
    ready_at_command = 3,
    shutting_down = 4
}hc05_state_t;



/**
 * @brief HC-05 BluetoothDriver configuration struct.
 */
typedef struct hc05_config{
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
    hc05_seriald_t serialdriver;
}hc05_config;


#ifdef __cplusplus
extern "C" {
#endif
    int hc05sendBuffer(BluetoothDriver *instance, char *buffer, int bufferlength);
    int hc05sendCommandByte(BluetoothDriver *instance, int commandByte);
    int hc05canRecieve(BluetoothDriver *instance);
    int hc05readBuffer(BluetoothDriver *instance, char *buffer, int maxlength);
    int hc05setPinCode(BluetoothDriver *instance, char *pin, int pinlength);
    int hc05setName(BluetoothDriver *instance, char *newname, int namelength);
    int hc05open(BluetoothDriver *instance, BluetoothConfig *config);
    int hc05close(BluetoothDriver *instance);
#ifdef __cplusplus
}
#endif

#endif  //HAL_USE_HC05 || defined(__DOXYGEN__)
#endif // HC05_H_INCLUDED


/** @} */
