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
#include "serial.h"
#include <string.h>

#if HAL_USE_HC05 || defined(__DOXYGEN__)

/*===========================================================================*/
/* Local variables                 .                                         */
/*===========================================================================*/

/*!
 * \brief SerialConfig to be used for communication with HC-05
 */
static SerialConfig hc05SerialConfig = {.sc_speed = 38400};

/*!
 * \brief current state of the driver / HC-05
 */
static volatile hc05_state_t hc05CurrentState = st_unknown;


/*===========================================================================*/
/* Threads                         .                                         */
/*===========================================================================*/


/*!
 * \brief Working area for hc05 send thread
 */
static WORKING_AREA(bthc05SendThreadWa, 128);


/*!
 *   \brief A thread that sends data over hc05 bluetooth module
 *
 *      Read the input queue (from the user application) and send everything through the serial interface
 *
 *   \param[in] instance A BluetoothDriver object
*/
static msg_t bthc05SendThread(void *instance) {

    if (!instance)
        chThdSleep(TIME_INFINITE);

    BluetoothDriver *drv = (BluetoothDriver *) instance;

    chRegSetThreadName("btSendThread");

    while (TRUE) {

        chThdSleepMilliseconds(drv->config->commSleepTimeMs);
        //check module state
        if ( hc05CurrentState != st_ready_communication )
            continue;

        if ( !chIQIsEmptyI(drv->btInputQueue) ){

            chnPutTimeout((BaseChannel *)drv->config->myhc05config->serialdriver, chIQGetTimeout(drv->btInputQueue, TIME_IMMEDIATE), TIME_INFINITE);
        }
      }

  return (msg_t) 0;
}

/*!
 * \brief Working area for hc05 recieve thread
 */
static WORKING_AREA(bthc05RecieveThreadWa, 128);


/*!
 *   \brief A thread that recieves data over hc05 bluetooth module
 *
 *      Read the serial interface for incoming data and store it in the output queue (that will be read by the application)
 *
 *   \param[in] instance A BluetoothDriver object
*/
static msg_t bthc05RecieveThread(void *instance) {

    if (!instance)
        chThdSleep(TIME_INFINITE);

    BluetoothDriver *drv = (BluetoothDriver *) instance;

    chRegSetThreadName("btRecieveThread");

    while (TRUE) {
        chThdSleepMilliseconds(drv->config->commSleepTimeMs);
        //check module state
        if ( hc05CurrentState != st_ready_communication )
            continue;

        if ( !chOQIsFullI(drv->btOutputQueue) ){

            chOQPut(drv->btOutputQueue, chnGetTimeout((BaseChannel *)drv->config->myhc05config->serialdriver, TIME_INFINITE));
        }
    }


  return (msg_t) 0;
}


/*===========================================================================*/
/* VMT functions                                                             */
/*===========================================================================*/

/*!
 * \brief Sends the given buffer
 *
 *  Writes bufferlength bytes from the buffer to the input queue, if there is enough space in it
 *
 * \param[in] instance A BluetoothDriver object
 * \param[in] buffer A pointer to a buffer to read from
 * \param[in] bufferlength The number of bytes to send
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */
int hc05sendBuffer(BluetoothDriver *instance, char *buffer, int bufferlength){

	if ( !instance || !buffer )
		return EXIT_FAILURE;
	if ( !bufferlength )
		return EXIT_SUCCESS;

	if ( bufferlength <=  (chQSizeI(drv->config->btInputQueue)-chQSpaceI(drv->config->btInputQueue)))
	{
		chOQWriteTimeout(drv->config->btInputQueue, buffer, bufferlength, TIME_INFINITE) == bufferlength
			? return EXIT_SUCCESS
			: return EXIT_FAILURE;
	}
	else
		return EXIT_FAILURE;

}

/*!
 * \brief Sends the given command byte by writing it to the input queue
 *
 * \param[in] instance A BluetoothDriver object
 * \param[in] commandByte A byte to send
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */
int hc05sendCommandByte(BluetoothDriver *instance, int commandByte){

	if ( !instance )
		return EXIT_FAILURE;

	if ( (chQSizeI(drv->config->btInputQueue)-chQSpaceI(drv->config->btInputQueue)) > 0)
	{
		chOQWriteTimeout(drv->config->btInputQueue, &commandbyte, 1, TIME_INFINITE) == 1
			? return EXIT_SUCCESS
			: return EXIT_FAILURE;
	}
	else
		return EXIT_FAILURE;
}


/*!
 * \brief Checks the input queue for incoming data
 *
 * \param[in] instance A BluetoothDriver object
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */
int hc05canRecieve(BluetoothDriver *instance){

	if ( !instance )
		return EXIT_FAILURE;

	chOQSpaceI(drv->config->btOutputQueue) > 0
			? return EXIT_SUCCESS
			: return EXIT_FAILURE;
}

/*!
 * \brief Reads from the input queue
 *
 *  Reads maxlength bytes (or less, if there is no more) from the output queue and puts it in the specified buffer to be processed by the applicaton
 *
 * \param[in] instance A BluetoothDriver object
 * \param[in] buffer A pointer to a buffer to write into
 * \param[in] maxlength The maximum number of bytes to read (size of the buffer)
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */
int hc05readBuffer(BluetoothDriver *instance, char *buffer, int maxlength){

	if ( !instance || !buffer )
		return EXIT_FAILURE;
	if ( !bufferlength )
		return EXIT_SUCCESS;

	chIQReadTimeout (drv->config->btOutputQueue, buffer, maxlength, TIME_IMMEDIATE) > 0
	? return EXIT_SUCCESS
	: return EXIT_SUCCESS;

}

/*!
*	\brief Sends an AT command
*
*	\param[in] instance A BluetoothDriver object
*	\param[in] command AT command to use. Must be '\0' terminated string
*	\return EXIT_SUCCESS or EXIT_FAILURE
*/
int hc05sendAtCommand(BluetoothDriver *instance, char* command){

	if ( !instance || !command )
		return EXIT_FAILURE;

    // allocate memory from the heap
	char *commandBuffer = chHeapAlloc(NULL, strlen(command)+4);

	if(!commandBuffer)
		return EXIT_FAILURE;

	if (hc05CurrentState != st_ready_at_command)
	{
		hc05CurrentState = st_unknown;
		//enter AT mode here, but wait for threads to detect state change
		chThdSleepMilliseconds(instance->config->commSleepTimeMs);
        hc05SetModeAt(instance->config, 100);

	}
	int commandLength = strlen(command);

    strncpy(commandBuffer, "\r\n", 2);
    strncpy(commandBuffer + 2, command, commandLength );
    strncpy(commandBuffer + 2 + commandLength, "\r\n", 2 );

    chnWrite((BaseChannel *)drv->config->myhc05config->serialdriver,
                &hc05_at_message,
                commandLength + 4);
    chHeapFree(commandBuffer);

    hc05SetModeComm(instance->config, 100);

	return EXIT_SUCCESS;
}


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
int hc05setPinCode(BluetoothDriver *instance, char *pin, int pinlength){

	if ( !instance || !pin )
		return EXIT_FAILURE;

    char Command[] = "AT+PIN=";
    char *CmdBuf = chHeapAlloc(NULL, strlen(Command) + pinlength + 1);
    if(!CmdBuf)
		return EXIT_FAILURE;

    strcpy(CmdBuf, Command);
    strncpy(CmdBuf, pin, pinlength);
    //must terminate the string with a \0
    *(CmdBuf+strlen(Command)+pinlength+1) = '\0';

    if ( (hc05sendAtCommand(instance, CmdBuf) == EXIT_FAILURE)
        return EXIT_FAILURE;

    chHeapFree(CmdBuf);
    return EXIT_SUCCESS;
}

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
int hc05setName(BluetoothDriver *instance, char *newname, int namelength){

    if ( !instance || !newname )
		return EXIT_FAILURE;

    char Command[] = "AT+NAME=";
    char *CmdBuf = chHeapAlloc(NULL, strlen(Command) + namelength + 1);
    if(!CmdBuf)
		return EXIT_FAILURE;

    strcpy(CmdBuf, Command);
    strncpy(CmdBuf, newname, namelength);
    //must terminate the string with a \0
    *(CmdBuf+strlen(Command)+namelength+1) = '\0';

    if ( (hc05sendAtCommand(instance, CmdBuf) == EXIT_FAILURE)
        return EXIT_FAILURE;

    chHeapFree(CmdBuf);
    return EXIT_SUCCESS;
}

/*!
 * \brief Default reset for HC-05 module
 *
 *  When in AT mode, we can reset the settings of the module.
 *
 * \param[in] instance A BluetoothDriver object
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */
int hc05resetDefaults(BluetoothDriver *instance){

    if ( !instance || !newname )
		return EXIT_FAILURE;

    char Command[] = "AT+ORGL";
    char *CmdBuf = chHeapAlloc(NULL, strlen(Command) + 1);
    if(!CmdBuf)
		return EXIT_FAILURE;

    strcpy(CmdBuf, Command);
    *(CmdBuf+strlen(Command)+1) = '\0';

    if ( (hc05sendAtCommand(instance, CmdBuf) == EXIT_FAILURE)
        return EXIT_FAILURE;

    chHeapFree(CmdBuf);
    return EXIT_SUCCESS;
}


/*!
 * \brief Starts the driver
 *
 *  Read config
 *  Set the apropriate port/pin settings
 *  Set the name/pin according to the config
 *  Get the in/out buffers ready
 *  Create buffer threads
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

    //flag
    hc05CurrentState = st_initializing;
    // set config location
    instance->config = config;
    //set up the key and reset pins... using external functions
    hc05_setkeypin(config);
    hc05_setresetpin(config);
    //set up the RX and TX pins
    hc05_settxpin(config);
    hc05_setrxpin(config);
    //set up the RTS and CTS pins
    hc05_setrtspin(config);
    hc05_setctspin(config);

    //buffers
    chSysLock();
    chOQResetI(instance->btOutputQueue);
    chIQResetI(instance->btOutputQueue);
    chSysUnlock();
    //threads
    //create driverThread, but do not start it yet
    config->sendThread=chThdCreateI(bthc05SendThreadWa, sizeof(bthc05SendThreadWa), NORMALPRIO, bthc05SendThread, instance);
    config->recieveThread=chThdCreateI(bthc05RecieveThreadWa, sizeof(bthc05RecieveThreadWa), NORMALPRIO, bthc05RecieveThread, instance);

    //serial driver
    hc05_updateserialconfig(config);
    hc05_startserial(config);

    //flag
    hc05CurrentState = st_ready_communication;


    //set default name, pin, other AT dependent stuff here
    hc05setName(instance, "Pumukli", strlen("Pumukli"));
    hc05setPin(instance, "1234", strlen("1234"));


    //return to communication mode
    hc05SetModeComm(config, 100);
    //start threads
    chThdResume(config->sendThread);
    chThdResume(config->recieveThread);


    return EXIT_SUCCESS;
}

/*!
 * \brief Stops the driver
 *
 * Clean up the communications channel and stop the driver.
 *
 *  Stop threads (set stop flag)
 *  Stop serial
 *  Empty buffers
 *
 *
 *
 * \param[in] instance A BluetoothDriver object
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */
int hc05close(BluetoothDriver *instance){

    if(!instance)
        return EXIT_FAILURE;

    //flag --> threads will stop
    hc05CurrentState = st_shutting_down;
    chThdSleepMilliseconds(100);
    //stop serial driver
    hc05_stopserial(instance->config);
    //empty buffers
    chSysLock();
    chOQResetI(instance->btOutputQueue);
    chIQResetI(instance->btOutputQueue);
    chSysUnlock();

    return EXIT_SUCCESS;
}

/*===========================================================================*/
/* VMT                                                                       */
/*===========================================================================*/


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
    .close = hc05close,
    .resetModuleSettings = hc05resetDefaults
};

/*===========================================================================*/
/* Internal functions                                                        */
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
                          config->myhc05config->txalternatefunction < 0
                          ? (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST)
                          : PAL_MODE_ALTERNATE(config->myhc05config->txalternatefunction));
            break;
        case gpiob_port:
            palSetPadMode(GPIOB, config->myhc05config->txpin,
                          config->myhc05config->txalternatefunction < 0
                          ? (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST)
                          : PAL_MODE_ALTERNATE(config->myhc05config->txalternatefunction));
            break;
        case gpioc_port:
            palSetPadMode(GPIOC, config->myhc05config->txpin,
                          config->myhc05config->txalternatefunction < 0
                          ? (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST)
                          : PAL_MODE_ALTERNATE(config->myhc05config->txalternatefunction));
            break;
        case gpiod_port:
            palSetPadMode(GPIOD, config->myhc05config->txpin,
                          config->myhc05config->txalternatefunction < 0
                          ? (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST)
                          : PAL_MODE_ALTERNATE(config->myhc05config->txalternatefunction));
            break;
        case gpioe_port:
            palSetPadMode(GPIOE, config->myhc05config->txpin,
                          config->myhc05config->txalternatefunction < 0
                          ? (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST)
                          : PAL_MODE_ALTERNATE(config->myhc05config->txalternatefunction));
            break;
        case gpiof_port:
            palSetPadMode(GPIOF, config->myhc05config->txpin,
                          config->myhc05config->txalternatefunction < 0
                          ? (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST)
                          : PAL_MODE_ALTERNATE(config->myhc05config->txalternatefunction));
            break;
        case gpiog_port:
            palSetPadMode(GPIOG, config->myhc05config->txpin,
                          config->myhc05config->txalternatefunction < 0
                          ? (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST)
                          : PAL_MODE_ALTERNATE(config->myhc05config->txalternatefunction));
            break;
        case gpioh_port:
            palSetPadMode(GPIOH, config->myhc05config->txpin,
                          config->myhc05config->txalternatefunction < 0
                          ? (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST)
                          : PAL_MODE_ALTERNATE(config->myhc05config->txalternatefunction));
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
                          config->myhc05config->rxalternatefunction < 0
                          ? (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST)
                          : PAL_MODE_ALTERNATE(config->myhc05config->rxalternatefunction));
            break;
        case gpiob_port:
            palSetPadMode(GPIOB, config->myhc05config->rxpin,
                          config->myhc05config->rxalternatefunction < 0
                          ? (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST)
                          : PAL_MODE_ALTERNATE(config->myhc05config->rxalternatefunction));
            break;
        case gpioc_port:
            palSetPadMode(GPIOC, config->myhc05config->rxpin,
                          config->myhc05config->rxalternatefunction < 0
                          ? (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST)
                          : PAL_MODE_ALTERNATE(config->myhc05config->rxalternatefunction));
            break;
        case gpiod_port:
            palSetPadMode(GPIOD, config->myhc05config->rxpin,
                          config->myhc05config->rxalternatefunction < 0
                          ? (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST)
                          : PAL_MODE_ALTERNATE(config->myhc05config->rxalternatefunction));
            break;
        case gpioe_port:
            palSetPadMode(GPIOE, config->myhc05config->rxpin,
                          config->myhc05config->rxalternatefunction < 0
                          ? (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST)
                          : PAL_MODE_ALTERNATE(config->myhc05config->rxalternatefunction));
            break;
        case gpiof_port:
            palSetPadMode(GPIOF, config->myhc05config->rxpin,
                          config->myhc05config->rxalternatefunction < 0
                          ? (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST)
                          : PAL_MODE_ALTERNATE(config->myhc05config->rxalternatefunction));
            break;
        case gpiog_port:
            palSetPadMode(GPIOG, config->myhc05config->rxpin,
                          config->myhc05config->rxalternatefunction < 0
                          ? (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST)
                          : PAL_MODE_ALTERNATE(config->myhc05config->rxalternatefunction));
            break;
        case gpioh_port:
            palSetPadMode(GPIOH, config->myhc05config->rxpin,
                          config->myhc05config->rxalternatefunction < 0
                          ? (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST)
                          : PAL_MODE_ALTERNATE(config->myhc05config->rxalternatefunction));
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
                          config->myhc05config->rtsalternatefunction < 0
                          ? (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST)
                          : PAL_MODE_ALTERNATE(config->myhc05config->rtsalternatefunction));
            break;
        case gpiob_port:
            palSetPadMode(GPIOB, config->myhc05config->rtspin,
                          config->myhc05config->rtsalternatefunction < 0
                          ? (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST)
                          : PAL_MODE_ALTERNATE(config->myhc05config->rtsalternatefunction));
            break;
        case gpioc_port:
            palSetPadMode(GPIOC, config->myhc05config->rtspin,
                          config->myhc05config->rtsalternatefunction < 0
                          ? (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST)
                          : PAL_MODE_ALTERNATE(config->myhc05config->rtsalternatefunction));
            break;
        case gpiod_port:
            palSetPadMode(GPIOD, config->myhc05config->rtspin,
                          config->myhc05config->rtsalternatefunction < 0
                          ? (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST)
                          : PAL_MODE_ALTERNATE(config->myhc05config->rtsalternatefunction));
            break;
        case gpioe_port:
            palSetPadMode(GPIOE, config->myhc05config->rtspin,
                          config->myhc05config->rtsalternatefunction < 0
                          ? (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST)
                          : PAL_MODE_ALTERNATE(config->myhc05config->rtsalternatefunction));
            break;
        case gpiof_port:
            palSetPadMode(GPIOF, config->myhc05config->rtspin,
                          config->myhc05config->rtsalternatefunction < 0
                          ? (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST)
                          : PAL_MODE_ALTERNATE(config->myhc05config->rtsalternatefunction));
            break;
        case gpiog_port:
            palSetPadMode(GPIOG, config->myhc05config->rtspin,
                          config->myhc05config->rtsalternatefunction < 0
                          ? (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST)
                          : PAL_MODE_ALTERNATE(config->myhc05config->rtsalternatefunction));
            break;
        case gpioh_port:
            palSetPadMode(GPIOH, config->myhc05config->rtspin,
                          config->myhc05config->rtsalternatefunction < 0
                          ? (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST)
                          : PAL_MODE_ALTERNATE(config->myhc05config->rtsalternatefunction));
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
                          config->myhc05config->ctsalternatefunction < 0
                          ? (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST)
                          : PAL_MODE_ALTERNATE(config->myhc05config->ctsalternatefunction));
            break;
        case gpiob_port:
            palSetPadMode(GPIOB, config->myhc05config->ctspin,
                          config->myhc05config->ctsalternatefunction < 0
                          ? (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST)
                          : PAL_MODE_ALTERNATE(config->myhc05config->ctsalternatefunction));
            break;
        case gpioc_port:
            palSetPadMode(GPIOC, config->myhc05config->ctspin,
                          config->myhc05config->ctsalternatefunction < 0
                          ? (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST)
                          : PAL_MODE_ALTERNATE(config->myhc05config->ctsalternatefunction));
            break;
        case gpiod_port:
            palSetPadMode(GPIOD, config->myhc05config->ctspin,
                          config->myhc05config->ctsalternatefunction < 0
                          ? (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST)
                          : PAL_MODE_ALTERNATE(config->myhc05config->ctsalternatefunction));
            break;
        case gpioe_port:
            palSetPadMode(GPIOE, config->myhc05config->ctspin,
                          config->myhc05config->ctsalternatefunction < 0
                          ? (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST)
                          : PAL_MODE_ALTERNATE(config->myhc05config->ctsalternatefunction));
            break;
        case gpiof_port:
            palSetPadMode(GPIOF, config->myhc05config->ctspin,
                          config->myhc05config->ctsalternatefunction < 0
                          ? (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST)
                          : PAL_MODE_ALTERNATE(config->myhc05config->ctsalternatefunction));
            break;
        case gpiog_port:
            palSetPadMode(GPIOG, config->myhc05config->ctspin,
                          config->myhc05config->ctsalternatefunction < 0
                          ? (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST)
                          : PAL_MODE_ALTERNATE(config->myhc05config->ctsalternatefunction));
            break;
        case gpioh_port:
            palSetPadMode(GPIOH, config->myhc05config->ctspin,
                          config->myhc05config->ctsalternatefunction < 0
                          ? (PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST)
                          : PAL_MODE_ALTERNATE(config->myhc05config->ctsalternatefunction));
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

/*!
 * \brief Updates the SerialConfig from the BluetoothConfig (change of baud rate)
 *
 *
 * \param[in] config A BluetoothConfig object
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */
int hc05_updateserialconfig(BluetoothConfig *config){

    if(!config || !(config->myhc05config))
        return EXIT_FAILURE;

    switch (config->btbaudrate) {

        case b1200:
            hc05SerialConfig.sc_speed = 1200;
            break;
        case b2400:
            hc05SerialConfig.sc_speed = 2400;
            break;
        case b4800:
            hc05SerialConfig.sc_speed = 4800;
            break;
        case b9600:
            hc05SerialConfig.sc_speed = 9600;
            break;
        case b19200:
            hc05SerialConfig.sc_speed = 19200;
            break;
        case b38400:
            hc05SerialConfig.sc_speed = 38400;
            break;
        case b57600:
            hc05SerialConfig.sc_speed = 57600;
            break;
        case b115200:
            hc05SerialConfig.sc_speed = 115200;
            break;
        default:
            hc05SerialConfig.sc_speed = BLUETOOTH_DEFAULT_BITRATE;
            break;
    }
    return EXIT_SUCCESS;
}


/*!
 * \brief Starts the given serial driver
 *
 *
 * \param[in] config A BluetoothConfig object
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */
int hc05_startserial(BluetoothConfig *config){

    if(!config || !(config->myhc05config))
        return EXIT_FAILURE;

    switch (config->myhc05config->serialdriver) {

        case sd1:
            sdStart(&SD1, &hc05SerialConfig);
            break;
        case sd2:
            sdStart(&SD2, &hc05SerialConfig);
            break;
        case sd3:
            sdStart(&SD3, &hc05SerialConfig);
            break;
        case sd4:
            sdStart(&SD4, &hc05SerialConfig);
            break;
        case sd5:
            sdStart(&SD5, &hc05SerialConfig);
            break;
        default:
            return EXIT_FAILURE;
            break;
    }
    return EXIT_SUCCESS;
}


/*!
 * \brief Starts the given serial driver
 *
 *
 * \param[in] config A BluetoothConfig object
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */
int hc05_stopserial(BluetoothConfig *config){

    if(!config || !(config->myhc05config))
        return EXIT_FAILURE;

    switch (config->myhc05config->serialdriver) {

        case sd1:
            sdStop(&SD1);
            break;
        case sd2:
            sdStop(&SD2);
            break;
        case sd3:
            sdStop(&SD3);
            break;
        case sd4:
            sdStop(&SD4);
            break;
        case sd5:
            sdStop(&SD5);
            break;
        default:
            return EXIT_FAILURE;
            break;
    }
    return EXIT_SUCCESS;
}

/*!
 * \brief Enters HC05 to AT command mode
 *
 *
 * \param[in] config A BluetoothConfig object
 * \param[in] timeout Time to wait in milliseconds
 */
void hc05SetModeAt(BluetoothConfig *config, uint16_t timeout){

    if(!config)
        return;

    //reset module (low), pull key high
    chSysLock();
    palSetPad((((config->myhc05config->keyport) == gpioa_port) ? GPIOA :
              ((config->myhc05config->keyport) == gpiob_port) ? GPIOB :
              ((config->myhc05config->keyport) == gpioc_port) ? GPIOC :
              ((config->myhc05config->keyport) == gpiod_port) ? GPIOD :
              ((config->myhc05config->keyport) == gpioe_port) ? GPIOE :
              ((config->myhc05config->keyport) == gpiof_port) ? GPIOF :
              ((config->myhc05config->keyport) == gpiog_port) ? GPIOG :
              ((config->myhc05config->keyport) == gpioh_port) ? GPIOH :
              NULL),
              (config->myhc05config->keypin));
    chSysUnlock();

    chThdSleepMilliseconds(timeout);

    chSysLock();
    palClearPad((((config->myhc05config->resetport) == gpioa_port) ? GPIOA :
              ((config->myhc05config->resetport) == gpiob_port) ? GPIOB :
              ((config->myhc05config->resetport) == gpioc_port) ? GPIOC :
              ((config->myhc05config->resetport) == gpiod_port) ? GPIOD :
              ((config->myhc05config->resetport) == gpioe_port) ? GPIOE :
              ((config->myhc05config->resetport) == gpiof_port) ? GPIOF :
              ((config->myhc05config->resetport) == gpiog_port) ? GPIOG :
              ((config->myhc05config->resetport) == gpioh_port) ? GPIOH :
              NULL),
              (config->myhc05config->resetpin));
    chSysUnlock();


    chThdSleepMilliseconds(timeout);   //wait for reset
    chSysLock();
    palSetPad((((config->myhc05config->resetport) == gpioa_port) ? GPIOA :
              ((config->myhc05config->resetport) == gpiob_port) ? GPIOB :
              ((config->myhc05config->resetport) == gpioc_port) ? GPIOC :
              ((config->myhc05config->resetport) == gpiod_port) ? GPIOD :
              ((config->myhc05config->resetport) == gpioe_port) ? GPIOE :
              ((config->myhc05config->resetport) == gpiof_port) ? GPIOF :
              ((config->myhc05config->resetport) == gpiog_port) ? GPIOG :
              ((config->myhc05config->resetport) == gpioh_port) ? GPIOH :
              NULL),
              (config->myhc05config->resetpin));
    chSysUnlock();

    chThdSleepMilliseconds(timeout);   //wait for module recovery
    //we should be in AT mode, with 38400 baud
    hc05CurrentState = st_ready_at_command;
};


/*!
 * \brief Enters HC05 to communication mode
 *
 *
 * \param[in] config A BluetoothConfig object
 * \param[in] timeout Time to wait in milliseconds
 */
void hc05SetModeComm(BluetoothConfig *config, uint16_t timeout){

    if(!config)
        return;

    //reset module (low), pull key high
    chSysLock();
    palClearPad((((config->myhc05config->keyport) == gpioa_port) ? GPIOA :
              ((config->myhc05config->keyport) == gpiob_port) ? GPIOB :
              ((config->myhc05config->keyport) == gpioc_port) ? GPIOC :
              ((config->myhc05config->keyport) == gpiod_port) ? GPIOD :
              ((config->myhc05config->keyport) == gpioe_port) ? GPIOE :
              ((config->myhc05config->keyport) == gpiof_port) ? GPIOF :
              ((config->myhc05config->keyport) == gpiog_port) ? GPIOG :
              ((config->myhc05config->keyport) == gpioh_port) ? GPIOH :
              NULL),
              (config->myhc05config->keypin));
    chSysUnlock();

    chThdSleepMilliseconds(timeout);

    chSysLock();
    palClearPad((((config->myhc05config->resetport) == gpioa_port) ? GPIOA :
              ((config->myhc05config->resetport) == gpiob_port) ? GPIOB :
              ((config->myhc05config->resetport) == gpioc_port) ? GPIOC :
              ((config->myhc05config->resetport) == gpiod_port) ? GPIOD :
              ((config->myhc05config->resetport) == gpioe_port) ? GPIOE :
              ((config->myhc05config->resetport) == gpiof_port) ? GPIOF :
              ((config->myhc05config->resetport) == gpiog_port) ? GPIOG :
              ((config->myhc05config->resetport) == gpioh_port) ? GPIOH :
              NULL),
              (config->myhc05config->resetpin));
    chSysUnlock();


    chThdSleepMilliseconds(timeout);   //wait for reset
    chSysLock();
    palSetPad((((config->myhc05config->resetport) == gpioa_port) ? GPIOA :
              ((config->myhc05config->resetport) == gpiob_port) ? GPIOB :
              ((config->myhc05config->resetport) == gpioc_port) ? GPIOC :
              ((config->myhc05config->resetport) == gpiod_port) ? GPIOD :
              ((config->myhc05config->resetport) == gpioe_port) ? GPIOE :
              ((config->myhc05config->resetport) == gpiof_port) ? GPIOF :
              ((config->myhc05config->resetport) == gpiog_port) ? GPIOG :
              ((config->myhc05config->resetport) == gpioh_port) ? GPIOH :
              NULL),
              (config->myhc05config->resetpin));
    chSysUnlock();

    chThdSleepMilliseconds(timeout);   //wait for module recovery
    //we should be in AT mode, with 38400 baud
    hc05CurrentState = st_ready_communication;
};

#endif //HAL_USE_HC05 || defined(__DOXYGEN__)
 /** @} */
