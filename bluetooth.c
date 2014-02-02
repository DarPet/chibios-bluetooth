/*!
 * @file bluetooth.c
 * @brief Source file for bluetooth module in ChibiosRT.
 *
 * @addtogroup BLUETOOTH
 * @{
 */
#include "hal.h"
#include "bluetooth.h"
#if HAL_USE_BLUETOOTH || defined(__DOXYGEN__)

/*!
 * \brief Sends a buffer of data through the specified BluetoothDriver
 *
 * \param[in] instance A BluetoothDriver object
 * \param[in] command Any command for the recieving side
 * \param[in] buffer A pointer to a buffer
 * \param[in] bufferlength The length of the buffer
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */

int btSend(BluetoothDriver *instance, int command, char *buffer, int bufferlength){




    return EXIT_SUCCESS;
}

/*!
 *  \brief Check if there is an incoming frame to be processed
 *
 * \param[in] instance BluetoothDriver to use
 * \return 0 if there is no frame, 1 if there is a frame
 */

int btIsFrame(BluetoothDriver *instance);

/*!
 * \brief Reads data from bluetooth module
 *
 * Copy the given (bufferlen sized) buffer to the specified BluetoothDriver input queue to send through the bluetooth device
 *
 * \param[in] instance A BluetoothDriver object
 * \param[in] buffer A pointer to a buffer
 * \param[in] maxlen The length of the buffer
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */

int btRead(BluetoothDriver *instance, char *buffer, int maxlen);


/** @} */
