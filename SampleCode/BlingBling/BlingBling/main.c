/**************************************************************************//**
 * @file     main.c
 * @version  V3.00
 * @brief    Show how to set GPIO pin to control ARGB led.
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2023 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include "stdio.h"
#include "NuMicro.h"
#include <string.h>

#include "ARGB.h"
#include "Word_Table.h"

#define DATA_FLASH_BASE       0x3000

#define RX_BUFFER_SIZE              50
#define RX_TIMEOUT_CNT              50 //40~255

extern uint8_t showArray_row;
extern uint8_t showArray_column;
extern uint8_t showArray[showArrayRow][showArrayColumn];

int IsDebugFifoEmpty(void);

char String[RX_BUFFER_SIZE] = "nuvoTon";
char flashData[RX_BUFFER_SIZE];

enum
{
    eUART_RX_Received_Data_Finish = 0,
    eUART_RX_Received_Data_NOT_Finish
};

volatile uint8_t receiveData = FALSE;
volatile uint8_t g_au8UART_RX_Buffer[RX_BUFFER_SIZE] = {0}; // UART Rx received data Buffer (RAM)
volatile uint8_t g_bUART_RX_Received_Data_State = eUART_RX_Received_Data_NOT_Finish;
volatile uint8_t g_u8UART_RDA_Trigger_Cnt = 0; // UART RDA interrupt trigger times counter
volatile uint8_t g_u8UART_RXTO_Trigger_Cnt = 0; // UART RXTO interrupt trigger times counter
volatile uint8_t u16UART_RX_Buffer_Index = 0;

/*---------------------------------------------------------------------------------------------------------*/
/* Define functions prototype                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
void Read_DataFlash(void);
void Convert_FlashData(void);
void Write_DataFlash(void);


int32_t fill_data_pattern(uint32_t u32StartAddr, uint32_t u32EndAddr, char u32Pattern[])
{
    uint32_t u32Addr;
    uint8_t i = 0;

    for (u32Addr = u32StartAddr; u32Addr < u32EndAddr; u32Addr += 4)
    {
        FMC_Write(u32Addr, (uint32_t)u32Pattern[i++]);
    }
    return 0;
}


void flash_read(uint32_t u32StartAddr, uint32_t u32EndAddr)
{
    uint32_t    u32Addr, u32data;
    uint32_t j, count = 0;
    char temp;

    for (u32Addr = u32StartAddr; u32Addr < u32EndAddr; u32Addr+=4)
    {
        u32data = FMC_Read(u32Addr);

        for(j = 0; j < 4; j++)
        {
            temp = ((u32data >> (j*8)) & 0xFF);
            flashData[count++] = temp;
        }
    }
}

void flash_write(uint32_t u32StartAddr, uint32_t u32EndAddr, uint32_t u32Pattern[])
{
    uint32_t u32Addr;
    uint32_t i = 0;

    for (u32Addr = u32StartAddr; u32Addr < u32EndAddr; u32Addr+=4)
    {
        FMC_Write(u32Addr, (uint32_t)u32Pattern[i++]);
    }
}

static int  set_data_flash_base(uint32_t u32DFBA)
{
    uint32_t   au32Config[2];

    /* Read User Configuration 0 & 1 */
    if (FMC_ReadConfig(au32Config, 2) < 0)
    {
        return -1;
    }

    /* Check if Data Flash is enabled (CONFIG0[0]) and is expected address (CONFIG1) */
    if ((!(au32Config[0] & 0x1)) && (au32Config[1] == u32DFBA))
        return 0;

    FMC_ENABLE_CFG_UPDATE();

    /* Erase User Configuration */
    FMC_Erase(FMC_CONFIG_BASE);

    au32Config[0] &= ~0x1;         /* CONFIG0[0] = 0 (Enabled) / 1 (Disabled) */
    au32Config[1] = u32DFBA;

    /* Update User Configuration settings. */
    if (FMC_WriteConfig(au32Config, 2) < 0)
        return -1;

    /* To check if all the debug messages are finished */
    while(!IsDebugFifoEmpty());

    /* Perform chip reset to make new User Config take effect */
    SYS->IPRST0 = SYS_IPRST0_CHIPRST_Msk;
    return 0;
}

void SYS_Init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Enable HIRC */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);

    /* Waiting for HIRC clock ready */
    CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);

    /* Switch HCLK clock source to HIRC */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));

    /* Set both PCLK0 and PCLK1 as HCLK/2 */
    CLK->PCLKDIV = (CLK_PCLKDIV_APB0DIV_DIV1 | CLK_PCLKDIV_APB1DIV_DIV1);

    /* Switch UART0 clock source to HIRC */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));

    /* Enable UART peripheral clock */
    CLK_EnableModuleClock(UART0_MODULE);
    CLK_EnableModuleClock(TMR0_MODULE);

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate PllClock, SystemCoreClock and CycylesPerUs automatically. */
    SystemCoreClockUpdate();

    /*----------------------------------------------------------------------*/
    /* Init I/O Multi-function                                              */
    /*----------------------------------------------------------------------*/
    /* Set PB multi-function pins for UART0 RXD=PB.6 and TXD=PB.4 */
    SYS->GPB_MFP1 = (SYS->GPB_MFP1 & ~(SYS_GPB_MFP1_PB4MFP_Msk | SYS_GPB_MFP1_PB6MFP_Msk)) |
                    (SYS_GPB_MFP1_PB4MFP_UART0_TXD | SYS_GPB_MFP1_PB6MFP_UART0_RXD);

    /* Lock protected registers */
    SYS_LockReg();
}

/*----------------------------------------------------------------------*/
/* Init UART0                                                           */
/*----------------------------------------------------------------------*/
void UART0_Init(void)
{
    /* Reset UART0 */
    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 baud rate */
    UART_Open(UART0, 115200);

    /* Enable UART RDA/THRE/Time-out interrupt */
    UART_EnableInt(UART0, UART_INTEN_RDAIEN_Msk);
    NVIC_EnableIRQ(UART0_IRQn);

    /* Set RX Trigger Level as 4 bytes */
    UART0->FIFO = ((UART0->FIFO & (~UART_FIFO_RFITL_Msk)) | UART_FIFO_RFITL_1BYTE);

    /* Set Timeout time counter in 60 bit-time and enable time-out counter */
    UART_SetTimeoutCnt(UART0, RX_TIMEOUT_CNT);
}

/*---------------------------------------------------------------------------------------------------------*/
/* ISR to handle UART0 interrupt event                                                            */
/*---------------------------------------------------------------------------------------------------------*/
void UART0_IRQHandler(void)
{
    uint8_t tmpChar;
    static uint16_t u16UART_RX_Buffer_Index = 0;

    if(UART_GET_INT_FLAG(UART0, UART_INTSTS_RDAINT_Msk))
    {
        /* UART receive data available flag */

        /* Record RDA interrupt trigger times */
        g_u8UART_RDA_Trigger_Cnt++;

        tmpChar = UART_READ(UART0);
        if(tmpChar == 0x0D)
        {
            u16UART_RX_Buffer_Index = 0;
            g_bUART_RX_Received_Data_State = eUART_RX_Received_Data_Finish;
            receiveData = TRUE;
        }
        else
        {
            UART_WRITE(UART0, tmpChar);

            g_au8UART_RX_Buffer[u16UART_RX_Buffer_Index] = tmpChar;
            u16UART_RX_Buffer_Index ++;

            if (u16UART_RX_Buffer_Index >= RX_BUFFER_SIZE)
                u16UART_RX_Buffer_Index = 0;
        }
    }
}

/*---------------------------------------------------------------------------------------------------------*/
/* Init TIMER0                                                                                             */
/*---------------------------------------------------------------------------------------------------------*/
void TIMER0_Init(void)
{
    /* Configure and set TIMER0 */
    // Give a dummy target frequency here. Will over write prescale and compare value with macro
    TIMER_Open(TIMER0, TIMER_PERIODIC_MODE, 48000000);

    // Update prescale and compare value to what we need in event counter mode.
    TIMER_SET_PRESCALE_VALUE(TIMER0, 0);
    TIMER_SET_CMP_VALUE(TIMER0, 160);

    // Counter increase on rising edge
    TIMER_EnableEventCounter(TIMER0, TIMER_COUNTER_EVENT_RISING);
}

int32_t main(void)
{

    /* Init System, IP clock and multi-function I/O. */
    SYS_Init();

    /* Init UART0 for printf */
    UART0_Init();

    /* Init ARGB */
    ARGB_init();

    TIMER_Delay(TIMER0, 1000000);

    Read_DataFlash();

    Convert_FlashData();

    printf("+-----------------------------------------------------------+\n");
    printf("|  Start to ""BlingBling"" Your Name!                       |\n");
    printf("+-----------------------------------------------------------+\n");
    printf("Please type your name and enter to send on BlingBling board.\n");

    while(1)
    {
        if(receiveData == FALSE)
        {
            put_string(String);
        }
        else
        {
            Write_DataFlash();

            receiveData = FALSE;
        }
    }
}

/**
  * @brief  Read DataFlash
  * @param  None
  * @retval None
  */
void Read_DataFlash()
{
    SYS_UnlockReg();

    /* Enable FMC ISP function */
    FMC_Open();

    /* Enable Data Flash and set base address. */
    set_data_flash_base(DATA_FLASH_BASE);

    FMC_ENABLE_AP_UPDATE();

    flash_read(DATA_FLASH_BASE, DATA_FLASH_BASE + RX_BUFFER_SIZE);

    FMC_DISABLE_AP_UPDATE();

    FMC_Close();
}

/**
  * @brief  Convert flash data to an array that used to show a string
  * @param  None
  * @retval None
  */
void Convert_FlashData()
{
    uint8_t i = 0;

    if((uint8_t)flashData[0] != 0xFF)
    {
        for(i = 0; i < RX_BUFFER_SIZE; i++)
        {
            String[i] = 0;
        }

        for(i = 0; i < RX_BUFFER_SIZE; i++)
        {
            if((uint8_t)flashData[i] != 0xFF)
                String[i] = flashData[i];
            else
                break;
        }

        for(i = 0; i < RX_BUFFER_SIZE; i++)
        {
            flashData[i] = 0;
        }
    }
}

/**
  * @brief  Write data to DataFlash
  * @param  None
  * @retval None
  */
void Write_DataFlash()
{
    uint8_t i = 0;

    for(i = 0; i < RX_BUFFER_SIZE; i++)
    {
        if((g_au8UART_RX_Buffer[i] < 0x20) || (g_au8UART_RX_Buffer[i] > 0x7E))
        {
            if(g_au8UART_RX_Buffer[i] == 0x0D)
                continue;
            if(g_au8UART_RX_Buffer[i] != 0x00)
                return;
        }
    }

    for(i = 0; i < RX_BUFFER_SIZE; i++)
    {
        String[i] = 0;
    }

    for(i = 0; i < RX_BUFFER_SIZE; i++)
    {
        if(g_au8UART_RX_Buffer[i] != 0x0D)
            String[i] = g_au8UART_RX_Buffer[i];
        else
            String[i] = 0x00;

        g_au8UART_RX_Buffer[i] = 0;
    }

    printf("\nYour name is %s\n", String);
    printf("Now you can BlingBling your name anytime and anywhere!\n");

    SYS_UnlockReg();

    /* Enable FMC ISP function */
    FMC_Open();

    FMC_ENABLE_AP_UPDATE();

    /* Erase page */
    FMC_Erase(DATA_FLASH_BASE);

    flash_write(DATA_FLASH_BASE, DATA_FLASH_BASE + RX_BUFFER_SIZE, (uint32_t*)String);

    FMC_DISABLE_AP_UPDATE();

    FMC_Close();

    g_u8UART_RDA_Trigger_Cnt = 0;
    g_u8UART_RXTO_Trigger_Cnt = 0;
    u16UART_RX_Buffer_Index = 0;

    showArray_row = 0;
    showArray_column = 0;
}
