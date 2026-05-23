/**
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  * 1. Redistributions of source code must retain the above copyright notice,
  * this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  * this list of conditions and the following disclaimer in the documentation
  * and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of its contributors
  * may be used to endorse or promote products derived from this software
  * without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
*/

/*! \file
 *
 *  \author 
 *
 *  \brief SPI communication handling implementation.
 *
 */
 
/* Includes ------------------------------------------------------------------*/

#include "ext_i2c.h"

#define I2C_TIMEOUT      5

extern I2C_HandleTypeDef hi2c2;

I2C_HandleTypeDef *pI2c = &hi2c2;

HAL_StatusTypeDef i2cRead(uint8_t Addr, uint8_t Reg, uint8_t *pBuffer, uint16_t Length)
{
    return HAL_I2C_Mem_Read(pI2c, Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT, pBuffer, Length, I2C_TIMEOUT);
}


HAL_StatusTypeDef i2cWite(uint8_t Addr, uint8_t Reg,  uint8_t *pBuffer, uint16_t Length)
{
    return HAL_I2C_Mem_Write(pI2c, Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT, pBuffer, Length, I2C_TIMEOUT);
}


HAL_StatusTypeDef i2cTx( uint8_t address, const uint8_t *txBuf, uint16_t txLen )
{
  return HAL_I2C_Master_Transmit( pI2c, address, (uint8_t*)txBuf, txLen, I2C_TIMEOUT);
}


HAL_StatusTypeDef i2cRx( uint8_t address, uint8_t *rxBuf, uint16_t rxLen )
{
  return HAL_I2C_Master_Transmit( pI2c, address, rxBuf, rxLen, I2C_TIMEOUT);
}


HAL_StatusTypeDef i2cSequentialTx( uint8_t address, const uint8_t *txBuf, uint16_t txLen, bool last, bool txOnly )
{
    HAL_StatusTypeDef ret;
    uint32_t txFlag;

    if( last && txOnly )
    {
      txFlag = I2C_LAST_FRAME;
    }
    else
    {
      txFlag = (last ? /*I2C_LAST_FRAME_NO_STOP*/ I2C_FIRST_FRAME : I2C_FIRST_AND_NEXT_FRAME);
    }
    
    ret = HAL_I2C_Master_Sequential_Transmit_IT(&hi2c2, (uint16_t)address, (uint8_t*)txBuf, txLen, txFlag );
    if( ret != HAL_OK )
    {
      return ret;
    }
    
    while( HAL_I2C_GetState(&hi2c2) != HAL_I2C_STATE_READY );

    if( HAL_I2C_GetError(&hi2c2) != HAL_I2C_ERROR_NONE )
    {
      return HAL_ERROR;
    }
    return HAL_OK;
}


HAL_StatusTypeDef i2cSequentialRx( uint8_t address, uint8_t *rxBuf, uint16_t rxLen )
{
    HAL_StatusTypeDef ret;
    
    //uint8_t atr[] = {0x0F, 0x78, 0x77, 0x71, 0x02, 0x4A, 0x54, 0x61, 0x78, 0x43, 0x6F, 0x72, 0x65, 0x56, 0x31};

    ret = HAL_I2C_Master_Sequential_Receive_IT(&hi2c2, (uint16_t)address, (uint8_t*)rxBuf, rxLen, I2C_LAST_FRAME);
    if( ret != HAL_OK )
    {
      return ret;
    }
    
    while( HAL_I2C_GetState(&hi2c2) != HAL_I2C_STATE_READY );

    if( HAL_I2C_GetError(&hi2c2) != HAL_I2C_ERROR_NONE )
    {
      return HAL_ERROR;
    }
    
    //if(memcmp(rxBuf, atr, 15) == 0)  print_hex(rxBuf, rxLen);

    return HAL_OK;
}


HAL_StatusTypeDef i2cSequentialTxRx( uint8_t address, const uint8_t *txBuf, uint16_t txLen, uint8_t *rxBuf, uint16_t rxLen )
{
    HAL_StatusTypeDef ret;
  
    ret = HAL_I2C_Master_Sequential_Transmit_IT(pI2c, (uint16_t)address, (uint8_t*)txBuf, txLen, ((rxLen==0) ? I2C_LAST_FRAME : I2C_FIRST_FRAME) );
    if( ret != HAL_OK )
    {
      return ret;
    }
    
    while( HAL_I2C_GetState(pI2c) != HAL_I2C_STATE_READY );
    
    if( HAL_I2C_GetError(pI2c) != HAL_I2C_ERROR_NONE )
    {
      return HAL_ERROR;
    }
  
    ret = HAL_I2C_Master_Sequential_Receive_IT(pI2c, (uint16_t)address, (uint8_t*)rxBuf, rxLen, I2C_LAST_FRAME);
    if( ret != HAL_OK )
    {
      return ret;
    }
    
    while( HAL_I2C_GetState(pI2c) != HAL_I2C_STATE_READY );
    
    if( HAL_I2C_GetError(pI2c) != HAL_I2C_ERROR_NONE )
    {
      return HAL_ERROR;
    }
    
    return HAL_OK;
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
