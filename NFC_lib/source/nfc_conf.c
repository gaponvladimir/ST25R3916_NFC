/**
  ******************************************************************************
  * @file           : nfc_conf.c
  * @brief          : This file contains definitions for the NFC6/8 components bus interfaces
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

#include "nfc_conf.h"
#include "spi.h"
#include "i2c.h"
#include "main.h"
#include "rfal_platform.h"

/* Private variables ---------------------------------------------------------*/
nfc_platform platform_spi_init = {
    .nfc_spi = ST25R_SPI,
    .nfc_cs_port = ST25R_SS_GPIO_Port,
    .nfc_cs_pin = ST25R_SS_Pin,
	.nfc_irq_port = ST25R_INT_PORT,
	.nfc_irq_pin = ST25R_INT_PIN,
};

extern nfc_platform platform_spi_init;

/* Private user code ---------------------------------------------------------*/
/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

int32_t platform_SPI_SendRecv(const uint8_t * const pTxData, uint8_t * const pRxData, uint16_t Length)
{
  HAL_StatusTypeDef status = HAL_ERROR;

  if((pTxData != NULL) && (pRxData != NULL))
  {
    status = HAL_SPI_TransmitReceive(platform_spi_init.nfc_spi, (uint8_t *)pTxData, (uint8_t *)pRxData, Length, 2000);
  }
  else if ((pTxData != NULL) && (pRxData == NULL))
  {
    status = HAL_SPI_Transmit(platform_spi_init.nfc_spi, (uint8_t *)pTxData, Length, 2000);
  }
  else if ((pTxData == NULL) && (pRxData != NULL))
  {
    status = HAL_SPI_Receive(platform_spi_init.nfc_spi, (uint8_t *)pRxData, Length, 2000);
  }
  else
  {

  }

  /* Check the communication status */
  if (status != HAL_OK)
  {
    /* Execute user timeout callback */

  }

  return 0;
}
