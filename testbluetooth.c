/*!
 * @file testbluetooth.c
 * @brief Source file for testing bluetooth module in ChibiosRT.
 *
 * @addtogroup BLUETOOTH
 * @{
 */
#include "testbluetooth.h"

#if HAL_USE_BLUETOOTH || defined(__DOXYGEN__)

static char myBtInBuffer[BLUETOOTH_INPUT_BUFFER_SIZE+1];
static char myBtOutBuffer[BLUETOOTH_OUTPUT_BUFFER_SIZE+1];


int main(void){


    INPUTQUEUE_DECL (myBtInputQueue, myBtInBuffer, BLUETOOTH_INPUT_BUFFER_SIZE, NULL, NULL);
    OUTPUTQUEUE_DECL (myBtOutputQueue, myBtOutBuffer, BLUETOOTH_OUTPUT_BUFFER_SIZE, NULL, NULL);

    static struct hc05_config_t myhc05_config = {
        .txport = gpioa_port,
        .txpin = 2,
        .txalternatefunction = 7,
        .rxport = gpioa_port,
        .rxpin = 3,
        .rxalternatefunction = 7,
        .resetport = gpioe_port,
        .resetpin = 5,
        .keyport = gpioe_port,
        .keypin = 4,
        .serialdriver = sd2
    };

    static struct BluetoothConfig myTestBluetoothConfig ={
        .name = "Pumukli",
        .pincode = "1234",
        .baudrate = b38400,
        .usedmodule = hc05,
        .myhc05config = &myhc05_config

        };

    static struct BluetoothDriver myTestBluetoothDriver ={
        .vmt = &hc05BtDevVMT,
        .config = &myTestBluetoothConfig,
        .btInputQueue = &myBtInputQueue,
        .btOutputQueue = &myBtOutputQueue,
        .driverIsReady = 0,
        .commSleepTimeMs = 100
        };

return 0;
}






#endif //HAL_USE_BLUETOOTH || defined(__DOXYGEN__)

 /** @} */
