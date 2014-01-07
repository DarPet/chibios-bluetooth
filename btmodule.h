#ifndef BTMODULE_H_INCLUDED
#define BTMODULE_H_INCLUDED

#include <sys/queue.h>
#include "ch.h"
#include "hal.h"
#include "serial.h"


#define HAL_USE_BLUETOOTH

#if defined HAL_USE_BLUETOOTH || defined(__DOXYGEN__)


#define BT_MAX_NAME_LENGTH 128
#define BT_MAX_COMMAND_SIZE 256
#define BT_DEFAULT_SERIAL_DRIVER_ADDRESS &SD2
#define BT_RESET_PIN 5
#define BT_RESET_PORT GPIOE
#define BT_MODE_KEY_PIN 4
#define BT_MODE_KEY_PORT GPIOE

//these should only recieve an "OK\r\n" response from the module

#define BT_AT_TEST(driver) driver->vmt->btSendAtCommand(driver , "AT\r\n");
#define BT_RESET(driver) driver->vmt->btSendAtCommand(driver , "AT+RESET\r\n");
#define BT_RESTORE_DEFAULTS(driver) driver->vmt->btSendAtCommand(driver , "AT+ORGL\r\n");
#define BT_SET_NAME(driver,param) driver->vmt->btSendAtCommand(driver , "AT+NAME="param"\r\n");
#define BT_SET_ROLE(driver,param) driver->vmt->btSendAtCommand(driver , "AT+ROLE="param"\r\n");
#define BT_SET_DEVICE_TYPE(driver,param) driver->vmt->btSendAtCommand(driver , "AT+CLASS="param"\r\n");
#define BT_SET_INQUIRE_ACCESS_CODE(driver,param) driver->vmt->btSendAtCommand(driver , "AT+IAC="param"\r\n");
#define BT_SET_INQUIRE_ACCESS_MODE(driver,param1,param2,param3) driver->vmt->btSendAtCommand(driver , "AT+INQM="param1","param2","param3"\r\n");
#define BT_SET_PASSKEY(driver,param) driver->vmt->btSendAtCommand(driver , "AT+PSWD="param"\r\n");
#define BT_SET_SERIAL_PARAMETERS(driver,param1,param2,param3) driver->vmt->btSendAtCommand(driver , "AT+UART="param1","param2","param3"\r\n");
#define BT_SET_CONNECTION_MODE(driver,param) driver->vmt->btSendAtCommand(driver , "AT+CMODE="param"\r\n");
#define BT_SET_BIND_BT_ADDRESS(driver,param) driver->vmt->btSendAtCommand(driver , "AT+BIND="param"\r\n");
#define BT_SET_LED_DRIVE(driver,param1,param2) driver->vmt->btSendAtCommand(driver , "AT+POLAR="param1","param2"\r\n");
#define BT_SET_PAGE_SCAN_INQUIRE_SCAN(driver,param1,param2,param3,param4) driver->vmt->btSendAtCommand(driver , "AT+IPSCAN="param1","param2","param3","param4"\r\n");
#define BT_SET_SNIFF_ENERGY_PARAMETER(driver,param1,param2,param3,param4) driver->vmt->btSendAtCommand(driver , "AT+SNIFF="param1","param2","param3","param4"\r\n");
#define BT_SET_SAFE_ENCRYPTION_MODE(driver,param1,param2) driver->vmt->btSendAtCommand(driver , "AT+SENM="param1","param2"\r\n");
#define BT_DELETE_AUTHENTICATED_DEVICE(driver,param) driver->vmt->btSendAtCommand(driver , "AT+PMSAD="param"\r\n");
#define BT_DELETE_ALL_AUTHENTICATED_DEVICES(driver) driver->vmt->btSendAtCommand(driver , "AT+RMAAD\r\n");
#define BT_SEEK_AUTHENTICATED_DEVICE(driver,param) driver->vmt->btSendAtCommand(driver , "AT+FSAD="param"\r\n");
#define BT_INIT_SPP_PROFILE(driver) driver->vmt->btSendAtCommand(driver , "AT+INIT\r\n");
#define BT_CANCEL_BT_DEVICE(driver) driver->vmt->btSendAtCommand(driver , "AT+INQC\r\n");
#define BT_SET_PAIR(driver,param1,param2) driver->vmt->btSendAtCommand(driver , "AT+PAIR="param1","param2"\r\n");
#define BT_CONNECT_DEVICE(driver,param) driver->vmt->btSendAtCommand(driver , "AT+LINK="param"\r\n");
#define BT_ENTER_ENERGY_MODE(driver,param) driver->vmt->btSendAtCommand(driver , "AT+ENSNIFF="param"\r\n");
#define BT_EXIT_ENERGY_MODE(driver,param) driver->vmt->btSendAtCommand(driver , "AT+EXSNIFF="param"\r\n");

//these recieve something else from the module

#define BT_GET_ADDRESS(driver) driver->vmt->btSendAtCommand(driver , "AT+ADDR?\r\n");
#define BT_GET_VERSION(driver) driver->vmt->btSendAtCommand(driver , "AT+VERSION?\r\n");
#define BT_GET_NAME(driver) driver->vmt->btSendAtCommand(driver , "AT+NAME?\r\n");
#define BT_GET_ROLE(driver) driver->vmt->btSendAtCommand(driver , "AT+ROLE?\r\n");
#define BT_GET_DEVICE_TYPE(driver) driver->vmt->btSendAtCommand(driver , "AT+CLASS?\r\n");
#define BT_GET_INQUIRE_ACCESS_CODE(driver) driver->vmt->btSendAtCommand(driver , "AT+IAC?\r\n");
#define BT_GET_INQUIRE_ACCESS_MODE(driver) driver->vmt->btSendAtCommand(driver , "AT+INQM?\r\n");
#define BT_GET_REMOTE_NAME(driver, param) driver->vmt->btSendAtCommand(driver , "AT+RNAME?"param"\r\n");
#define BT_GET_PASSKEY(driver) driver->vmt->btSendAtCommand(driver , "AT+PSWD?\r\n");
#define BT_GET_SERIAL_PARAMETERS(driver) driver->vmt->btSendAtCommand(driver , "AT+UART?\r\n");
#define BT_GET_CONNECTION_MODE(driver) driver->vmt->btSendAtCommand(driver , "AT+CMODE?\r\n");
#define BT_GET_BIND_BT_ADDRESS(driver) driver->vmt->btSendAtCommand(driver , "AT+BIND?\r\n");
#define BT_GET_LED_DRIVE(driver) driver->vmt->btSendAtCommand(driver , "AT+POLAR?\r\n");
#define BT_GET_PAGE_SCAN_INQUIRE_SCAN(driver) driver->vmt->btSendAtCommand(driver , "AT+IPSCAN?\r\n");
#define BT_GET_SNIFF_ENERGY_PARAMETER(driver) driver->vmt->btSendAtCommand(driver , "AT+SNIFF?\r\n");
#define BT_GET_SAFE_ENCRYPTION_MODE(driver) driver->vmt->btSendAtCommand(driver , "AT+SENM?\r\n");
#define BT_GET_AUTHENTICATED_DEVICE_COUNT(driver) driver->vmt->btSendAtCommand(driver , "AT+ADCN?\r\n");
#define BT_GET_MOST_RECENT_ADDRESS(driver) driver->vmt->btSendAtCommand(driver , "AT+MRAD?\r\n");
#define BT_GET_WORK_STATUS(driver) driver->vmt->btSendAtCommand(driver , "AT+STATE?\r\n");
#define BT_INQUIRE_BT_DEVICE(driver) driver->vmt->btSendAtCommand(driver , "AT+INQ\r\n");
#define BT_DISCONNECT_DEVICE(driver) driver->vmt->btSendAtCommand(driver , "AT+DISC\r\n");


#define _bluetooth_device_methods                                                           \
    void (*btStart)(void *instance);                                      \
    void (*btStartReceive)(void *instance);                                                     \
    void (*btStopReceive)(void *instance);                                                      \
    void (*btSetModeAt)(void * instance, uint16_t timeout);                                           \
    void (*btSetModeComm)(void * instance, uint16_t timeout);                                           \
    void (*btEmptyIncomingSerial)(void *instance);                         \
    void (*btSendAtCommand)(void *instance, char *command);

#define _base_bluetooth_device_data                                         \
    /* Name of driver */                                                    \
    char *name;                                                             \
    /* Serial driver */                                                     \
    SerialDriver *serialDriver;                                             \
    /* Serial Config*/                                                      \
    SerialConfig *serialConfig;                                             \
    /* Bluetooth Config*/                                                      \
    BluetoothConfig *bluetoothConfig;                                             \
    /* Error count */                                                        \
    uint16_t errorCount;                    \
    /* Thread which handles data recieving*/                                        \
    btModuleWorkMode currentWorkMode;                                                   \
    /* Thread which handles data recieving*/                                        \
    Thread *sendThread;                                                   \
    /* Thread which handles data sending*/                                        \
    Thread *recieveThread;

/**
    @brief Bluetooth module work modes
*/
typedef enum btModuleWorkMode{
    atMode,
    communicationMode,
}btModuleWorkMode;

struct BluetoothDeviceVMT {
    _bluetooth_device_methods
};

/**
    @brief Bluetooth driver configuration structure
*/

typedef struct BluetoothConfig{
    SerialConfig *btSerialConfig;
    SerialDriver *btSerialDriver;
    uint8_t btModuleName[BT_MAX_NAME_LENGTH];
    uint16_t commBaudRate;
    uint16_t atBaudRate;
    uint16_t commSleepTimeMs;
    uint16_t sendQueueSize;
    uint16_t recieveQueueSize;
    InputQueue *btInputQueue;
    OutputQueue *btOutputQueue;

} BluetoothConfig;

typedef struct BluetoothDriver{
    const struct BluetoothDeviceVMT *vmt;
    _base_bluetooth_device_data

} BluetoothDriver;




#ifdef __cplusplus
extern "C" {
#endif

    void btStart(void *instance);

    void btStartReceive(void *instance);

    void btStopReceive(void *instance);

    void btSendAtCommand(void *instance, char *command);

    void btInit(void *instance, BluetoothConfig *config);

    void btSetModeAt(void * instance, uint16_t timeout);

    void btSetModeComm(void * instance, uint16_t timeout);

    void btEmptyIncomingSerial(void *instance);

#ifdef __cplusplus
}
#endif

#endif //HAL_USE_BLUETOOTH

#endif // BTMODULE_H_INCLUDED
