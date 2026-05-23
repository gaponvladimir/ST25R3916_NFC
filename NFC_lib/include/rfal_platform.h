
/**
  ******************************************************************************
  * @file           : rfal_platform.h
  * @brief          : Platform header file. Defining platform independent functionality.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include "main.h"
#include "i2c.h"

#include "timer.h"
#include "nfc_conf.h"

// my add
#define RFAL_USE_I2C
//#define RFAL_USE_SPI

typedef struct  {
	SPI_HandleTypeDef *nfc_spi;
	GPIO_TypeDef *nfc_cs_port;
	uint16_t nfc_cs_pin;
	GPIO_TypeDef *nfc_irq_port;
	uint16_t nfc_irq_pin;
}nfc_platform;

/* Exported types ------------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
/** @defgroup PTD_Platform_Exported_Macro
 *  @{
 */

//extern nfc_platform platform_spi_init;

// Move interrupt
#define platformProtectST25RComm()
#define platformUnprotectST25RComm()

#define platformSPI_INIT(spi,cs_port,cs_pin)	  do{	platform_spi_init.nfc_spi =spi; 		\
														platform_spi_init.nfc_cs_port = port; 	\
														platform_spi_init.nfc_cs_pin = pin; 	\
														}while(0)

#define platformProtectST25RIrqStatus()           platformProtectST25RComm()                /*!< Protect unique access to IRQ status var - IRQ disable on single thread environment (MCU) ; Mutex lock on a multi thread environment */
#define platformUnprotectST25RIrqStatus()         platformUnprotectST25RComm()              /*!< Unprotect the IRQ status var - IRQ enable on a single thread environment (MCU) ; Mutex unlock on a multi thread environment         */

#define platformProtectWorker()                                                                     /* Protect RFAL Worker/Task/Process from concurrent execution on multi thread platforms   */
#define platformUnprotectWorker()                                                                   /* Unprotect RFAL Worker/Task/Process from concurrent execution on multi thread platforms */

#define platformIrqST25RSetCallback( cb )
#define platformIrqST25RPinInitialize()

#define platformLedsInitialize()                                                                    /*!< Initializes the pins used as LEDs to outputs*/

#define platformLedOff( port, pin )                   platformGpioClear(port, pin)                  /*!< Turns the given LED Off                     */
#define platformLedOn( port, pin )                    platformGpioSet(port, pin)                    /*!< Turns the given LED On                      */
#define platformLedToogle( port, pin )                platformGpioToogle(port, pin)                 /*!< Toogle the given LED                        */

#define platformGpioSet( port, pin )                  HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET)    /*!< Turns the given GPIO High                   */
#define platformGpioClear( port, pin )                HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET)  /*!< Turns the given GPIO Low                    */
#define platformGpioToogle( port, pin )               HAL_GPIO_TogglePin(port, pin)                 /*!< Toogles the given GPIO                      */
#define platformGpioIsHigh( port, pin )               (HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_SET) /*!< Checks if the given LED is High             */
#define platformGpioIsLow( port, pin )                (!platformGpioIsHigh(port, pin))              /*!< Checks if the given LED is Low              */

#define platformTimerCreate( t )                      timerCalculateTimer(t)                        /*!< Create a timer with the given time (ms)     */
#define platformTimerIsExpired( timer )               timerIsExpired(timer)                         /*!< Checks if the given timer is expired        */
#define platformTimerDestroy( timer )                                                               /*!< Stop and release the given timer            */
#define platformDelay( t )                            HAL_Delay( t )                                /*!< Performs a delay for the given time (ms)    */

#define platformGetSysTick()                          HAL_GetTick()                                 /*!< Get System Tick ( 1 tick = 1 ms)            */

//#define platformErrorHandle()                         _Error_Handler(__FILE__,__LINE__)             /*!< Global error handler or trap                */
#define platformErrorHandle()                                      									/*!< Global error handler or trap                */
#define platformSpiSelect()                           platformGpioClear(platform_spi_init.nfc_cs_port, platform_spi_init.nfc_cs_pin)/*!< SPI SS\CS: Chip|Slave Select                */
#define platformSpiDeselect()                         platformGpioSet(platform_spi_init.nfc_cs_port, platform_spi_init.nfc_cs_pin)  /*!< SPI SS\CS: Chip|Slave Deselect              */
#define platformSpiTxRx( txBuf, rxBuf, len )          platform_SPI_SendRecv(txBuf, rxBuf, len)          /*!< SPI transceive                              */

//#define platformLog(...)                              logUsart(__VA_ARGS__)                         /*!< Log  method                                 */

/* Exported functions ------------------------------------------------------- */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_H */

