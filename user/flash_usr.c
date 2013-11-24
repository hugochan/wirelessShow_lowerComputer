/**
  ******************************************************************************
  * @file    flash_usr.c 
  * @author  Hugo Chan
  * @version V1.0.0
  * @date    24-November-2013
  * @brief   user_defined_file for FLASHModle
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

#include "stm32f4xx_flash.h"
#include "userConfig.h"

/* Private define ------------------------------------------------------------*/
#define FLASH_USER_START_ADDR   ADDR_FLASH_SECTOR_4   /* Start @ of user Flash area */
#define FLASH_USER_END_ADDR     ADDR_FLASH_SECTOR_11   /* End @ of user Flash area */
#define FLASH_USER_START_Index_ADDR   ADDR_FLASH_SECTOR_4   /* Start @ of user Flash index area */
#define FLASH_USER_START_Data_ADDR   ADDR_FLASH_SECTOR_5   /* Start @ of user Flash data area */

/* Base address of the Flash sectors */
#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base @ of Sector 7, 128 Kbytes */
#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) /* Base @ of Sector 8, 128 Kbytes */
#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) /* Base @ of Sector 9, 128 Kbytes */
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) /* Base @ of Sector 10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbytes */

#define maxTotalDataNumber ((__IO uint32_t)131072) //128Kbytes/1bytes=128*1024
#define dataSectorNum  ((uint8_t)7) //user flash data area sector number 
/* Private variables ---------------------------------------------------------*/
uint32_t StartSector = 0, EndSector = 0;
extern __IO uint32_t totalDataNumber;//各段记录数据个数计数器 
__IO uint32_t Address = FLASH_USER_START_ADDR;//flash内部地址（此处作为指针用）
//__IO uint32_t maxTotalDataNumber = 131072;//128Kbytes/1bytes=128*1024

/* Private functions ---------------------------------------------------------*/
uint32_t GetSector(uint32_t Address);
_Bool flash_init(void);
_Bool flash_init_sector(uint32_t sectorAddr);
uint32_t getFreeDataStartAddr(void);
char flash_writeIndex(uint32_t IndexList[], uint8_t IndexLength);
char flash_writeData(uint8_t Data[],uint32_t DataNumber, uint32_t dataStartAddr);
char flash_readIndex(uint32_t readIndexList[], uint8_t readIndexLength, uint8_t* readIndexCount);
char flash_readData(uint8_t ReadData[], uint32_t totalDataNumber, uint32_t startDataNumber, uint32_t dataStartAddr);


/**
  * @brief  Initialize the flash control
  * @param  None
  * @retval None
  */
_Bool flash_init(void)
{ 
  uint32_t i = 0; 
  Address = FLASH_USER_START_ADDR;//每次初始化flash时初始化地址
  
  /* Unlock the Flash to enable the flash control register access *************/ 
  FLASH_Unlock();
  
  /* Clear pending flags (if any) */  
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 
  
  /* Get the number of the start and end sectors */
  StartSector = GetSector(FLASH_USER_START_ADDR);
  EndSector = GetSector(FLASH_USER_END_ADDR);
  
  /* Erase the user Flash area
    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/
   for (i = StartSector; i < EndSector; i += 8)
  {
    /* Device voltage range supposed to be [2.7V to 3.6V], the operation will
       be done by word */ 
    if (FLASH_EraseSector(i, VoltageRange_3) != FLASH_COMPLETE)
    { 
      /* Error occurred while sector erase. 
         User can add here some code to deal with this error  */
     return 1;
    }
  }
  
  /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
  FLASH_Lock();
  return 0;
}

/**
  * @brief  Initialize the specific flash sector control
  * @param  None
  * @retval None
  */
_Bool flash_init_sector(uint32_t sectorAddr)
{ 
  /* Unlock the Flash to enable the flash control register access *************/ 
  FLASH_Unlock();
  
  /* Clear pending flags (if any) */  
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 
  
  /* Erase the user Flash area */
 
  /* Device voltage range supposed to be [2.7V to 3.6V], the operation will
       be done by word */ 
  if (FLASH_EraseSector(GetSector(sectorAddr), VoltageRange_3) != FLASH_COMPLETE)
  { 
    /* Error occurred while sector erase. 
     User can add here some code to deal with this error  */
    return 1;
  }
  
  /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
  FLASH_Lock();
  return 0;
}

/**
  * @brief  get free flash user data area address
  * @param  None
  * @retval None
  */
uint32_t getFreeDataStartAddr(void)
{
  uint32_t readIndexList[uniIndexLength*dataSectorNum];//最大存满所有7个sector的index
  uint8_t* readIndexCount = (uint8_t*)malloc(sizeof(uint8_t));
  uint32_t freeDataStartAddr = 0;//存放返回空闲地址块
  flash_readIndex(readIndexList,uniIndexLength, readIndexCount);//???????查看程序 断点
  if (*readIndexCount != 0)
  {
    uint8_t iCount = 0;
    uint32_t tempAddr[dataSectorNum];//最多存放7个地址块空间
    while(iCount < *readIndexCount)
    {
      tempAddr[iCount] = *(readIndexList+iCount*uniIndexLength);//取出readIndexList中地址块存入临时数组
      iCount++;
    }
    uint8_t iSector = 0;
    _Bool getFlag;
    while(iSector < dataSectorNum)
    {
      getFlag = 1;
      iCount = 0;
      while(iCount < (uint8_t)((*readIndexCount)/uniIndexLength))
      {
        //如果某个sector首地址在tempAddr数组中，则该首地址不合格，查询下一块sector
        if(tempAddr[iCount] == (ADDR_FLASH_SECTOR_5+(uint32_t)iSector*((uint32_t)0x00020000)))
        {
          getFlag = 0;
          break;//跳出循环
        }
        iCount++;
      }
      if (getFlag == 1)//如果某个sector首地址不在tempAddr数组中，则该首地址为有效freeDataStartAddr
      {
        freeDataStartAddr = (ADDR_FLASH_SECTOR_5+(uint32_t)iSector*((uint32_t)0x00020000));
        break;//跳出循环
      }
      iSector++;
    }
  }
  else
  {
    freeDataStartAddr = FLASH_USER_START_Data_ADDR;//若index为空，返回flash user data area首地址
  }
  return freeDataStartAddr;
}
/**
  * @brief  Write Data[] to flash
  * @param  Data[]: Data to be written
  * @param  DataNumber: Number of Data[]
  * @retval _Bool data(0 for success, 1 for failure)
  */
char flash_writeIndex(uint32_t IndexList[], uint8_t IndexLength)//每次先读取index内容然后擦除后重新写入新内容
{//uint32_t dataStartAddr,uint32_t name, uint32_t totalDataNumber
    uint8_t i = 0;
    uint32_t indexStartAddr = FLASH_USER_START_Index_ADDR;
  /* Unlock the Flash to enable the flash control register access *************/ 
    FLASH_Unlock();
   /* Program the user Flash area word by word
     (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/   
    while(i < IndexLength)  
    {
      if (FLASH_ProgramWord(indexStartAddr, IndexList[i]) == FLASH_COMPLETE)
      {
          indexStartAddr += 4;
          i++;
      }
      else
      { 
        /* Error occurred while writing data in Flash memory. 
           User can add here some code to deal with this error */
        return 1;
      }
    }
    
    
    /* Lock the Flash to disable the flash control register access (recommended
       to protect the FLASH memory against possible unwanted operation) *********/
    FLASH_Lock();
    return 0;
    
    
}
/**
  * @brief  Write Data[] to flash
  * @param  Data[]: Data to be written
  * @param  DataNumber: Number of Data[]
  * @retval char data(0 for success, 1 for write error, 2 for data overflow)
  */
char flash_writeData(uint8_t Data[],uint32_t DataNumber, uint32_t dataStartAddr)
{
    uint32_t i = 0;
      
    /* Unlock the Flash to enable the flash control register access *************/ 
    FLASH_Unlock();
   
    /* Program the user Flash area word by word
     (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/ 
    while(i < DataNumber)
    {  
      if(totalDataNumber < maxTotalDataNumber)
      { 
        if (FLASH_ProgramByte(dataStartAddr+totalDataNumber, Data[i]) == FLASH_COMPLETE)
        {
            i++;
            totalDataNumber += 1;
        }
        else
        { 
           /* Error occurred while writing data in Flash memory. 
             User can add here some code to deal with this error */
           /* Lock the Flash to disable the flash control register access (recommended
           to protect the FLASH memory against possible unwanted operation) *********/
           FLASH_Lock();
           return 1;
        }
     }
     else
     {
       //data overflow occurred, extra datas will be omitted
       /* Lock the Flash to disable the flash control register access (recommended
       to protect the FLASH memory against possible unwanted operation) *********/
       FLASH_Lock();
       return 2;
     }
    }
    
    /* Lock the Flash to disable the flash control register access (recommended
       to protect the FLASH memory against possible unwanted operation) *********/
    FLASH_Lock();
    return 0;
}


/**
  * @brief  Write Data[] to flash
  * @param  ReadData[]: Data to be read
  * @param  totalDataNumber: Number of Data[]
  * @param  dataStartAddr: start of dataAddr
  * @retval char data(0 for success, 1 for failure)
  */
char flash_readIndex(uint32_t readIndexList[], uint8_t readIndexLength, uint8_t* readIndexCount)
{
  *readIndexCount = 0;
  uint32_t readIndexStartAddr = FLASH_USER_START_Index_ADDR;
  uint32_t tempIndex = *(__IO uint32_t*)(readIndexStartAddr+(*readIndexCount)*4);
  
  while (tempIndex != (uint32_t)0xFFFFFFFF)//判断flash user index area是否有存储内容
  {
    uint8_t iLength = 0;
    while(iLength < readIndexLength)
    {  
      readIndexList[*readIndexCount] = *(__IO uint32_t*)(readIndexStartAddr+(*readIndexCount)*4);
      iLength++;
      (*readIndexCount)++;
    }
    tempIndex = *(__IO uint32_t*)(readIndexStartAddr+(*readIndexCount)*4);
  }
  return 0;
}
/**
  * @brief  Read Data from flash
  * @param  ReadData[]: Data to be read
  * @param  totalDataNumber: Number of ReadData[]
  * @params tartDataNumber: start number of ReadData[]
  * @param  dataStartAddr: start of dataAddr
  * @retval char data(0 for success, 1 for failure)
  */
char flash_readData(uint8_t ReadData[], uint32_t totalDataNumber, uint32_t startDataNumber, uint32_t dataStartAddr)
{
  uint32_t i = 0;
  
  while (i < totalDataNumber)
  {
    ReadData[i] = *(__IO uint32_t*)(dataStartAddr+startDataNumber+i);
    i++;
  }
  return 0;
}


/**
  * @brief  Gets the sector of a given address
  * @param  None
  * @retval The sector of a given address
  */
uint32_t GetSector(uint32_t Address)
{
  uint32_t sector = 0;
  
  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_Sector_0;  
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_Sector_1;  
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_Sector_2;  
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_Sector_3;  
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_Sector_4;  
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_Sector_5;  
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_Sector_6;  
  }
  else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
  {
    sector = FLASH_Sector_7;  
  }
  else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
  {
    sector = FLASH_Sector_8;  
  }
  else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
  {
    sector = FLASH_Sector_9;  
  }
  else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
  {
    sector = FLASH_Sector_10;  
  }
  else/*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_11))*/
  {
    sector = FLASH_Sector_11;  
  }

  return sector;
}
/**
  * @}
  */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
