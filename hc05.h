/*!
 * @file hc05.h
 * @brief Header file for HC-05 SPP device for bluetooth module in ChibiosRT.
 *
 * @addtogroup BLUETOOTH
 * @{
 */

#include "hal.h"
#include <ch.h>
#include <chthreads.h>
#include <chmemcore.h>
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
 * @brief GPIO ports that can be used
 */
 typedef enum{
    gpioa_port = 0,
    gpiob_port = 1,
    gpioc_port = 2,
    gpiod_port = 3,
    gpioe_port = 4,
    gpiof_port = 5,
    gpiog_port = 6,
    gpioh_port = 7
}hc05_port_t;


/**
 * @brief HC-05 BluetoothDriver configuration struct.
 */
typedef struct hc05_config{
    /* Rx and Tx ports and pins are dependent on the selected serial driver, and on the develpoment board*/
    hc05_port_t txport;
    int txpin;
    int txalternatefunction;    //number of alternate function of the pin
    hc05_port_t rxport;
    int rxpin;
    int rxalternatefunction;    //number of alternate function of the pin
    hc05_port_t rtsport;
    int rtspin;
    int rtsalternatefunction;    //number of alternate function of the pin
    hc05_port_t ctsport;
    int ctspin;
    int ctsalternatefunction;    //number of alternate function of the pin
    hc05_port_t resetport;
    int resetpin;
    hc05_port_t keyport;
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
