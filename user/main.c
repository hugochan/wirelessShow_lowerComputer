/**
  ******************************************************************************
  * @file    main.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    24-November-2013
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD mLIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

#define TESTRESULT_ADDRESS         0x080FFFFC
#define ALLTEST_PASS               0x00000000
#define ALLTEST_FAIL               0x55555555
    
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

  
uint16_t PrescalerValue = 0;

__IO uint32_t TimingDelay;
__IO uint16_t TimeDev = 400;//sys中断周期为1000/TimeDev=2.5ms 
__IO uint8_t DemoEnterCondition = 0x00;
__IO uint8_t UserButtonPressed = 0x00;
//__IO uint8_t SerialButtonPressed = 0x00;
LIS302DL_InitTypeDef  LIS302DL_InitStruct;
LIS302DL_FilterConfigTypeDef LIS302DL_FilterStruct;  
uint8_t X_Offset, Y_Offset, Z_Offset  = 0x00;
uint8_t Buffer[6];
_Bool NoWrittenFlag = 0;//未写入（flash）标志，初始状态（0）为已完成一次完整写入
_Bool DataWriteErrorFlag = 0;//flash中各块地址空间数据写入出错标志，0 for未出错
_Bool WritingSectorFlag = 0;//正在写入sector标志位，表示某块sector正在被使用,0 for 未正在写入
_Bool NoFreeSectors = 0;//无空闲sector块标志位，0 for 有，1 for 无
_Bool serialFlag = 1;//串口通信有效标志位，1 for有效，0 for无效
__IO uint32_t totalDataNumber = 0;//各段记录数据个数计数器
uint8_t Counter = 0;//采样组计数器
uint32_t SendCounter = 0;//串口数据发送计数器
uint32_t tempCount = 0;//串口数据发送临时计数器
uint8_t tempCount2 = 0;//串口数据发送临时计数器2
uint8_t Total_Buffer[Total_Buffer_Number] = {0};//定义数据缓冲器(大小Total_Buffer_Number = 80)，
                              //用于缓冲写入flash
//uint32_t addr = (uint8_t*)malloc(sizeof(uint8_t));
uint8_t SendData[lowerComputerBufferNum] = {0};//存放读取到的flash中有效数据,将通过串口发送给上位机处理的加速度数据(缓冲区大小2000，与上位机缓冲区匹配)
uint32_t IndexList[uniIndexLength*7];
uint32_t writingDataAddr;//正在写入数据的sector块首地址
uint8_t IndexCount = 0;//Index区域中数据个数计数器，IndexCount=记录段数*uniIndexLength，最大值为uniIndexLength*7
unsigned char serialRecv[20];
uint8_t serialRecvNum = 0;//serial recv counter

/* Private function prototypes -----------------------------------------------*/
static void Demo_Exec(void);
void USART1_Config(void);
void NVIC_Config(void);
void SendChar(unsigned char ch);
void SendString(unsigned char *p);
unsigned char GetChar(void);
void IO_Init(void);//初始化用于检测是否通过串口发送数据的PB11端口
void uint32_to_uint8(unsigned char* data2,uint32_t data);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{
  RCC_ClocksTypeDef RCC_Clocks;
  
  /*Initialize IO*/
  IO_Init();
  
  /* Initialize LEDs and User_Button on STM32F4-Discovery --------------------*/
  STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_EXTI); 
  
  STM_EVAL_LEDInit(LED4);
  STM_EVAL_LEDInit(LED3);
  STM_EVAL_LEDInit(LED5);
  STM_EVAL_LEDInit(LED6);
  
  /* SysTick end of count event each 2.5ms */
  RCC_GetClocksFreq(&RCC_Clocks);
  SysTick_Config(RCC_Clocks.HCLK_Frequency / TimeDev);
  
  if (STM_EVAL_PBGetState(BUTTON_USER) == Bit_SET)
  {
    /* Turn on LEDs available on STM32F4-Discovery ---------------------------*/
    STM_EVAL_LEDOn(LED4);
    STM_EVAL_LEDOn(LED3);
    STM_EVAL_LEDOn(LED5);
    STM_EVAL_LEDOn(LED6); 

    if ((*(__IO uint32_t*) TESTRESULT_ADDRESS) == ALLTEST_PASS)
    {
      TimingDelay = 300;
      /* Waiting User Button is pressed or Test Program condition verified */
      while ((STM_EVAL_PBGetState(BUTTON_USER) == Bit_SET)&&(TimingDelay != 0x00))
      {}
    }
    else
    {
      /* Waiting User Button is Released  or TimeOut*/
      TimingDelay = 300;
      while ((STM_EVAL_PBGetState(BUTTON_USER) == Bit_SET)&&(TimingDelay != 0x00))
      {}
      if (STM_EVAL_PBGetState(BUTTON_USER) == Bit_RESET)
      {
        TimingDelay = 0x00;
      }
    }
    if (TimingDelay == 0x00)
    {
      /* Turn off LEDs available on STM32F4-Discovery ------------------------*/
      STM_EVAL_LEDOff(LED4);
      STM_EVAL_LEDOff(LED3);
      STM_EVAL_LEDOff(LED5);
      STM_EVAL_LEDOff(LED6); 
      
      /* Waiting User Button is released */
      while (STM_EVAL_PBGetState(BUTTON_USER) == Bit_SET)
      {}
      
      /* Unlocks the FLASH control register access */
      FLASH_Unlock();
      
      /* Move discovery kit to detect negative and positive acceleration values 
      on X, Y and Z axis */
      Accelerometer_MEMS_Test();
      
      /* Write PASS code at last word in the flash memory */
      FLASH_ProgramWord(TESTRESULT_ADDRESS, ALLTEST_PASS);
      
      while(1)
      {
        /* Toggle Green LED: signaling the End of the Test program */
        STM_EVAL_LEDToggle(LED4);
        Delay(10);
      }
    }
    else
    {
      Demo_Exec();
    }
  }
  else
  {    
    Demo_Exec();
  }
}

/**
  * @brief  Execute the demo application.
  * @param  None
  * @retval None
  */
static void Demo_Exec(void)
{
  RCC_ClocksTypeDef RCC_Clocks;
  
  /* 串口配置 */
  USART1_Config();
  NVIC_Config();
  
  /* Initialize LEDs to be managed by GPIO */
  STM_EVAL_LEDInit(LED4);
  STM_EVAL_LEDInit(LED3);
  STM_EVAL_LEDInit(LED5);
  STM_EVAL_LEDInit(LED6);
  
  //每次系统初始化后，都读取flash查看存储空间剩余情况
  flash_readIndex(IndexList,uniIndexLength, &IndexCount);
  
  while(1)
  {
    DemoEnterCondition = 0x00;
    
    /* Reset UserButton_Pressed variable */
    UserButtonPressed = 0x00;
//    SerialButtonPressed = 0x00;
    
    /* SysTick end of count event each 2.5ms */
    RCC_GetClocksFreq(&RCC_Clocks);
    SysTick_Config(RCC_Clocks.HCLK_Frequency / TimeDev);
    
    

    /******************************************/
    /***空闲状态，清空flash或者发送数据选项****/
    /******************************************/
    /* Waiting User Button is pressed */
    while (UserButtonPressed == 0x00)
    {
      /********通过串口与上位机通信*******************/
      /* if Send serial data Button(PB11) is pressed */
      if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == Bit_SET)//不太灵敏
      {
        while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == Bit_SET);
        
        /*串口通信状态指示灯*/
        STM_EVAL_LEDOn(LED4);
        STM_EVAL_LEDOn(LED3);
        STM_EVAL_LEDOn(LED5);
        STM_EVAL_LEDOn(LED6);
        
        //读取flash中index区域
        flash_readIndex(IndexList,uniIndexLength, &IndexCount);
        
        
        //与上位机握手操作
        unsigned char* shakeHandSend = "ready? ";//最后一个字符必须是空格字符
        while (*shakeHandSend != ' ')
        {
          SendChar(*shakeHandSend);
          shakeHandSend++;
        }
        
        Delay(100);//等待1s确认接收自上位机的握手信号"ok!"
        if (serialRecv[0] == 'o'&&serialRecv[1] == 'k'&&serialRecv[2] == '!')
        {
          //与上位机握手成功，可发送数据
          serialFlag = 1;
        }
        else
        {
          //与上位机握手失败，无法发送数据
          serialFlag = 0;
          
          /*串口握手失败指示灯*/
          STM_EVAL_LEDOff(LED4);
          Delay(10);
          STM_EVAL_LEDOff(LED3);
          Delay(10);
          STM_EVAL_LEDOff(LED5);
          Delay(10);
          STM_EVAL_LEDOff(LED6);
          Delay(100);
        }
        
        
        if (1 == serialFlag)
        {
          //先发送IndexList数据给上位机，上位机选择需要的记录段（segment）反馈给下位机，下位机据此发送相应记录段给上位机
          uint8_t i;
          unsigned char sendData[4];
          
          SendString("index");
          SendCounter = 0;
          while(SendCounter < IndexCount)
          {
            uint32_to_uint8(sendData,IndexList[SendCounter]);
            i = 0;
            while(i < 4)
            {
              SendChar((unsigned char)sendData[i]);
              i+= 1 ;
            }
            SendCounter += 1;
          }
          
          
          /*开始发送segment数据给上位机*/
          
          while(serialRecv[0] == 'o');//等待确认接收自上位机的选择信号"0"——"6"
          Delay(2);//等待20ms后开始发送data,实现收发同步
          
          if (serialRecv[0] == '0'||serialRecv[0] == '1'||serialRecv[0] == '2'||serialRecv[0] == '3'||serialRecv[0] == '4'||serialRecv[0] == '5'||serialRecv[0] == '6')
          {
            tempCount = IndexList[(serialRecv[0]-48)*uniIndexLength+2];
            tempCount2 = 0;
            //读取flash user data area中对应数据
            while (tempCount > lowerComputerBufferNum)
            {
              
              flash_readData(SendData,lowerComputerBufferNum, tempCount2*lowerComputerBufferNum, IndexList[(serialRecv[0]-48)*uniIndexLength]);
              SendCounter = 0;
              while(SendCounter < lowerComputerBufferNum)
              {
                SendChar((unsigned char)SendData[SendCounter]);
                SendCounter += 1 ;
              }
              tempCount -= lowerComputerBufferNum;
              tempCount2 += 1;
              Delay(9);//延时9ms以便上位机有时间处理缓冲区（上位机缓冲区大小同下位机） 
            }
            
            if (tempCount != 0)
            {
               flash_readData(SendData,tempCount, tempCount2*lowerComputerBufferNum, IndexList[(serialRecv[0]-48)*uniIndexLength]);
               SendCounter = 0;
               while(SendCounter < tempCount)
               {
                  SendChar((unsigned char)SendData[SendCounter]);
                  SendCounter += 1 ;
               } 
            }
          }
          
          /*选择是否擦除对应segment和index*/
          //接收自上位机的删除信号 'Y' for 删除，'N' for 不删除
          if(serialRecv[1] == 'Y')
          {
             //uint32_t tempIndexList[uniIndexLength*7];
             flash_init_sector(IndexList[(serialRecv[0]-48)*uniIndexLength]);//擦除该index所对应flash user data area区域
             flash_init_sector(((uint32_t)0x08010000));//先擦除Index区域（sector 4）
             i = 0;
             uint8_t k = 0;
             while(i < IndexCount)
             {
               if (i != (serialRecv[0]-48)*uniIndexLength)
               {
                  IndexList[k] = IndexList[i];
                  IndexList[k+1] = IndexList[i+1];
                  IndexList[k+2] = IndexList[i+2];
                  k += uniIndexLength;
               }
               i += uniIndexLength;
             }
             flash_writeIndex(IndexList,IndexCount-uniIndexLength);//后重写该条index记录
             IndexCount -= 3;
          }
          
        }
      }
      
      /********************清空flash********************************/
      /* if Button(PB12) is pressed, flash user area will be erased*/
      if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == Bit_SET)
      {
        /*waiting Button(PB12) being released*/
        while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == Bit_SET);
        
        /* flash初始化 */
        flash_init();
        IndexCount = 0;
        /*灯全闪烁后全灭，指示flash中用户数据被擦除*/
         STM_EVAL_LEDToggle(LED4);
         STM_EVAL_LEDToggle(LED3);
         STM_EVAL_LEDToggle(LED5);
         STM_EVAL_LEDToggle(LED6);
         Delay(10);
         STM_EVAL_LEDOff(LED4);
         STM_EVAL_LEDOff(LED3);
         STM_EVAL_LEDOff(LED5);
         STM_EVAL_LEDOff(LED6);
         Delay(100);
      }
      
      /* Toggle LED4 */
      STM_EVAL_LEDToggle(LED6);
      Delay(50);
      //LED4 3 5用于二进制编码指示flash存储区占用个数
      if((IndexCount/uniIndexLength)&(0x01))//最低位
      { 
        STM_EVAL_LEDOn(LED5);
      }
      else
      {
         STM_EVAL_LEDOff(LED5);
      }
      if((IndexCount/uniIndexLength)&(0x02))
      { 
        STM_EVAL_LEDOn(LED3);
      }
      else
      {
         STM_EVAL_LEDOff(LED3);
      }
      if((IndexCount/uniIndexLength)&(0x04))//最高位
      { 
        STM_EVAL_LEDOn(LED4);
      }
      else
      {
         STM_EVAL_LEDOff(LED4);
      }
    }
    
    /* Waiting User Button is Released */
    while (STM_EVAL_PBGetState(BUTTON_USER) == Bit_SET)
    {}
    
    
    
    /******************************************/
    /***退出空闲状态，采样状态配置初始化*******/
    /******************************************/
    STM_EVAL_LEDOn(LED4);
    STM_EVAL_LEDOff(LED4);
    Delay(10);
    STM_EVAL_LEDOn(LED3);
    STM_EVAL_LEDOff(LED3);
    Delay(10);
    STM_EVAL_LEDOn(LED5);
    STM_EVAL_LEDOff(LED5);
    Delay(10);
    STM_EVAL_LEDOn(LED6);
    STM_EVAL_LEDOff(LED6);
    Delay(10);
    
    
    
    
    /*各段记录相互独立的标志位均清零*/
    Counter = 0;//分组采用计数器复位
    DataWriteErrorFlag = 0;//flash写入错误标志位复位
    NoWrittenFlag = 0;//未写入标志位复位
    WritingSectorFlag = 0;//正在写入sector标志位复位
    totalDataNumber = 0;//各段记录数据个数计数器复位
    
    
    UserButtonPressed = 0x00;

    
    /* MEMS configuration */
    LIS302DL_InitStruct.Power_Mode = LIS302DL_LOWPOWERMODE_ACTIVE;
    LIS302DL_InitStruct.Output_DataRate = LIS302DL_DATARATE_400;//加速度传感器采用率400HZ
    LIS302DL_InitStruct.Axes_Enable = LIS302DL_XYZ_ENABLE;
    LIS302DL_InitStruct.Full_Scale = LIS302DL_FULLSCALE_2_3;//18 mg/digit
    LIS302DL_InitStruct.Self_Test = LIS302DL_SELFTEST_NORMAL;
    LIS302DL_Init(&LIS302DL_InitStruct);
    
    /* Required delay for the MEMS Accelerometre: Turn-on time = 3/Output data Rate 
    = 3/400 = 7.5ms */
    Delay(1);
    
    /* MEMS High Pass Filter configuration */
    LIS302DL_FilterStruct.HighPassFilter_Data_Selection = LIS302DL_FILTEREDDATASELECTION_OUTPUTREGISTER;
    LIS302DL_FilterStruct.HighPassFilter_CutOff_Frequency = LIS302DL_HIGHPASSFILTER_LEVEL_1;
    LIS302DL_FilterStruct.HighPassFilter_Interrupt = LIS302DL_HIGHPASSFILTERINTERRUPT_1_2;
    LIS302DL_FilterConfig(&LIS302DL_FilterStruct);
    
    LIS302DL_Read(Buffer, LIS302DL_OUT_X_ADDR, 6);
    X_Offset = Buffer[0];
    Y_Offset = Buffer[2];
    Z_Offset = Buffer[4];
  
    
    
    /******************************************/
    /***********开始进入采样模式，写入flash****/
    /******************************************/
    DemoEnterCondition = 0x01;
    /* Waiting User Button is pressed */
    while (UserButtonPressed == 0x00) //利用采样间隙写入缓冲器内数据，提高效率
    {
      if((0 == Counter)&&(1 == NoWrittenFlag)&&(0 == DataWriteErrorFlag))
      {
        if (WritingSectorFlag == 0)
        {
          writingDataAddr = getFreeDataStartAddr();
          if (writingDataAddr == 0)
          {
            NoFreeSectors = 1;//无空闲sector块置位
            NoWrittenFlag = 0;
            
            /* 指示灯全亮，指示存储空间已满 */
            STM_EVAL_LEDOn(LED4);
            STM_EVAL_LEDOn(LED3);
            STM_EVAL_LEDOn(LED5);
            STM_EVAL_LEDOn(LED6);
            break;//退出采样模式
          }
          else
          {
            //一次完整flash写入
            flash_init_sector(writingDataAddr);//保险起见，先擦除该空闲sector块
            DataWriteErrorFlag = flash_writeData(Total_Buffer, Total_Buffer_Number,writingDataAddr);
            NoWrittenFlag = 0;//表示已写入(flash)
            WritingSectorFlag = 1;//正在写入sector标志位置位，表示启用该块sector
          }
        }
        else
        {
          //flash写入
          DataWriteErrorFlag = flash_writeData(Total_Buffer, Total_Buffer_Number,writingDataAddr);
          NoWrittenFlag = 0;//表示已写入(flash)
        }
      }
      if(DataWriteErrorFlag != 0)
      {
        flash_init_sector(writingDataAddr);//flash写入错误后，擦除相应存储空间，退出采样状态
        WritingSectorFlag = 0;//归还对该块存储空间的使用权
        
        /* 指示灯全亮后全灭，指示进入采样状态 */
        STM_EVAL_LEDOn(LED4);
        STM_EVAL_LEDOn(LED3);
        STM_EVAL_LEDOn(LED5);
        STM_EVAL_LEDOn(LED6);
        Delay(100);
        STM_EVAL_LEDOff(LED4);
        STM_EVAL_LEDOff(LED3);
        STM_EVAL_LEDOff(LED5);
        STM_EVAL_LEDOff(LED6);
        Delay(100);
        break;
      }
      
      /* 指示灯全灭，指示进入采样状态 */
      STM_EVAL_LEDOff(LED4);
      STM_EVAL_LEDOff(LED4);
      STM_EVAL_LEDOff(LED3);
      STM_EVAL_LEDOff(LED5);
      STM_EVAL_LEDOff(LED6);
    }
    
    /* Waiting User Button is Released */
    while (STM_EVAL_PBGetState(BUTTON_USER) == Bit_SET)
    {}
    
   
    
    /******************************************/
    /***********退出采样模式，做善后处理*******/
    /******************************************/
    DemoEnterCondition = 0x00;//防止此时进入采样模式
    
    /*如果退出采样模式时尚有数据未写入flash，在此一并写入flash*/
    if(1 == NoWrittenFlag)  flash_writeData(Total_Buffer, Counter,writingDataAddr);
    
    if ((0 == NoFreeSectors)&&(1 == WritingSectorFlag))
    {
      /* 指示灯全灭后全亮，指示进入数据善后状态 */
      STM_EVAL_LEDOff(LED4);
      STM_EVAL_LEDOff(LED3);
      STM_EVAL_LEDOff(LED5);
      STM_EVAL_LEDOff(LED6);
      Delay(10);
      STM_EVAL_LEDOn(LED4);
      STM_EVAL_LEDOn(LED3);
      STM_EVAL_LEDOn(LED5);
      STM_EVAL_LEDOn(LED6);
      Delay(10);
      
      /*至此，一条“完整”记录写入flash完毕，需要在flash中建立关于这条记录的索引index*/    
      flash_readIndex(IndexList,uniIndexLength, &IndexCount);//从flash中读取原IndexList
      IndexList[IndexCount] = writingDataAddr;//写入最新的数据地址块首地址
      srand((int)time(0));
      IndexList[IndexCount+1] = rand()%100;//写入最新的记录名
      IndexList[IndexCount+2] = totalDataNumber;//写入最新的段记录总个数
      flash_init_sector(((uint32_t)0x08010000));//先擦除Index区域（sector 4）
      flash_writeIndex(IndexList,IndexCount+3);//后写入
      IndexCount += 3;
    }
         
    /* Disable SPI1 used to drive the MEMS accelerometre */
    SPI_Cmd(LIS302DL_SPI, DISABLE);  
  }
}


/******************************************/
/************用户自定义函数****************/
/******************************************/


/**
  * @brief  convert uint32_t data to 4 uint8_t datas
  * @param  None
  * @retval None
  */
void uint32_to_uint8(unsigned char* data2,uint32_t data)//高字节先发送
{
  data2[0] = (data&0xff000000)>>24;
  data2[1] = (data&0x00ff0000)>>16;
  data2[2] = (data&0x0000ff00)>>8;
  data2[3] = data&0x000000ff;
}

/**
  * @brief  Initialize IO for deciding whether to Send Data through USART
  * @param  None
  * @retval None
  */
void IO_Init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  
  /* GPIOD Periph clock enable */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  
  /* Configure PB12 pin as input */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  /* Configure PB11 pin as input */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GPIOB, &GPIO_InitStructure);


  
  /* Enable SYSCFG clock */
//  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
  /* Connect EXTI Line1 to PB11 pin */
//  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource11);
  
  /* Configure EXTI Line1 */
/*  EXTI_InitTypeDef   EXTI_InitStructure;
  EXTI_InitStructure.EXTI_Line = EXTI_Line2;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
*/  
  /* Enable and set EXTI Line1 Interrupt to the low priority */
/*  NVIC_InitTypeDef NVIC_InitStructure;
    
  NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0E;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0E;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
*/
}


/*********************************NVIC配置*******************************/
void NVIC_Config(void)
{
    /* config usart1 Interrupt*/  
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x03;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;
    NVIC_InitStructure.NVIC_IRQChannel  = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        
    NVIC_Init(&NVIC_InitStructure);
    
}

/*********************************串口初始化*******************************/
void USART1_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    USART_ClockInitTypeDef USART_ClockInitStruct;

    /* config USART1 clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    /* Enable GPIO clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
   
    GPIO_PinAFConfig(GPIOB,GPIO_PinSource6,GPIO_AF_USART1);  //PB6--USART1_TX
    GPIO_PinAFConfig(GPIOB,GPIO_PinSource7,GPIO_AF_USART1);  //PB7--USART1_RX   
    /* Configure USART Tx as alternate function  */
    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_6;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF;     //设置为复用，必须为AF
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB,&GPIO_InitStructure);
    /* Configure USART Rx as alternate function  */
    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_Init(GPIOB,&GPIO_InitStructure);
    /* USART1 mode config */
    USART_InitStructure.USART_BaudRate = 19200;  //19200HZ波特率
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);
    USART_ClockStructInit(&USART_ClockInitStruct);//之前没有填入缺省值，是不行的
    USART_ClockInit(USART6, &USART_ClockInitStruct);
    /*使能中断*/
    USART_ITConfig(USART1,USART_IT_RXNE,ENABLE); 
    USART_Cmd(USART1, ENABLE);
}

/************************************发送一个字节******************************/

void SendChar(unsigned char ch)
{  
   USART_SendData(USART1,ch);
   while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET); //等待发送完一个字节
}
/******************************************************************************
//串口发送一个字符串
//输入参数：待发送字符串首地址 p
*******************************************************************************/
void SendString(unsigned char *p)
{
    while(*p != '\0')	SendChar(*(p++));
}
/******************************************************************************
//串口接收一个字符
//输入参数：无
//输出参数：接收字符
*******************************************************************************/
unsigned char GetChar(void)
{
    unsigned char ch = ' ';
    if(USART_GetFlagStatus(USART1,USART_FLAG_RXNE)==SET)
    {
      USART_ClearFlag(USART1,USART_FLAG_RXNE);
      ch = USART_ReceiveData(USART1);
    }
    return ch;
}


/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in 10 ms.
  * @retval None
  */
void Delay(__IO uint32_t nTime)
{
  TimingDelay = nTime;

  while(TimingDelay != 0);
}

/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  { 
    TimingDelay--;
  }
}

/**
  * @brief  This function handles the test program fail.
  * @param  None
  * @retval None
  */
void Fail_Handler(void)
{
  /* Erase last sector */ 
  FLASH_EraseSector(FLASH_Sector_11, VoltageRange_3);
  /* Write FAIL code at last word in the flash memory */
  FLASH_ProgramWord(TESTRESULT_ADDRESS, ALLTEST_FAIL);
  
  while(1)
  {
    /* Toggle Red LED */
    STM_EVAL_LEDToggle(LED5);
    Delay(5);
  }
}

/**
  * @brief  MEMS accelerometre management of the timeout situation.
  * @param  None.
  * @retval None.
  */
uint32_t LIS302DL_TIMEOUT_UserCallback(void)
{
  /* MEMS Accelerometer Timeout error occured during Test program execution */
  if (DemoEnterCondition == 0x00)
  {
    /* Timeout error occured for SPI TXE/RXNE flags waiting loops.*/
    Fail_Handler();    
  }
  /* MEMS Accelerometer Timeout error occured during Demo execution */
  else
  {
    while (1)
    {   
    }
  }
  return 0;  
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
