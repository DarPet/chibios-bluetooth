/*!
 * @file hc05.c
 * @brief Source file for HC-05 SPP device for bluetooth module in ChibiosRT.
 *
 * @addtogroup BLUETOOTH
 * @{
 */

#include "hal.h"
#include "hc05.h"

#if HAL_USE_HC05 || defined(__DOXYGEN__)

/*!
 * \brief Sends the given buffer
 *
 *  Writes bufferlength bytes from the buffer to the output queue, if there is enough space in it
 *
 * \param[in] instance A BluetoothDriver object
 * \param[in] buffer A pointer to a buffer to read from
 * \param[in] bufferlength The number of bytes to send
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */
int hc05sendBuffer(BluetoothDriver *instance, char *buffer, int bufferlength);

/*!
 * \brief Sends the given command byte
 *
 * \param[in] instance A BluetoothDriver object
 * \param[in] commandByte A byte to send
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */
int hc05sendCommandByte(BluetoothDriver *instance, int commandByte);

/*!
 * \brief Checks the input queue for incoming data
 *
 * \param[in] instance A BluetoothDriver object
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */
int hc05canRecieve(BluetoothDriver *instance);

/*!
 * \brief Reads from the input queue
 *
 *  Reads maxlength bytes (or less, if there is no more) from the input queue and puts it in the specified buffer
 *
 * \param[in] instance A BluetoothDriver object
 * \param[in] buffer A pointer to a buffer to write into
 * \param[in] maxlength The maximum number of bytes to read (size of the buffer)
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */
int hc05readBuffer(BluetoothDriver *instance, char *buffer, int maxlength);

/*!
 * \brief Sets the pin/access code for the HC-05 module
 *
 *  When in AT mode, we can set the pin/access code of the module.
 *
 * \param[in] instance A BluetoothDriver object
 * \param[in] pin A pointer to a char array containing the new pin code
 * \param[in] pinlength The length of the new pin
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */
int hc05setPinCode(BluetoothDriver *instance, char *pin, int pinlength);

/*!
 * \brief Sets the name for the HC-05 module
 *
 *  When in AT mode, we can set the name of the module.
 *
 * \param[in] instance A BluetoothDriver object
 * \param[in] newname A pointer to a char array containing the new name
 * \param[in] namelength The length of the new name
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */
int hc05setName(BluetoothDriver *instance, char *newname, int namelength);

/*!
 * \brief Starts the driver
 *
 *  Read config
 *  Initialize the serial driver
 *  Set the apropriate port/pin settings
 *  Set the name/pin according to the config
 *  Get the packet-pool ready
 *  Set the ready flag
 *
 * \param[in] instance A BluetoothDriver object
 * \param[in] config A BluetoothConfig the use
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */
int hc05open(BluetoothDriver *instance, BluetoothConfig *config);

/*!
 * \brief Stops the driver
 *
 * Clean up the communications channel and stop the driver.
 *
 * \param[in] instance A BluetoothDriver object
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */
int hc05close(BluetoothDriver *instance);


/**
 * @brief HC05 BluetoothDriver virtual methods table.
 */
const BluetoothDeviceVMT hc05BtDevVMT = {
    .sendBuffer = hc05sendBuffer,
    .sendCommandByte = hc05sendCommandByte,
    .canRecieve = hc05canRecieve,
    .readBuffer = hc05readBuffer,
    .setPinCode = hc05setPinCode,
    .setName = hc05setName,
    .open = hc05open,
    .close = hc05close
};


#endif //HAL_USE_HC05 || defined(__DOXYGEN__)
 /** @} */
