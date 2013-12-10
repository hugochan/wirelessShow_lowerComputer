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
__IO uint16_t TimeDev = 400;//sys�ж�����Ϊ1000/TimeDev=2.5ms 
__IO uint8_t DemoEnterCondition = 0x00;
__IO uint8_t UserButtonPressed = 0x00;
//__IO uint8_t SerialButtonPressed = 0x00;
LIS302DL_InitTypeDef  LIS302DL_InitStruct;
LIS302DL_FilterConfigTypeDef LIS302DL_FilterStruct;  
uint8_t X_Offset, Y_Offset, Z_Offset  = 0x00;
uint8_t Buffer[6];
_Bool NoWrittenFlag = 0;//δд�루flash����־����ʼ״̬��0��Ϊ�����һ������д��
_Bool DataWriteErrorFlag = 0;//flash�и����ַ�ռ�����д������־��0 forδ����
_Bool WritingSectorFlag = 0;//����д��sector��־λ����ʾĳ��sector���ڱ�ʹ��,0 for δ����д��
_Bool NoFreeSectors = 0;//�޿���sector���־λ��0 for �У�1 for ��
_Bool serialFlag = 1;//����ͨ����Ч��־λ��1 for��Ч��0 for��Ч
__IO uint32_t totalDataNumber = 0;//���μ�¼���ݸ���������
uint8_t Counter = 0;//�����������
uint32_t SendCounter = 0;//�������ݷ��ͼ�����
uint32_t tempCount = 0;//�������ݷ�����ʱ������
uint8_t tempCount2 = 0;//�������ݷ�����ʱ������2
uint8_t Total_Buffer[Total_Buffer_Number] = {0};//�������ݻ�����(��СTotal_Buffer_Number = 80)��
                              //���ڻ���д��flash
//uint32_t addr = (uint8_t*)malloc(sizeof(uint8_t));
uint8_t SendData[lowerComputerBufferNum] = {0};//��Ŷ�ȡ����flash����Ч����,��ͨ�����ڷ��͸���λ������ļ��ٶ�����(��������С2000������λ��������ƥ��)
uint32_t IndexList[uniIndexLength*7];
uint32_t writingDataAddr;//����д�����ݵ�sector���׵�ַ
uint8_t IndexCount = 0;//Index���������ݸ�����������IndexCount=��¼����*uniIndexLength�����ֵΪuniIndexLength*7
unsigned char serialRecv[20];
uint8_t serialRecvNum = 0;//serial recv counter

/* Private function prototypes -----------------------------------------------*/
static void Demo_Exec(void);
void USART1_Config(void);
void NVIC_Config(void);
void SendChar(unsigned char ch);
void SendString(unsigned char *p);
unsigned char GetChar(void);
void IO_Init(void);//��ʼ�����ڼ���Ƿ�ͨ�����ڷ������ݵ�PB11�˿�
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
  
  /* �������� */
  USART1_Config();
  NVIC_Config();
  
  /* Initialize LEDs to be managed by GPIO */
  STM_EVAL_LEDInit(LED4);
  STM_EVAL_LEDInit(LED3);
  STM_EVAL_LEDInit(LED5);
  STM_EVAL_LEDInit(LED6);
  
  //ÿ��ϵͳ��ʼ���󣬶���ȡflash�鿴�洢�ռ�ʣ�����
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
    /***����״̬�����flash���߷�������ѡ��****/
    /******************************************/
    /* Waiting User Button is pressed */
    while (UserButtonPressed == 0x00)
    {
      /********ͨ����������λ��ͨ��*******************/
      /* if Send serial data Button(PB11) is pressed */
      if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == Bit_SET)//��̫����
      {
        while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == Bit_SET);
        
        /*����ͨ��״ָ̬ʾ��*/
        STM_EVAL_LEDOn(LED4);
        STM_EVAL_LEDOn(LED3);
        STM_EVAL_LEDOn(LED5);
        STM_EVAL_LEDOn(LED6);
        
        //��ȡflash��index����
        flash_readIndex(IndexList,uniIndexLength, &IndexCount);
        
        
        //����λ�����ֲ���
        unsigned char* shakeHandSend = "ready? ";//���һ���ַ������ǿո��ַ�
        while (*shakeHandSend != ' ')
        {
          SendChar(*shakeHandSend);
          shakeHandSend++;
        }
        
        Delay(100);//�ȴ�1sȷ�Ͻ�������λ���������ź�"ok!"
        if (serialRecv[0] == 'o'&&serialRecv[1] == 'k'&&serialRecv[2] == '!')
        {
          //����λ�����ֳɹ����ɷ�������
          serialFlag = 1;
        }
        else
        {
          //����λ������ʧ�ܣ��޷���������
          serialFlag = 0;
          
          /*��������ʧ��ָʾ��*/
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
          //�ȷ���IndexList���ݸ���λ������λ��ѡ����Ҫ�ļ�¼�Σ�segment����������λ������λ���ݴ˷�����Ӧ��¼�θ���λ��
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
          
          
          /*��ʼ����segment���ݸ���λ��*/
          
          while(serialRecv[0] == 'o');//�ȴ�ȷ�Ͻ�������λ����ѡ���ź�"0"����"6"
          Delay(2);//�ȴ�20ms��ʼ����data,ʵ���շ�ͬ��
          
          if (serialRecv[0] == '0'||serialRecv[0] == '1'||serialRecv[0] == '2'||serialRecv[0] == '3'||serialRecv[0] == '4'||serialRecv[0] == '5'||serialRecv[0] == '6')
          {
            tempCount = IndexList[(serialRecv[0]-48)*uniIndexLength+2];
            tempCount2 = 0;
            //��ȡflash user data area�ж�Ӧ����
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
              Delay(9);//��ʱ9ms�Ա���λ����ʱ�䴦����������λ����������Сͬ��λ���� 
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
          
          /*ѡ���Ƿ������Ӧsegment��index*/
          //��������λ����ɾ���ź� 'Y' for ɾ����'N' for ��ɾ��
          if(serialRecv[1] == 'Y')
          {
             //uint32_t tempIndexList[uniIndexLength*7];
             flash_init_sector(IndexList[(serialRecv[0]-48)*uniIndexLength]);//������index����Ӧflash user data area����
             flash_init_sector(((uint32_t)0x08010000));//�Ȳ���Index����sector 4��
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
             flash_writeIndex(IndexList,IndexCount-uniIndexLength);//����д����index��¼
             IndexCount -= 3;
          }
          
        }
      }
      
      /********************���flash********************************/
      /* if Button(PB12) is pressed, flash user area will be erased*/
      if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == Bit_SET)
      {
        /*waiting Button(PB12) being released*/
        while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == Bit_SET);
        
        /* flash��ʼ�� */
        flash_init();
        IndexCount = 0;
        /*��ȫ��˸��ȫ��ָʾflash���û����ݱ�����*/
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
      //LED4 3 5���ڶ����Ʊ���ָʾflash�洢��ռ�ø���
      if((IndexCount/uniIndexLength)&(0x01))//���λ
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
      if((IndexCount/uniIndexLength)&(0x04))//���λ
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
    /***�˳�����״̬������״̬���ó�ʼ��*******/
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
    
    
    
    
    /*���μ�¼�໥�����ı�־λ������*/
    Counter = 0;//������ü�������λ
    DataWriteErrorFlag = 0;//flashд������־λ��λ
    NoWrittenFlag = 0;//δд���־λ��λ
    WritingSectorFlag = 0;//����д��sector��־λ��λ
    totalDataNumber = 0;//���μ�¼���ݸ�����������λ
    
    
    UserButtonPressed = 0x00;

    
    /* MEMS configuration */
    LIS302DL_InitStruct.Power_Mode = LIS302DL_LOWPOWERMODE_ACTIVE;
    LIS302DL_InitStruct.Output_DataRate = LIS302DL_DATARATE_400;//���ٶȴ�����������400HZ
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
    /***********��ʼ�������ģʽ��д��flash****/
    /******************************************/
    DemoEnterCondition = 0x01;
    /* Waiting User Button is pressed */
    while (UserButtonPressed == 0x00) //���ò�����϶д�뻺���������ݣ����Ч��
    {
      if((0 == Counter)&&(1 == NoWrittenFlag)&&(0 == DataWriteErrorFlag))
      {
        if (WritingSectorFlag == 0)
        {
          writingDataAddr = getFreeDataStartAddr();
          if (writingDataAddr == 0)
          {
            NoFreeSectors = 1;//�޿���sector����λ
            NoWrittenFlag = 0;
            
            /* ָʾ��ȫ����ָʾ�洢�ռ����� */
            STM_EVAL_LEDOn(LED4);
            STM_EVAL_LEDOn(LED3);
            STM_EVAL_LEDOn(LED5);
            STM_EVAL_LEDOn(LED6);
            break;//�˳�����ģʽ
          }
          else
          {
            //һ������flashд��
            flash_init_sector(writingDataAddr);//����������Ȳ����ÿ���sector��
            DataWriteErrorFlag = flash_writeData(Total_Buffer, Total_Buffer_Number,writingDataAddr);
            NoWrittenFlag = 0;//��ʾ��д��(flash)
            WritingSectorFlag = 1;//����д��sector��־λ��λ����ʾ���øÿ�sector
          }
        }
        else
        {
          //flashд��
          DataWriteErrorFlag = flash_writeData(Total_Buffer, Total_Buffer_Number,writingDataAddr);
          NoWrittenFlag = 0;//��ʾ��д��(flash)
        }
      }
      if(DataWriteErrorFlag != 0)
      {
        flash_init_sector(writingDataAddr);//flashд�����󣬲�����Ӧ�洢�ռ䣬�˳�����״̬
        WritingSectorFlag = 0;//�黹�Ըÿ�洢�ռ��ʹ��Ȩ
        
        /* ָʾ��ȫ����ȫ��ָʾ�������״̬ */
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
      
      /* ָʾ��ȫ��ָʾ�������״̬ */
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
    /***********�˳�����ģʽ�����ƺ���*******/
    /******************************************/
    DemoEnterCondition = 0x00;//��ֹ��ʱ�������ģʽ
    
    /*����˳�����ģʽʱ��������δд��flash���ڴ�һ��д��flash*/
    if(1 == NoWrittenFlag)  flash_writeData(Total_Buffer, Counter,writingDataAddr);
    
    if ((0 == NoFreeSectors)&&(1 == WritingSectorFlag))
    {
      /* ָʾ��ȫ���ȫ����ָʾ���������ƺ�״̬ */
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
      
      /*���ˣ�һ������������¼д��flash��ϣ���Ҫ��flash�н�������������¼������index*/    
      flash_readIndex(IndexList,uniIndexLength, &IndexCount);//��flash�ж�ȡԭIndexList
      IndexList[IndexCount] = writingDataAddr;//д�����µ����ݵ�ַ���׵�ַ
      srand((int)time(0));
      IndexList[IndexCount+1] = rand()%100;//д�����µļ�¼��
      IndexList[IndexCount+2] = totalDataNumber;//д�����µĶμ�¼�ܸ���
      flash_init_sector(((uint32_t)0x08010000));//�Ȳ���Index����sector 4��
      flash_writeIndex(IndexList,IndexCount+3);//��д��
      IndexCount += 3;
    }
         
    /* Disable SPI1 used to drive the MEMS accelerometre */
    SPI_Cmd(LIS302DL_SPI, DISABLE);  
  }
}


/******************************************/
/************�û��Զ��庯��****************/
/******************************************/


/**
  * @brief  convert uint32_t data to 4 uint8_t datas
  * @param  None
  * @retval None
  */
void uint32_to_uint8(unsigned char* data2,uint32_t data)//���ֽ��ȷ���
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


/*********************************NVIC����*******************************/
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

/*********************************���ڳ�ʼ��*******************************/
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
    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF;     //����Ϊ���ã�����ΪAF
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB,&GPIO_InitStructure);
    /* Configure USART Rx as alternate function  */
    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_Init(GPIOB,&GPIO_InitStructure);
    /* USART1 mode config */
    USART_InitStructure.USART_BaudRate = 19200;  //19200HZ������
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);
    USART_ClockStructInit(&USART_ClockInitStruct);//֮ǰû������ȱʡֵ���ǲ��е�
    USART_ClockInit(USART6, &USART_ClockInitStruct);
    /*ʹ���ж�*/
    USART_ITConfig(USART1,USART_IT_RXNE,ENABLE); 
    USART_Cmd(USART1, ENABLE);
}

/************************************����һ���ֽ�******************************/

void SendChar(unsigned char ch)
{  
   USART_SendData(USART1,ch);
   while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET); //�ȴ�������һ���ֽ�
}
/******************************************************************************
//���ڷ���һ���ַ���
//����������������ַ����׵�ַ p
*******************************************************************************/
void SendString(unsigned char *p)
{
    while(*p != '\0')	SendChar(*(p++));
}
/******************************************************************************
//���ڽ���һ���ַ�
//�����������
//��������������ַ�
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
