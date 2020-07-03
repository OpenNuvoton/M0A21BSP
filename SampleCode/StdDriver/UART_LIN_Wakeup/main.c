/**************************************************************************//**
 * @file     main.c
 * @version  V3.00
 * @brief
 *           LIN Power-down/Wake-up and header and response.
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include "stdio.h"
#include "NuMicro.h"

#define LIN_ID      0x30

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
volatile int32_t g_i32pointer;
volatile int32_t g_i32RxCounter;
uint8_t g_u8SendData[12];
uint8_t g_u8ReceiveData[9];

/*---------------------------------------------------------------------------------------------------------*/
/* Define functions prototype                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
void UART_FunctionTest(void);
void LIN_Send_Wakeup_FunctionTest(void);
void LIN_PowerDown_Wakeup_FunctionTest(void);


void UART1_IRQHandler(void)
{
    volatile uint32_t u32IntSts = UART1->INTSTS;

    if(UART_GET_INT_FLAG(UART1, UART_INTSTS_WKIF_Msk))
    {
        g_i32RxCounter=0;
        UART_ClearIntFlag(UART1, UART_INTSTS_WKINT_Msk);
    }

    if(u32IntSts & UART_INTSTS_LINIF_Msk)
    {
        if(UART1->LINSTS & UART_LINSTS_SLVHDETF_Msk)
        {
            // Clear LIN slave header detection flag
            UART1->LINSTS = UART_LINSTS_SLVHDETF_Msk;
            printf("\n LIN Slave Header detected ");
        }

        if(UART1->LINSTS & (UART_LINSTS_SLVHEF_Msk | UART_LINSTS_SLVIDPEF_Msk | UART_LINSTS_BITEF_Msk))
        {
            // Clear LIN error flag
            UART1->LINSTS = (UART_LINSTS_SLVHEF_Msk | UART_LINSTS_SLVIDPEF_Msk | UART_LINSTS_BITEF_Msk);
            printf("\n LIN error detected ");
        }
    }

    if(u32IntSts & UART_INTSTS_RDAIF_Msk)
    {
        g_u8ReceiveData[g_i32RxCounter] = UART1->DAT;
        g_i32RxCounter++;

    }

}

void SYS_Init(void)
{

    /* Set XT1_OUT(PF.2) and XT1_IN(PF.3) to input mode */
    PF->MODE &= ~(GPIO_MODE_MODE2_Msk | GPIO_MODE_MODE3_Msk);

    /* Enable HXT */
    CLK->PWRCTL |= CLK_PWRCTL_HXTEN_Msk; // XTAL12M (HXT) Enabled
    CLK->PWRCTL |= (CLK_PWRCTL_LXTEN_Msk);      // enable LXT for Wakeup Test

    /* Waiting clock ready */
    CLK_WaitClockReady(CLK_STATUS_HXTSTB_Msk);

    /* Enable IP clock */
    CLK->APBCLK0 |= CLK_APBCLK0_UART0CKEN_Msk; // UART0 Clock Enable
    CLK->APBCLK0 |= CLK_APBCLK0_UART1CKEN_Msk; // UART1 Clock Enable

    /* Enable UART module clock */
    CLK_EnableModuleClock(UART0_MODULE);
    CLK_EnableModuleClock(UART1_MODULE);

    /* Select UART clock source is HXT */
    CLK->CLKSEL1 = (CLK->CLKSEL1 & ~CLK_CLKSEL1_UART0SEL_Msk) | (0x0 << CLK_CLKSEL1_UART0SEL_Pos);
    CLK->CLKSEL1 = (CLK->CLKSEL1 & ~CLK_CLKSEL1_UART1SEL_Msk) | (0x0 << CLK_CLKSEL1_UART1SEL_Pos);

    /* Set PB multi-function pins for UART0 RXD=PB.6 and TXD=PB.4 */
    SYS->GPB_MFP1 = (SYS->GPB_MFP1 & ~(SYS_GPB_MFP1_PB4MFP_Msk | SYS_GPB_MFP1_PB6MFP_Msk)) |        \
                    (SYS_GPB_MFP1_PB4MFP_UART0_TXD | SYS_GPB_MFP1_PB6MFP_UART0_RXD);

    /* Set PB multi-function pins for UART1 TXD and RXD */
    SYS->GPB_MFP1 = (SYS->GPB_MFP1 & ~(SYS_GPB_MFP1_PB5MFP_Msk | SYS_GPB_MFP1_PB7MFP_Msk)) |        \
                    (SYS_GPB_MFP1_PB5MFP_UART1_TXD | SYS_GPB_MFP1_PB7MFP_UART1_RXD);

    /* Set PD multi-function pins for UART1 CTS and RTS */
    SYS->GPD_MFP0 = (SYS->GPD_MFP0 & ~(SYS_GPD_MFP0_PD2MFP_Msk | SYS_GPD_MFP0_PD3MFP_Msk)) |    \
                    (SYS_GPD_MFP0_PD2MFP_UART1_nCTS | SYS_GPD_MFP0_PD3MFP_UART1_nRTS);

}

void UART0_Init()
{

    /* Configure UART0 and set UART0 baud rate */
    UART_Open(UART0, 115200);
}


int32_t main(void)
{

    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Init System, peripheral clock and multi-function I/O */
    SYS_Init();

    /* Lock protected registers */
    SYS_LockReg();

    /* Init UART0 for printf and test */
    UART0_Init();

    printf("\n\nCPU @ %d Hz\n", SystemCoreClock);

    printf("\nUART Sample Program\n");

    /* UART sample function */
    UART_FunctionTest();

    while(1);

}

/*---------------------------------------------------------------------------------------------------------*/
/* Compute Parity Value                                                                                    */
/*---------------------------------------------------------------------------------------------------------*/
uint8_t GetParityValue(uint32_t u32id)
{
    uint32_t u32Res = 0, ID[6], p_Bit[2], mask = 0;

    for(mask = 0; mask < 6; mask++)
        ID[mask] = (u32id & (1 << mask)) >> mask;

    p_Bit[0] = (ID[0] + ID[1] + ID[2] + ID[4]) % 2;
    p_Bit[1] = (!((ID[1] + ID[3] + ID[4] + ID[5]) % 2));

    u32Res = u32id + (p_Bit[0] << 6) + (p_Bit[1] << 7);
    return u32Res;
}

/*---------------------------------------------------------------------------------------------------------*/
/* Compute CheckSum Value                                                                                  */
/*---------------------------------------------------------------------------------------------------------*/
uint8_t ComputeChksumValue(uint8_t *pu8Buf, uint32_t u32ByteCnt)
{
    uint32_t i, CheckSum = 0;

    for(i = 0 ; i < u32ByteCnt; i++)
    {
        CheckSum += pu8Buf[i];
        if(CheckSum >= 256)
            CheckSum -= 255;
    }
    return (uint8_t)(255 - CheckSum);
}


/*---------------------------------------------------------------------------------------------------------*/
/*  UART Function Test                                                                                     */
/*---------------------------------------------------------------------------------------------------------*/

void TestItem(void)
{
    printf("+--------------------------------------------------------------------------------+\n");
    printf("|  UART LIN Demo Function Select                                                 |\n");
    printf("+--------------------------------------------------------------------------------+\n");
    printf("|  1 : Demo Power-Down/Wake-up and RX Function                                   |\n");
    printf("|  2 : Demo Send Wake-up signal and TX Function                                  |\n");
    printf("+--------------------------------------------------------------------------------+\n");
    printf("| Quit                                                                   - [ESC] |\n");
    printf("+--------------------------------------------------------------------------------+\n\n");
    printf("Please Select key (1~2): ");
}

void UART_FunctionTest(void)
{
    uint32_t u32Item;

    do
    {
        TestItem();
        u32Item = getchar();
        printf("%c\n", u32Item);

        switch (u32Item)
        {
        case '1':
            LIN_PowerDown_Wakeup_FunctionTest();
            break;

        case '2':
            LIN_Send_Wakeup_FunctionTest();
            break;

        default:
            break;
        }
    }
    while (u32Item != 27);

    printf("\nUART Demo Program End\n");

}

void LIN_Send_Wakeup_FunctionTest(void)
{
    //The sample code will send a LIN header with ID is 0x30 and response field.
    //The response field with 8 data bytes and checksum without including ID.
    uint32_t WakeupPulse;

    uint8_t au8TestPattern[9] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x0}; // 8 data byte + 1 byte checksum

    printf("+--------------------------------------------------------------------------------+\n");
    printf("|  UART LIN Send Wake-up Signal Function Test                                    |\n");
    printf("+--------------------------------------------------------------------------------+\n");
    printf("|  Description :                                                                 |\n");
    printf("|    The sample code will send a Wake-up Signal with 250us,                      |\n");
    printf("|    then send a LIN header with ID is 0x30 and response field                   |\n");
    printf("+--------------------------------------------------------------------------------+\n");

    /* Send break+sync+ID */
    g_i32pointer = 0 ;

    /* Set UART Configuration, LIN Max Speed is 20K */
    UART_SetLine_Config(UART1, 9600, UART_WORD_LEN_8, UART_PARITY_NONE, UART_STOP_BIT_1);

    /* Switch back to LIN Function */
    UART1->FUNCSEL = UART_FUNCSEL_LIN;

    // Set LIN Wake-up Signal length with 250us @HXT clock source
    WakeupPulse = ((250*1000) / (1000000000/__HXT));
    UART1->LINWKCTL = (UART1->LINWKCTL & ~UART_LINWKCTL_LINWKC_Msk) | WakeupPulse;      // uint engine clock.

    /* Send Wake-up Signal */
    UART1->LINWKCTL |= UART_LINWKCTL_SENDLINW_Msk;

    /* Wait slve is wakeup and initialized in 100ms */
    CLK_SysTickDelay(100*1000);

    // Send Header
    UART1->LINCTL = UART_LINCTL_PID(LIN_ID) | UART_LINCTL_HSEL_BREAK_SYNC_ID |
                    UART_LINCTL_BSL(1) | UART_LINCTL_BRKFL(12) | UART_LINCTL_IDPEN_Msk;
    /* LIN TX Send Header Enable */
    UART1->LINCTL |= UART_LINCTL_SENDH_Msk;
    /* Wait until break field, sync field and ID field transfer completed */
    while((UART1->LINCTL & UART_LINCTL_SENDH_Msk) == UART_LINCTL_SENDH_Msk);

    /* Compute checksum without ID and fill checksum value to  au8TestPattern[8] */
    au8TestPattern[8] = ComputeChksumValue(&au8TestPattern[0], 8);
    UART_Write(UART1, &au8TestPattern[0], 9);

    printf("\n UART LIN Send Wake-up Signal Function Demo End !!\n");
}

/*---------------------------------------------------------------------------------------------------------*/
/*  Function for System Entry to Power Down Mode                                                           */
/*---------------------------------------------------------------------------------------------------------*/
void PowerDownFunction(void)
{
    /* Check if all the debug messages are finished */
    UART_WAIT_TX_EMPTY(UART0);

    /* Enter to Power-down mode */
    CLK_PowerDown();
}

void LIN_PowerDown_Wakeup_FunctionTest(void)
{
    uint32_t u32backupBaudReg, u32backupCLKReg ;
    //The sample code will detect LIN header(break+sync+ID) and receive response field.
    //The response field with 8 data bytes and checksum without including ID.

    printf("+--------------------------------------------------------------------------------+\n");
    printf("|  UART LIN Power-down/Wake-up Function Test                                     |\n");
    printf("+--------------------------------------------------------------------------------+\n");
    printf("|  Description :                                                                 |\n");
    printf("|    The sample Wake-up and receive a LIN response field data                    |\n");
    printf("+--------------------------------------------------------------------------------+\n");

    g_i32RxCounter = 0;

    /* reset RX FIFO */
    UART1->FIFO |= UART_FIFO_RXRST_Msk;

    /* Set UART Configuration, LIN Max Speed is 20K */
    UART_SetLine_Config(UART1, 9600, UART_WORD_LEN_8, UART_PARITY_NONE, UART_STOP_BIT_1);

    /* Switch back to LIN Function */
    UART1->FUNCSEL = UART_FUNCSEL_LIN;

    /* Enable RDA\Time-out\LIN Interrupt  */
    UART_ENABLE_INT(UART1, UART_INTEN_WKIEN_Msk);
    NVIC_EnableIRQ(UART1_IRQn);

    // backup register fo wake-up to restore faster
    u32backupCLKReg = CLK->CLKSEL1;
    u32backupBaudReg = UART1->BAUD;

    /* Select LXT as clock source */
    CLK->CLKSEL1 = (CLK->CLKSEL1 & ~CLK_CLKSEL1_UART1SEL_Msk) | CLK_CLKSEL1_UART1SEL_LXT;

    /* Unlock protected registers before entering Power-down mode */
    SYS_UnlockReg();

    /* LIN Wake-up Enable to wailt wake-up signal from master */
    UART1->LINWKCTL |= UART_LINWKCTL_LINWKEN_Msk;

    printf("Power-down\n");
    UART_WAIT_TX_EMPTY(UART0);

    /* Enter to Power-down mode */
    PowerDownFunction();

    /* Lock protected registers after entering Power-down mode */
    SYS_LockReg();

    UART_DISABLE_INT(UART1, UART_INTEN_WKIEN_Msk);
    /* restore setting */
    CLK->CLKSEL1 = u32backupCLKReg;
    UART1->BAUD = u32backupBaudReg;

    printf("wakeup\n");

    UART_ENABLE_INT(UART1, (UART_INTEN_RDAIEN_Msk | UART_INTEN_RXTOIEN_Msk | UART_INTEN_LINIEN_Msk));

    // Receive Header: break+sync+ID
    UART1->LINCTL = UART_LINCTL_PID(LIN_ID) | UART_LINCTL_HSEL_BREAK_SYNC_ID | UART_LINCTL_SLVHDEN_Msk |
                    UART_LINCTL_IDPEN_Msk | UART_LINCTL_MUTE_Msk | UART_LINCTL_SLVEN_Msk;

    while(g_i32RxCounter < 9);

    printf("\n Receive Data:\n [ 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x 0x%x ] \n",
           g_u8ReceiveData[0], g_u8ReceiveData[1], g_u8ReceiveData[2], g_u8ReceiveData[3],
           g_u8ReceiveData[4], g_u8ReceiveData[5], g_u8ReceiveData[6], g_u8ReceiveData[7],
           g_u8ReceiveData[8]);

    NVIC_DisableIRQ(UART1_IRQn);

    printf("\n UART LIN Power-down/Wake-up Function Demo End !!\n");
}



