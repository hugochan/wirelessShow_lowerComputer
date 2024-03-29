/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @author  Hugo Chan
  * @version V1.0.0
  * @date    24-November-2013
  * @brief   Main Interrupt Service Routines.
  *          This file provides all exceptions handler and peripherals interrupt
  *          service routine.
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

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern __IO uint8_t DemoEnterCondition;
extern __IO uint8_t UserButtonPressed;
//extern __IO uint8_t SerialButtonPressed;
extern uint8_t Buffer[];
extern uint8_t X_Offset;
extern uint8_t Y_Offset;
extern uint8_t Z_Offset;
extern _Bool NoWrittenFlag;
extern uint8_t Counter;
extern uint8_t Total_Buffer[];
extern unsigned char serialRecv[10];
extern uint8_t serialRecvNum;
unsigned char tempRecv;
/* Private function prototypes -----------------------------------------------*/
//extern void SendChar(unsigned char ch);
//extern void SendString(unsigned char *p);
//extern unsigned char GetChar(void);
/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
  if (DemoEnterCondition == 0x00)
  {
    TimingDelay_Decrement();
  }
  else
  { 
    LIS302DL_Read(Buffer, LIS302DL_OUT_X_ADDR, 6);
    Buffer[0] = Buffer[0] - X_Offset;
    Buffer[2] = Buffer[2] - Y_Offset;
    if(Counter < Total_Buffer_Number)
    {
      Total_Buffer[Counter] = Buffer[0];//每组xy轴加速度数据依次写入数据缓冲器
      Total_Buffer[Counter + 1] = Buffer[2];
      Counter += 2;
    }
    if(Counter >= Total_Buffer_Number) Counter = 0;
    NoWrittenFlag = 1;//表示未写入（flash）
    
//    SendChar((unsigned char)Buffer[0]);
//    SendChar((unsigned char)Buffer[2]);
  }
  
}

/******************************************************************************/
/*                 STM32Fxxx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32fxxx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @brief  This function handles EXTI0_IRQ Handler.
  * @param  None
  * @retval None
  */
void EXTI0_IRQHandler(void)
{
 
     UserButtonPressed = 0x01;
     /* Clear the EXTI line pending bit */
     EXTI_ClearITPendingBit(USER_BUTTON_EXTI_LINE);

}

/**
  * @brief  This function handles EXTI1_IRQ Handler.
  * @param  None
  * @retval None
  */
/*void EXTI2_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line2) != RESET)
  {   
    SerialButtonPressed = 0x01;  
    EXTI_ClearITPendingBit(EXTI_Line2);
  }

}*/

/**
  * @brief  This function handles EXTI15_10_IRQ Handler.
  * @param  None
  * @retval None
  */


/**
  * @brief  This function handles OTG_HS Handler.
  * @param  None
  * @retval None
  */


/**
* @brief  USBD_HID_GetPos
* @param  None
* @retval Pointer to report
*/


/**
  * @brief  This function handles USART1 interrupt request.
  * @param  None
  * @retval None
  */
void USART1_IRQHandler(void)
{    
    if(USART_GetFlagStatus(USART1,USART_FLAG_RXNE) == SET)
    {
        USART_ClearFlag(USART1,USART_FLAG_RXNE);
        tempRecv = USART_ReceiveData(USART1);
        if(tempRecv != ' ')
        {
          serialRecv[serialRecvNum] = tempRecv;
          serialRecvNum += 1;
        }
        else 
        {
          serialRecvNum = 0;
        }
    }
}

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
