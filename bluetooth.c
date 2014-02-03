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

    // Abort on non-existent driver or buffer (when it should exist)
    if (!instance || (bufferlength && !buffer))
        return EXIT_FAILURE;

    if (!(instance->vmt->sendCommandByte(instance, command)))
        return EXIT_FAILURE;

    //only command is sent
    if(!bufferlength)
        return EXIT_SUCCESS;

    if (!(instance->vmt->sendBuffer(instance, buffer, bufferlength)))
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

/*!
 *  \brief Check if there is an incoming frame to be processed
 *
 * \param[in] instance BluetoothDriver to use
 * \return 0 if there is no frame, 1 if there is a frame
 */

int btIsFrame(BluetoothDriver *instance){

    if (!instance)
        return 0;

    return instance->vmt->canRecieve(instance);
}

/*!
 * \brief Reads data from bluetooth module
 *
 * Copy the given (bufferlen sized) buffer to the specified BluetoothDriver input queue to send through the bluetooth device
 *
 * \param[in] instance A BluetoothDriver object
 * \param[in] buffer A pointer to a buffer
 * \param[in] maxlen The length of the buffer
 * \return EXIT_SUCCESS when we wrote data to the buffer, or EXIT_FAILURE
 */

int btRead(BluetoothDriver *instance, char *buffer, int maxlen){

    if (!instance || !buffer || maxlen == 0)
        return EXIT_FAILURE;

    //we have incoming data ready to be served
    if (instance->vmt->canRecieve(instance))
    {
        instance->vmt->readBuffer(instance, buffer, maxlen);
        return EXIT_SUCCESS;
    }
    else
        return EXIT_FAILURE;
}


/*!
 * \brief Starts the driver
 *
 * Get the given bluetooth driver ready for further communication, using the specified config.
 *
 * \param[in] instance A BluetoothDriver object
 * \param[in] config A BluetoothConfig the use
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */
int btOpen(BluetoothDriver *instance, BluetoothConfig *config){

    if (!instance || !config)
        return EXIT_FAILURE;

    return instance->vmt->open(instance, config);
}


/*!
 * \brief Stops the driver
 *
 * Clean up the communications channel and stop the driver.
 *
 * \param[in] instance A BluetoothDriver object
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */
int btClose(BluetoothDriver *instance){

    if (!instance)
        return EXIT_FAILURE;

    return instance->vmt->close(instance);
}

/** @} */
