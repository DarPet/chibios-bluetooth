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


#define _bluetooth_device_methods                                                           \
    void (*btStart)(void *instance);                                      \
    int (*btSendChar)(void *instance, uint8_t ch);                                              \
    int (*btSendBuffer)(void *instance, uint16_t len, uint8_t *buffer);                         \
    void (*btStartReceive)(void *instance);                                                     \
    void (*btStopReceive)(void *instance);                                                      \
    void (*btRxChar)(void *instance, uint8_t ch);                                           \
    void (*btSetModeAt)(void * instance, uint16_t timeout);                                           \
    void (*btSetModeComm)(void * instance, uint16_t timeout);                                           \
    void (*btEmptyIncomingSerial)(void *instance);                         \
    void (*btSetDeviceName)(void *instance, char *newname);

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

typedef enum btModuleWorkMode{
    atMode,
    communicationMode,
}btModuleWorkMode;

struct BluetoothDeviceVMT {
    _bluetooth_device_methods
};

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

    int btSendChar(void *instance, uint8_t ch);

    int btSendBuffer(void *instance, uint16_t len, uint8_t *buffer);

    void btStartReceive(void *instance);

    void btStopReceive(void *instance);

    void btRxChar(void *instance, uint8_t ch);

    void btSetDeviceName(void *instance, char *newname);

    void btInit(void *instance, BluetoothConfig *config);

    void btSetModeAt(void * instance, uint16_t timeout);

    void btSetModeComm(void * instance, uint16_t timeout);

    int btTestAT(void *instance);

    void btEmptyIncomingSerial(void *instance);

#ifdef __cplusplus
}
#endif

#endif //HAL_USE_BLUETOOTH

#endif // BTMODULE_H_INCLUDED
