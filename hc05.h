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



#if HAL_USE_HC_05_BLUETOOTH || defined(__DOXYGEN__)


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
struct hc05_config_t {
    int txpin;
    enum hc05_port_t txport;
    int txalternatefunction;    //number of alternate function of the pin, if negative --> pushpull is used
    enum hc05_port_t rxport;
    int rxpin;
    int rxalternatefunction;    //number of alternate function of the pin, if negative --> pushpull is used
    enum hc05_port_t rtsport;
    int rtspin;
    int rtsalternatefunction;    //number of alternate function of the pin, if negative --> pushpull is used
    enum hc05_port_t ctsport;
    int ctspin;
    int ctsalternatefunction;    //number of alternate function of the pin, if negative --> pushpull is used
    enum hc05_port_t resetport;
    int resetpin;
    enum hc05_port_t keyport;
    int keypin;
    enum hc05_seriald_t serialdriver;
    SerialDriver *hc05serialpointer;
};

#ifdef __cplusplus
extern "C" {
#endif
    int hc05sendBuffer(struct BluetoothDriver *instance, char *buffer, unsigned int bufferlength);
    int hc05sendByte(struct BluetoothDriver *instance, int mybyte);
    int hc05canRecieve(struct BluetoothDriver *instance);
    int hc05readBuffer(struct BluetoothDriver *instance, unsigned char *buffer, int maxlength);
    int hc05sendAtCommand(struct BluetoothDriver *instance, char* command);
    int hc05setPinCode(struct BluetoothDriver *instance, char *pin, int pinlength);
    int hc05setName(struct BluetoothDriver *instance, char *newname, int namelength);
    int hc05resetDefaults(struct BluetoothDriver *instance);
    int hc05open(struct BluetoothDriver *instance, struct BluetoothConfig *config);
    int hc05close(struct BluetoothDriver *instance);
    int hc05_settxpin(struct BluetoothConfig *config);
    int hc05_setrxpin(struct BluetoothConfig *config);
    int hc05_setrtspin(struct BluetoothConfig *config);
    int hc05_setctspin(struct BluetoothConfig *config);
    int hc05_setresetpin(struct BluetoothConfig *config);
    int hc05_setkeypin(struct BluetoothConfig *config);
    int hc05_updateserialconfig(struct BluetoothConfig *config);
    int hc05_startserial(struct BluetoothConfig *config);
    int hc05_stopserial(struct BluetoothConfig *config);
    void hc05SetModeAt(struct BluetoothConfig *config, uint16_t timeout);
    void hc05SetModeComm(struct BluetoothConfig *config, uint16_t timeout);
#ifdef __cplusplus
}
#endif

#endif  //HAL_USE_HC05 || defined(__DOXYGEN__)
#endif // HC05_H_INCLUDED


/** @} */
