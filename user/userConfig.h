/**
  ******************************************************************************
  * @file    userConfig.h 
  * @author  Hugo Chan
  * @version V1.0.0
  * @date    24-November-2013
  * @brief   user_defined_headfile user config
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
#ifndef __userConfig_H
#define __userConfig_H


#include <stdio.h>
#include<time.h>
#include <stdlib.h>
/*user define*/
#define uniIndexLength ((uint8_t)3) //Index 元素个数
#define Total_Buffer_Number ((uint8_t)20) //采样模式单次写入flash最大数据数（采样缓冲区大小）
#define upperComputerBufferNum ((uint16_t)2000)
#define lowerComputerBufferNum ((uint16_t)2000)
#endif /* __userConfig_H */
/**
  * @}
  */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
