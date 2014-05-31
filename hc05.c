/*!
 * @file hc05.c
 * @brief Source file for HC-05 SPP device for bluetooth module in ChibiosRT.
 *
 * @addtogroup BLUETOOTH
 * @{
 */

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "shell.h"
#include "chqueues.h"
#include "bluetooth.h"
#include "hc05.h"
#include "serial.h"
#include "serial_lld.h"
#include "mcuconf.h"
#include <string.h>


#if HAL_USE_HC_05_BLUETOOTH || defined(__DOXYGEN__)


extern SerialUSBDriver SDU1;

/*===========================================================================*/
/* Local variables                 .                                         */
/*===========================================================================*/

/*!
 * \brief SerialConfig to be used for communication with HC-05
 */
static SerialConfig hc05SerialConfig = {38400,0,0,0};


/*!
 * \brief current state of the driver / HC-05
 */
static volatile enum hc05_state_t hc05CurrentState = st_unknown;


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

    struct BluetoothDriver *drv = (struct BluetoothDriver *) instance;

    chRegSetThreadName("btSendThread");

    while (TRUE) {

        chThdSleepMilliseconds(drv->commSleepTimeMs);
        //check module state
        if ( hc05CurrentState != st_ready_communication )
            continue;

        if ( !chIQIsEmptyI(drv->btInputQueue) || 1){

            chprintf(&SDU1, "?");
            sdPutTimeout(drv->config->myhc05config->hc05serialpointer, chIQGetTimeout(drv->btInputQueue, TIME_IMMEDIATE), TIME_IMMEDIATE);
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

    struct BluetoothDriver *drv = (struct BluetoothDriver *) instance;

    chRegSetThreadName("btRecieveThread");

    while (TRUE) {
        chThdSleepMilliseconds(drv->commSleepTimeMs);

        //check module state
        if ( hc05CurrentState != st_ready_communication )
            continue;

        if (!chOQIsEmptyI(drv->btOutputQueue) || 1)
        {
            chprintf(&SDU1, "!");
            chOQPutTimeout(drv->btOutputQueue, sdGetTimeout(drv->config->myhc05config->hc05serialpointer, TIME_IMMEDIATE),TIME_IMMEDIATE);
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
int hc05sendBuffer(struct BluetoothDriver *instance, char *buffer, unsigned int bufferlength){

	if ( !instance || !buffer )
		return EXIT_FAILURE;
	if ( !bufferlength )
		return EXIT_SUCCESS;
/*
	if ( bufferlength <=  (chQSizeI(instance->btInputQueue)-chQSpaceI(instance->btInputQueue)))
	{
		return (chOQWriteTimeout(instance->btInputQueue, buffer, bufferlength, TIME_IMMEDIATE) == bufferlength)
			? EXIT_SUCCESS
			: EXIT_FAILURE;
	}
	else
		return EXIT_FAILURE;


*/

    return sdWriteTimeout(instance->config->myhc05config->hc05serialpointer, buffer, bufferlength, TIME_IMMEDIATE) > 0
            ? EXIT_SUCCESS
            : EXIT_FAILURE;

}

/*!
 * \brief Sends the given command byte by writing it to the input queue
 *
 * \param[in] instance A BluetoothDriver object
 * \param[in] commandByte A byte to send
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */
int hc05sendCommandByte(struct BluetoothDriver *instance, int commandByte){

	if ( !instance )
		return EXIT_FAILURE;

    /*
	if ( (chQSizeI(instance->btInputQueue)-chQSpaceI(instance->btInputQueue)) > 0)
	{
		return (chOQPutTimeout(instance->btInputQueue, commandByte, TIME_IMMEDIATE) == 1)
			? EXIT_SUCCESS
			: EXIT_FAILURE;
	}
	else
		return EXIT_FAILURE;
    */

    return sdPut(instance->config->myhc05config->hc05serialpointer, commandByte);
}


/*!
 * \brief Checks the input queue for incoming data
 *
 * \param[in] instance A BluetoothDriver object
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */
int hc05canRecieve(struct BluetoothDriver *instance){

	if ( !instance )
		return EXIT_FAILURE;

    /*
	return (chQSpaceI(instance->btOutputQueue) > 0)
			? EXIT_SUCCESS
			: EXIT_FAILURE;
			*/

	return sdGetWouldBlock(instance->config->myhc05config->hc05serialpointer);

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
int hc05readBuffer(struct BluetoothDriver *instance, unsigned char *buffer, int maxlength){

	if ( !instance || !buffer )
		return EXIT_FAILURE;
	if ( !maxlength )
		return EXIT_SUCCESS;

    /*
	return (chIQReadTimeout (instance->btOutputQueue, buffer, maxlength, TIME_IMMEDIATE) > 0)
	? EXIT_SUCCESS
	: EXIT_SUCCESS;

	*/

	return sdReadTimeout(instance->config->myhc05config->hc05serialpointer, buffer, maxlength, TIME_IMMEDIATE) > 0
            ? EXIT_SUCCESS
            : EXIT_FAILURE;

}

/*!
*	\brief Sends an AT command
*
*	\param[in] instance A BluetoothDriver object
*	\param[in] command AT command to use. Must be '\0' terminated string
*	\return EXIT_SUCCESS or EXIT_FAILURE
*/
int hc05sendAtCommand(struct BluetoothDriver *instance, char* command){

	if ( !instance || !command )
		return EXIT_FAILURE;


	if (hc05CurrentState != st_ready_at_command)
	{
		hc05CurrentState = st_unknown;
		//enter AT mode here, but wait for threads to detect state change

        hc05SetModeAt(instance->config, 200);
        //chThdSleepMilliseconds(instance->commSleepTimeMs);
        chThdSleepMilliseconds(500);
	}


    sdWrite(instance->config->myhc05config->hc05serialpointer, command, strlen(command));
    sdWrite(instance->config->myhc05config->hc05serialpointer,"\r\n",2);

    chThdSleepMilliseconds(1000);

    hc05SetModeComm(instance->config, 200);

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
int hc05setPinCode(struct BluetoothDriver *instance, char *pin, int pinlength){

	if ( !instance || !pin )
		return EXIT_FAILURE;

    char Command[] = "AT+PIN=";
    int cmdLen = strlen(Command);
    int bufLen = cmdLen + pinlength + 1;

    char *CmdBuf = chHeapAlloc(NULL, bufLen);

    if(!CmdBuf)
		return EXIT_FAILURE;
    else
        memset(CmdBuf, '\0', bufLen);

    strcpy(CmdBuf, Command);
    strncpy(CmdBuf+cmdLen, pin, pinlength);
    //must terminate the string with a \0
    *(CmdBuf+bufLen-1) = '\0';

    if ( (hc05sendAtCommand(instance, CmdBuf) == EXIT_FAILURE))
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
int hc05setName(struct BluetoothDriver *instance, char *newname, int namelength){

    if ( !instance || !newname )
		return EXIT_FAILURE;

    char Command[] = "AT+NAME=";
    int cmdLen = strlen(Command);
    int bufLen = cmdLen + namelength + 1;

    char *CmdBuf = chHeapAlloc(NULL, bufLen);

    if(!CmdBuf)
		return EXIT_FAILURE;
    else
        memset(CmdBuf, '\0', bufLen);

    strcpy(CmdBuf, Command);
    strncpy(CmdBuf+cmdLen, newname, namelength);
    //must terminate the string with a \0
    *(CmdBuf+bufLen-1) = '\0';

    if ( (hc05sendAtCommand(instance, CmdBuf) == EXIT_FAILURE))
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
int hc05resetDefaults(struct BluetoothDriver *instance){

    if ( !instance )
		return EXIT_FAILURE;

    char Command[] = "AT+ORGL";
    char *CmdBuf = chHeapAlloc(NULL, strlen(Command) + 1);
    if(!CmdBuf)
		return EXIT_FAILURE;

    strcpy(CmdBuf, Command);
    *(CmdBuf+strlen(Command)+1) = '\0';

    if ( (hc05sendAtCommand(instance, CmdBuf) == EXIT_FAILURE))
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
int hc05open(struct BluetoothDriver *instance, struct  BluetoothConfig *config){

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
/*
    hc05_setrtspin(config);
    hc05_setctspin(config);
*/
    //buffers
/*
    chOQResetI(instance->btOutputQueue);
    chIQResetI(instance->btInputQueue);

    //threads
    //create driverThread, but do not start it yet
    config->sendThread=chThdCreateI(bthc05SendThreadWa, sizeof(bthc05SendThreadWa), NORMALPRIO, bthc05SendThread, instance);
    config->recieveThread=chThdCreateI(bthc05RecieveThreadWa, sizeof(bthc05RecieveThreadWa), NORMALPRIO, bthc05RecieveThread, instance);
*/
    //serial driver
    hc05_updateserialconfig(config);
    hc05_startserial(config);

    //flag
    hc05CurrentState = st_ready_communication;

/*
    //set default name, pin, other AT dependent stuff here
    hc05setName(instance, "Wait", strlen("Wait"));

    hc05setPinCode(instance, "1234", strlen("1234"));
*/

    //return to communication mode
    hc05SetModeComm(config, 200);
/*
    //start threads
    chThdResume(config->sendThread);
    chThdResume(config->recieveThread);
*/

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
int hc05close(struct BluetoothDriver *instance){

    if(!instance)
        return EXIT_FAILURE;

    //flag --> threads will stop
    hc05CurrentState = st_shutting_down;
    chThdSleepMilliseconds(100);
    //stop serial driver
    hc05_stopserial(instance->config);
    //empty buffers

    chOQResetI(instance->btOutputQueue);
    chIQResetI(instance->btOutputQueue);


    return EXIT_SUCCESS;
}

/*===========================================================================*/
/* VMT                                                                       */
/*===========================================================================*/

/**
 * @brief HC05 BluetoothDriver virtual methods table.
 */
struct BluetoothDeviceVMT hc05BtDevVMT = {
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
int hc05_settxpin(struct BluetoothConfig *config){

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
int hc05_setrxpin(struct BluetoothConfig *config){

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
int hc05_setrtspin(struct BluetoothConfig *config){

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
int hc05_setctspin(struct BluetoothConfig *config){

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
int hc05_setresetpin(struct BluetoothConfig *config){

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
int hc05_setkeypin(struct BluetoothConfig *config){

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
int hc05_updateserialconfig(struct BluetoothConfig *config){

    if(!config || !(config->myhc05config))
        return EXIT_FAILURE;

    switch (config->baudrate) {

        case b1200:
            hc05SerialConfig.speed = 1200;
            break;
        case b2400:
            hc05SerialConfig.speed = 2400;
            break;
        case b4800:
            hc05SerialConfig.speed = 4800;
            break;
        case b9600:
            hc05SerialConfig.speed = 9600;
            break;
        case b19200:
            hc05SerialConfig.speed = 19200;
            break;
        case b38400:
            hc05SerialConfig.speed = 38400;
            break;
        case b57600:
            hc05SerialConfig.speed = 57600;
            break;
        case b115200:
            hc05SerialConfig.speed = 115200;
            break;
        default:
            hc05SerialConfig.speed = BLUETOOTH_DEFAULT_BITRATE;
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
int hc05_startserial(struct BluetoothConfig *config){

    if(!config || !(config->myhc05config))
        return EXIT_FAILURE;

    switch (config->myhc05config->serialdriver) {

#if STM32_SERIAL_USE_USART1 == TRUE
        case sd1:
            config->myhc05config->hc05serialpointer = &SD1;
            sdStart(&SD1, &hc05SerialConfig);
            break;
#endif

#if STM32_SERIAL_USE_USART2 == TRUE
        case sd2:
            config->myhc05config->hc05serialpointer = &SD2;
            sdStart(&SD2, &hc05SerialConfig);
            break;
#endif

#if STM32_SERIAL_USE_USART3 == TRUE
        case sd3:
            config->myhc05config->hc05serialpointer = &SD3;
            sdStart(&SD3, &hc05SerialConfig);
            break;
#endif

#if STM32_SERIAL_USE_UART4 == TRUE
        case sd4:
            config->myhc05config->hc05serialpointer = &SD4;
            sdStart(&SD4, &hc05SerialConfig);
            break;
#endif

#if STM32_SERIAL_USE_UART5 == TRUE
        case sd5:
            config->myhc05config->hc05serialpointer = &SD5;
            sdStart(&SD5, &hc05SerialConfig);
            break;
#endif
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
int hc05_stopserial(struct BluetoothConfig *config){

    if(!config || !(config->myhc05config))
        return EXIT_FAILURE;

    switch (config->myhc05config->serialdriver) {

#if STM32_SERIAL_USE_USART1 == TRUE
        case sd1:
            sdStop(&SD1);
            break;
#endif
#if STM32_SERIAL_USE_USART2 == TRUE
        case sd2:
            sdStop(&SD2);
            break;
#endif
#if STM32_SERIAL_USE_USART3 == TRUE
        case sd3:
            sdStop(&SD3);
            break;
#endif
#if STM32_SERIAL_USE_UART4 == TRUE
        case sd4:
            sdStop(&SD4);
            break;
#endif
#if STM32_SERIAL_USE_UART5 == TRUE
        case sd5:
            sdStop(&SD5);
            break;
#endif
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
void hc05SetModeAt(struct BluetoothConfig *config, uint16_t timeout){

    if(!config)
        return;

    //reset module (low), pull key high

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


    chThdSleepMilliseconds(timeout);


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



    chThdSleepMilliseconds(timeout);   //wait for reset

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
void hc05SetModeComm(struct BluetoothConfig *config, uint16_t timeout){

    if(!config)
        return;

    //reset module (low), pull key low
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

    chThdSleepMilliseconds(timeout);


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


    chThdSleepMilliseconds(timeout);   //wait for reset

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


    chThdSleepMilliseconds(timeout);   //wait for module recovery
    hc05CurrentState = st_ready_communication;
};

#endif //HAL_USE_HC_05_BLUETOOTH || defined(__DOXYGEN__)
 /** @} */
