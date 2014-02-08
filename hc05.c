/*!
 * @file hc05.c
 * @brief Source file for HC-05 SPP device for bluetooth module in ChibiosRT.
 *
 * @addtogroup BLUETOOTH
 * @{
 */

#include "hal.h"
#include "bluetooth.h"
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
 *  Set the apropriate port/pin settings
 *  Set the name/pin according to the config
 *  Get the packet-pool ready
 *  Create pool/buffer threads
 *  Initialize the serial driver
 *  Set the ready flag
 *
 * \param[in] instance A BluetoothDriver object
 * \param[in] config A BluetoothConfig the use
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */
int hc05open(BluetoothDriver *instance, BluetoothConfig *config){

    if(!instance || !config || !(config->myhc05config))
        return EXIT_FAILURE;

    //set up the key and reset pins... using external functions
    hc05_setkeypin(config);
    hc05_setresetpin(config);
    //set up the RX and TX pins
    hc05_settxpin(config);
    hc05_setrxpin(config);
    //set up the RTS and CTS pins
    hc05_setrtspin(config);
    hc05_setctspin(config);

    //packet pool


    //threads



    //serial driver


    //flag

    return EXIT_SUCCESS;
}

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

/*===========================================================================*/
/* Internal functions              .                                         */
/*===========================================================================*/

/*!
 * \brief Sets up the TX pin of the serial line
 *
 *
 * \param[in] config A BluetoothConfig object
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */
int hc05_settxpin(BluetoothConfig *config){

    if(!config || !(config->myhc05config))
        return EXIT_FAILURE;

    switch (config->myhc05config->txport){

        case gpioa_port:
            palSetPadMode(GPIOA, config->myhc05config->txpin,
                          PAL_MODE_ALTERNATE(config->myhc05config->txalternatefunction));
            break;
        case gpiob_port:
            palSetPadMode(GPIOB, config->myhc05config->txpin,
                          PAL_MODE_ALTERNATE(config->myhc05config->txalternatefunction));
            break;
        case gpioc_port:
            palSetPadMode(GPIOC, config->myhc05config->txpin,
                          PAL_MODE_ALTERNATE(config->myhc05config->txalternatefunction));
            break;
        case gpiod_port:
            palSetPadMode(GPIOD, config->myhc05config->txpin,
                          PAL_MODE_ALTERNATE(config->myhc05config->txalternatefunction));
            break;
        case gpioe_port:
            palSetPadMode(GPIOE, config->myhc05config->txpin,
                          PAL_MODE_ALTERNATE(config->myhc05config->txalternatefunction));
            break;
        case gpiof_port:
            palSetPadMode(GPIOF, config->myhc05config->txpin,
                          PAL_MODE_ALTERNATE(config->myhc05config->txalternatefunction));
            break;
        case gpiog_port:
            palSetPadMode(GPIOG, config->myhc05config->txpin,
                          PAL_MODE_ALTERNATE(config->myhc05config->txalternatefunction));
            break;
        case gpioh_port:
            palSetPadMode(GPIOH, config->myhc05config->txpin,
                          PAL_MODE_ALTERNATE(config->myhc05config->txalternatefunction));
            break;
        default:
            return EXIT_FAILURE;
            break;
    }
    return EXIT_SUCCESS;
}

/*!
 * \brief Sets up the RX pin of the serial line
 *
 *
 * \param[in] config A BluetoothConfig object
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */
int hc05_setrxpin(BluetoothConfig *config){

    if(!config || !(config->myhc05config))
        return EXIT_FAILURE;

    switch (config->myhc05config->rxport){

        case gpioa_port:
            palSetPadMode(GPIOA, config->myhc05config->rxpin,
                          PAL_MODE_ALTERNATE(config->myhc05config->rxalternatefunction));
            break;
        case gpiob_port:
            palSetPadMode(GPIOB, config->myhc05config->rxpin,
                          PAL_MODE_ALTERNATE(config->myhc05config->rxalternatefunction));
            break;
        case gpioc_port:
            palSetPadMode(GPIOC, config->myhc05config->rxpin,
                          PAL_MODE_ALTERNATE(config->myhc05config->rxalternatefunction));
            break;
        case gpiod_port:
            palSetPadMode(GPIOD, config->myhc05config->rxpin,
                          PAL_MODE_ALTERNATE(config->myhc05config->rxalternatefunction));
            break;
        case gpioe_port:
            palSetPadMode(GPIOE, config->myhc05config->rxpin,
                          PAL_MODE_ALTERNATE(config->myhc05config->rxalternatefunction));
            break;
        case gpiof_port:
            palSetPadMode(GPIOF, config->myhc05config->rxpin,
                          PAL_MODE_ALTERNATE(config->myhc05config->rxalternatefunction));
            break;
        case gpiog_port:
            palSetPadMode(GPIOG, config->myhc05config->rxpin,
                          PAL_MODE_ALTERNATE(config->myhc05config->rxalternatefunction));
            break;
        case gpioh_port:
            palSetPadMode(GPIOH, config->myhc05config->rxpin,
                          PAL_MODE_ALTERNATE(config->myhc05config->rxalternatefunction));
            break;
        default:
            return EXIT_FAILURE;
            break;
    }
    return EXIT_SUCCESS;
}

/*!
 * \brief Sets up the RTS pin of the serial line
 *
 *
 * \param[in] config A BluetoothConfig object
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */
int hc05_setrtspin(BluetoothConfig *config){

    if(!config || !(config->myhc05config))
        return EXIT_FAILURE;

    switch (config->myhc05config->rtsport){

        case gpioa_port:
            palSetPadMode(GPIOA, config->myhc05config->rtspin,
                          PAL_MODE_ALTERNATE(config->myhc05config->rtsalternatefunction));
            break;
        case gpiob_port:
            palSetPadMode(GPIOB, config->myhc05config->rtspin,
                          PAL_MODE_ALTERNATE(config->myhc05config->rtsalternatefunction));
            break;
        case gpioc_port:
            palSetPadMode(GPIOC, config->myhc05config->rtspin,
                          PAL_MODE_ALTERNATE(config->myhc05config->rtsalternatefunction));
            break;
        case gpiod_port:
            palSetPadMode(GPIOD, config->myhc05config->rtspin,
                          PAL_MODE_ALTERNATE(config->myhc05config->rtsalternatefunction));
            break;
        case gpioe_port:
            palSetPadMode(GPIOE, config->myhc05config->rtspin,
                          PAL_MODE_ALTERNATE(config->myhc05config->rtsalternatefunction));
            break;
        case gpiof_port:
            palSetPadMode(GPIOF, config->myhc05config->rtspin,
                          PAL_MODE_ALTERNATE(config->myhc05config->rtsalternatefunction));
            break;
        case gpiog_port:
            palSetPadMode(GPIOG, config->myhc05config->rtspin,
                          PAL_MODE_ALTERNATE(config->myhc05config->rtsalternatefunction));
            break;
        case gpioh_port:
            palSetPadMode(GPIOH, config->myhc05config->rtspin,
                          PAL_MODE_ALTERNATE(config->myhc05config->rtsalternatefunction));
            break;
        default:
            return EXIT_FAILURE;
            break;
    }
    return EXIT_SUCCESS;
}


/*!
 * \brief Sets up the CTS pin of the serial line
 *
 *
 * \param[in] config A BluetoothConfig object
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */
int hc05_setctspin(BluetoothConfig *config){

    if(!config || !(config->myhc05config))
        return EXIT_FAILURE;

    switch (config->myhc05config->ctsport){

        case gpioa_port:
            palSetPadMode(GPIOA, config->myhc05config->ctspin,
                          PAL_MODE_ALTERNATE(config->myhc05config->ctsalternatefunction));
            break;
        case gpiob_port:
            palSetPadMode(GPIOB, config->myhc05config->ctspin,
                          PAL_MODE_ALTERNATE(config->myhc05config->ctsalternatefunction));
            break;
        case gpioc_port:
            palSetPadMode(GPIOC, config->myhc05config->ctspin,
                          PAL_MODE_ALTERNATE(config->myhc05config->ctsalternatefunction));
            break;
        case gpiod_port:
            palSetPadMode(GPIOD, config->myhc05config->ctspin,
                          PAL_MODE_ALTERNATE(config->myhc05config->ctsalternatefunction));
            break;
        case gpioe_port:
            palSetPadMode(GPIOE, config->myhc05config->ctspin,
                          PAL_MODE_ALTERNATE(config->myhc05config->ctsalternatefunction));
            break;
        case gpiof_port:
            palSetPadMode(GPIOF, config->myhc05config->ctspin,
                          PAL_MODE_ALTERNATE(config->myhc05config->ctsalternatefunction));
            break;
        case gpiog_port:
            palSetPadMode(GPIOG, config->myhc05config->ctspin,
                          PAL_MODE_ALTERNATE(config->myhc05config->ctsalternatefunction));
            break;
        case gpioh_port:
            palSetPadMode(GPIOH, config->myhc05config->ctspin,
                          PAL_MODE_ALTERNATE(config->myhc05config->ctsalternatefunction));
            break;
        default:
            return EXIT_FAILURE;
            break;
    }
    return EXIT_SUCCESS;
}

/*!
 * \brief Sets up the reset pin for the module
 *
 *
 * \param[in] config A BluetoothConfig object
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */
int hc05_setresetpin(BluetoothConfig *config){

    if(!config || !(config->myhc05config))
        return EXIT_FAILURE;

    switch (config->myhc05config->resetport){

        case gpioa_port:
            palSetPadMode(GPIOA, config->myhc05config->resetpin,
                          (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST));
            break;
        case gpiob_port:
            palSetPadMode(GPIOB, config->myhc05config->resetpin,
                          (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST));
            break;
        case gpioc_port:
            palSetPadMode(GPIOC, config->myhc05config->resetpin,
                          (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST));
            break;
        case gpiod_port:
            palSetPadMode(GPIOD, config->myhc05config->resetpin,
                          (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST));
            break;
        case gpioe_port:
            palSetPadMode(GPIOE, config->myhc05config->resetpin,
                          (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST));
            break;
        case gpiof_port:
            palSetPadMode(GPIOF, config->myhc05config->resetpin,
                          (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST));
            break;
        case gpiog_port:
            palSetPadMode(GPIOG, config->myhc05config->resetpin,
                          (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST));
            break;
        case gpioh_port:
            palSetPadMode(GPIOH, config->myhc05config->resetpin,
                          (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST));
            break;
        default:
            return EXIT_FAILURE;
            break;
    }
    return EXIT_SUCCESS;
}

/*!
 * \brief Sets up the key pin for the module
 *
 *
 * \param[in] config A BluetoothConfig object
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */
int hc05_setkeypin(BluetoothConfig *config){

    if(!config || !(config->myhc05config))
        return EXIT_FAILURE;

    switch (config->myhc05config->keyport){

        case gpioa_port:
            palSetPadMode(GPIOA, config->myhc05config->keypin,
                          (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST));
            break;
        case gpiob_port:
            palSetPadMode(GPIOB, config->myhc05config->keypin,
                          (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST));
            break;
        case gpioc_port:
            palSetPadMode(GPIOC, config->myhc05config->keypin,
                          (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST));
            break;
        case gpiod_port:
            palSetPadMode(GPIOD, config->myhc05config->keypin,
                          (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST));
            break;
        case gpioe_port:
            palSetPadMode(GPIOE, config->myhc05config->keypin,
                          (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST));
            break;
        case gpiof_port:
            palSetPadMode(GPIOF, config->myhc05config->keypin,
                          (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST));
            break;
        case gpiog_port:
            palSetPadMode(GPIOG, config->myhc05config->keypin,
                          (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST));
            break;
        case gpioh_port:
            palSetPadMode(GPIOH, config->myhc05config->keypin,
                          (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST));
            break;
        default:
            return EXIT_FAILURE;
            break;
    }
    return EXIT_SUCCESS;
}

#endif //HAL_USE_HC05 || defined(__DOXYGEN__)
 /** @} */
