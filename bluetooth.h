/*!
 * @file bluetooth.h
 * @brief Header file for bluetooth module in ChibiosRT.
 *
 * @addtogroup BLUETOOTH
 * @{
 */


#ifndef BLUETOOTH_H_INCLUDED
#define BLUETOOTH_H_INCLUDED

#if HAL_USE_BLUETOOTH || defined(__DOXYGEN__)

struct BluetoothDeviceVMT {
    int (*sendBuffer)(void *instance, char *buffer, int bufferlength);
    int (*sendCommandByte)(void *instance, int commandByte);
    int (*canRecieve)(void *instance);
    int (*readBuffer)(void *instance, char *buffer, int maxlength);
    int (*setPinCode)(void *instance, char *pin, int pinlength);
    int (*setName)(void *instance, char *newname, int namelength);
};

typedef struct BluetoothConfig BluetoothConfig;

typedef struct BluetoothDriver{
    const struct BluetoothDeviceVMT *vmt;

};




int btSend(BluetoothDriver *instance, int commandByte, char *buffer, int bufferlength);

int btIsFrame(BluetoothDriver *instance);

int btRead(BluetoothDriver *instance, char *buffer, int maxlen);




#endif // BLUETOOTH_H_INCLUDED
/** @} */
