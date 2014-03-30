/*!
 * @file hc05.h
 * @brief Header file for HC-05 SPP device for bluetooth module in ChibiosRT.
 *
 * @addtogroup BLUETOOTH
 * @{
 */

#ifndef HC05_H_INCLUDED
#define HC05_H_INCLUDED

#include "hal.h"
#include <ch.h>
#include <chthreads.h>
#include <chmemcore.h>
#include "bluetooth.h"



#if HAL_USE_HC05 || defined(__DOXYGEN__)


/**
 * @brief SerialDrivers that can be used by the HC-05
 */

enum hc05_seriald_t{
    sd1 = 0,
    sd2 = 1,
    sd3 = 2,
    sd4 = 4,
    sd5 = 5
};

/**
 * @brief Possible states of the HC-05 module
 */
enum hc05_state_t{
    st_unknown = 0,
    st_initializing = 1,
    st_ready_communication = 2,
    st_ready_at_command = 3,
    st_shutting_down = 4
};


/**
 * @brief GPIO ports that can be used
 */
 enum hc05_port_t{
    gpioa_port = 0,
    gpiob_port = 1,
    gpioc_port = 2,
    gpiod_port = 3,
    gpioe_port = 4,
    gpiof_port = 5,
    gpiog_port = 6,
    gpioh_port = 7
};


/**
 * @brief HC-05 BluetoothDriver configuration struct.
 *
 *
 *  Alternate function values can be negative. Negative number (e.g. -1) means that no alternate function
 *  should be used, instead we use the pushpull configuration (cts as output, rts as input)
 *
 */
 /* Rx and Tx ports and pins are dependent on the selected serial driver, and on the develpoment board*/
struct hc05_config{
    int txpin;
    enum hc05_port_t txport;
    int txalternatefunction;    //number of alternate function of the pin, if negative --> pushpull is used
    enum hc05_port_t rxport;
    int rxpin;
    int rxalternatefunction;    //number of alternate function of the pin, if negative --> pushpull is used
//    hc05_port_t rtsport;
//    int rtspin;
//    int rtsalternatefunction;    //number of alternate function of the pin, if negative --> pushpull is used
//    hc05_port_t ctsport;
//    int ctspin;
//    int ctsalternatefunction;    //number of alternate function of the pin, if negative --> pushpull is used
    enum hc05_port_t resetport;
    int resetpin;
    enum hc05_port_t keyport;
    int keypin;
    enum hc05_seriald_t serialdriver;
};


#ifdef __cplusplus
extern "C" {
#endif
    int hc05sendBuffer(BluetoothDriver *instance, char *buffer, int bufferlength);
    int hc05sendCommandByte(BluetoothDriver *instance, int commandByte);
    int hc05canRecieve(BluetoothDriver *instance);
    int hc05readBuffer(BluetoothDriver *instance, char *buffer, int maxlength);
    int hc05sendAtCommand(BluetoothDriver *instance, char* command);
    int hc05setPinCode(BluetoothDriver *instance, char *pin, int pinlength);
    int hc05setName(BluetoothDriver *instance, char *newname, int namelength);
    int hc05resetDefaults(BluetoothDriver *instance);
    int hc05open(BluetoothDriver *instance, BluetoothConfig *config);
    int hc05close(BluetoothDriver *instance);
    int hc05_settxpin(BluetoothConfig *config);
    int hc05_setrxpin(BluetoothConfig *config);
    int hc05_setrtspin(BluetoothConfig *config);
    int hc05_setctspin(BluetoothConfig *config);
    int hc05_setresetpin(BluetoothConfig *config);
    int hc05_setkeypin(BluetoothConfig *config);
    int hc05_updateserialconfig(BluetoothConfig *config);
    int hc05_startserial(BluetoothConfig *config)
    int hc05_stopserial(BluetoothConfig *config);
    void hc05SetModeAt(BluetoothConfig *config, uint16_t timeout);
    void hc05SetModeComm(BluetoothConfig *config, uint16_t timeout);
#ifdef __cplusplus
}
#endif

#endif  //HAL_USE_HC05 || defined(__DOXYGEN__)
#endif // HC05_H_INCLUDED


/** @} */
