/******************************************************************************
 * @file     LDROM_main.c
 * @version  V1.00
 * $Revision: 4 $
 * $Date: 20/05/28 5:40p $
 * @brief    This sample code includes LDROM image (fmc_ld_iap)
 *           and APROM image (fmc_ap_main).
 *           It shows how to branch between APROM and LDROM. To run
 *           this sample code, the boot mode must be "Boot from APROM
 *           with IAP".
 *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/
#include <stdio.h>
#include "NuMicro.h"

typedef void (FUNC_PTR)(void);

void SYS_Init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Enable HIRC clock */
    CLK->PWRCTL |= CLK_PWRCTL_HIRCEN_Msk;

    /* Waiting for HIRC clock ready */
    while((CLK->STATUS & CLK_STATUS_HIRCSTB_Msk) != CLK_STATUS_HIRCSTB_Msk);

    /* Switch HCLK clock source to HIRC */
    CLK->CLKSEL0 = (CLK->CLKSEL0 & ~CLK_CLKSEL0_HCLKSEL_Msk ) | CLK_CLKSEL0_HCLKSEL_HIRC ;

    /* Enable UART0 clock */
    CLK->APBCLK0 |= CLK_APBCLK0_UART0CKEN_Msk ;

    /* Switch UART0 clock source to HIRC */
    CLK->CLKSEL1 = (CLK->CLKSEL1 & ~CLK_CLKSEL1_UART0SEL_Msk) | CLK_CLKSEL1_UART0SEL_HIRC;

    /* Update System Core Clock */
    SystemCoreClockUpdate();

    /* Set PB multi-function pins for UART0 RXD=PB.6 and TXD=PB.4 */
    SYS->GPB_MFP1 = (SYS->GPB_MFP1 & ~(SYS_GPB_MFP1_PB4MFP_Msk | SYS_GPB_MFP1_PB6MFP_Msk)) |        \
                    (SYS_GPB_MFP1_PB4MFP_UART0_TXD | SYS_GPB_MFP1_PB6MFP_UART0_RXD);

    /* Lock protected registers */
    SYS_LockReg();
}

void UART_Init()
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init UART                                                                                               */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Configure UART0 and set UART0 baud rate */
    UART0->BAUD = UART_BAUD_MODE2 | UART_BAUD_MODE2_DIVIDER(__HIRC, 115200);
    UART0->LINE = UART_WORD_LEN_8 | UART_PARITY_NONE | UART_STOP_BIT_1;

}


#ifdef __ARMCC_VERSION
__asm __set_SP(uint32_t _sp)
{
    MSR MSP, r0
    BX lr
}
#endif

/*
 * @returns     Send value from UART debug port
 * @details     Send a target char to UART debug port .
 */
static void SendChar_ToUART(int ch)
{
    while (UART0->FIFOSTS & UART_FIFOSTS_TXFULL_Msk);

    UART0->DAT = ch;
    if(ch == '\n')
    {
        while (UART0->FIFOSTS & UART_FIFOSTS_TXFULL_Msk);
        UART0->DAT = '\r';
    }
}

/**
 * @brief    Routine to get a char
 * @param    None
 * @returns  Get value from UART debug port or semihost
 * @details  Wait UART debug port or semihost to input a char.
 */
static char GetChar(void)
{
    while(1)
    {
        if ((UART0->FIFOSTS & UART_FIFOSTS_RXEMPTY_Msk) == 0)
        {
            return (UART0->DAT);
        }
    }
}

static void PutString(char *str)
{
    while (*str != '\0')
    {
        SendChar_ToUART(*str++);
    }
}

#ifdef __GNUC__                        /* for GNU C compiler */
/**
 * @brief       Hard fault handler
 * @param[in]   stack pointer points to the dumped registers in SRAM
 * @return      None
 * @details     Replace while(1) at the end of this function with chip reset if WDT is not enabled for end product
 */
void Hard_Fault_Handler(uint32_t stack[])
{
    PutString("In Hard Fault Handler\n");
    while(1);
}
#endif

int main()
{
#ifdef __GNUC__                        /* for GNU C compiler */
    uint32_t    u32Data;
#endif
    FUNC_PTR    *func;                 /* function pointer */

    SYS_Init();                        /* Init System, IP clock and multi-function I/O */

    UART_Init();                      /* Initialize UART0 */

    PutString("\n\n");
    PutString("+------------------------------------+\n");
    PutString("|      M0A21 FMC IAP Sample Code     |\n");
    PutString("|            [LDROM code]            |\n");
    PutString("+------------------------------------+\n");

    SYS_UnlockReg();                   /* Unlock protected registers */

    FMC_Open();                        /* Enable FMC ISP function */

    PutString("\n\nPress any key to branch to APROM...\n");
    GetChar();                         /* block on waiting for any one character input from UART0 */

    PutString("\n\nChange VECMAP and branch to APROM...\n");
    while (!(UART0->FIFOSTS & UART_FIFOSTS_TXEMPTY_Msk));       /* wait until UART0 TX FIFO is empty */

    /*  NOTE!
     *     Before change VECMAP, user MUST disable all interrupts.
     */
    FMC_SetVectorPageAddr(FMC_APROM_BASE);        /* Vector remap APROM page 0 to address 0. */
    SYS_LockReg();                                /* Lock protected registers */

    /*
     *  The reset handler address of an executable image is located at offset 0x4.
     *  Thus, this sample get reset handler address of APROM code from FMC_APROM_BASE + 0x4.
     */
    func = (FUNC_PTR *)*(uint32_t *)(FMC_APROM_BASE + 4);

    /*
     *  The stack base address of an executable image is located at offset 0x0.
     *  Thus, this sample get stack base address of APROM code from FMC_APROM_BASE + 0x0.
     */
#ifdef __GNUC__                        /* for GNU C compiler */
    u32Data = *(uint32_t *)FMC_LDROM_BASE;
    asm("msr msp, %0" : : "r" (u32Data));
#else
    __set_SP(*(uint32_t *)FMC_APROM_BASE);
#endif
    /*
     *  Branch to the LDROM code's reset handler in way of function call.
     */
    func();

    while (1);
}

/*** (C) COPYRIGHT 2020 Nuvoton Technology Corp. ***/
