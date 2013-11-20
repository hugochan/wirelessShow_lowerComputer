/**
  ******************************************************************************
  * @file    flash_usr.h 
  * @author  Hugo Chan
  * @version V1.0.0
  * @date    26-May-2013
  * @brief   user_defined_headfile for FLASHModle
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************  
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __flash_usr_H
#define __flash_usr_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_flash.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern __IO uint32_t TotalNumber;
/* Exported functions ------------------------------------------------------- */
extern uint32_t GetSector(uint32_t Address);
extern void flash_init(void);
extern _Bool flash_write(uint8_t Data[],uint32_t DataNumber);
extern void flash_read(uint8_t ReadData[]);

#endif /* __flash_usr_H */
/**
  * @}
  */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
