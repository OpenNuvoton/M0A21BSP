/****************************************************************************
 * @file     main.c
 * @version  V0.10
 * @brief    Demonstrate how to trigger DAC by timer.
 *
 * @copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "stdio.h"
#include "NuMicro.h"

volatile uint16_t g_au16Sine[] = {2047, 2251, 2453, 2651, 2844, 3028, 3202, 3365, 3515, 3650, 3769, 3871, 3954,
                                  4019, 4064, 4088, 4095, 4076, 4040, 3984, 3908, 3813, 3701, 3573, 3429, 3272,
                                  3102, 2921, 2732, 2536, 2335, 2132, 1927, 1724, 1523, 1328, 1141,  962,  794,
                                  639,  497,  371,  262,  171,   99,   45,   12,    0,    7,   35,   84,  151,
                                  238,  343,  465,  602,  754,  919, 1095, 1281, 1475, 1674, 1876
                                 };

const uint32_t g_u32ArraySize = sizeof(g_au16Sine) / sizeof(uint16_t);
static uint32_t g_u32Index = 0;

#if defined (__GNUC__) && !defined(__ARMCC_VERSION) && defined(OS_USE_SEMIHOSTING)
extern void initialise_monitor_handles(void);
#endif

void TIMER_SetTriggerTarget(TIMER_T *timer, uint32_t u32Mask);

void DAC0_IRQHandler(void)
{
    if (DAC_GET_INT_FLAG(DAC, 0))
    {
        if (g_u32Index == g_u32ArraySize)
            g_u32Index = 0;
        else
        {
            DAC_WRITE_DATA(DAC, 0, g_au16Sine[g_u32Index++]);
            /* Clear the DAC conversion complete finish flag */
            DAC_CLR_INT_FLAG(DAC, 0);
        }
    }

    return;
}


void SYS_Init(void)
{

    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Enable HIRC clock */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);

    /* Waiting for HIRC clock ready */
    CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);

    /* Select HCLK clock source as HIRC and and HCLK clock divider as 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));

    /* Enable UART module clock */
    CLK_EnableModuleClock(UART0_MODULE);

    /* Select UART module clock source as HIRC and UART module clock divider as 1 */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));

    /* Enable DAC module clock */
    CLK_EnableModuleClock(DAC0_MODULE);

    /* Enable Timer 0 module clock */
    CLK_EnableModuleClock(TMR0_MODULE);

    /* Select timer 0 module clock source as HIRC */
    CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_HIRC, 0);

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set PB multi-function pins for UART0 RXD=PB.6 and TXD=PB.4 */
    SYS->GPB_MFP1 = (SYS->GPB_MFP1 & ~(SYS_GPB_MFP1_PB4MFP_Msk | SYS_GPB_MFP1_PB6MFP_Msk)) |        \
                    (SYS_GPB_MFP1_PB4MFP_UART0_TXD | SYS_GPB_MFP1_PB6MFP_UART0_RXD);

    /* Lock protected registers */
    SYS_LockReg();
}


int32_t main(void)
{
    uint32_t ii;

    /* Init System, IP clock and multi-function I/O */
    SYS_Init();

#if defined (__GNUC__) && !defined(__ARMCC_VERSION) && defined(OS_USE_SEMIHOSTING)
    initialise_monitor_handles();
#endif

    /* modify the test pattern to 5-bit width */
    for(ii=0; ii<sizeof(g_au16Sine)/2; ii++)
    {
        g_au16Sine[ii] = (g_au16Sine[ii]>>7)&0x1F;
    }

    /* Configure UART0 and set UART0 baud rate */
    UART_Open(UART0, 115200);

    /* Enable DAC output pin function (PA0/PA1) */
    printf("*****************************************************\n");
    printf("*  select PA0 pin function                          *\n");
    printf("*  [0] PA0/PA1 as ICE DAT/CLK                       *\n");
    printf("*  [1] PA0/PA1 as DAC output and VREFF              *\n");
    printf("*****************************************************\n");
    switch(getchar())
    {
    case '0':
    default:
        SYS->GPA_MFP0 &= ~SYS_GPA_MFP0_PA0MFP_Msk;
        SYS->GPA_MFP0 |= SYS_GPA_MFP0_PA0MFP_ICE_DAT;
        SYS->GPA_MFP0 &= ~SYS_GPA_MFP0_PA1MFP_Msk;
        SYS->GPA_MFP0 |= SYS_GPA_MFP0_PA1MFP_ICE_CLK;
        break;
    case '1':
        SYS->GPA_MFP0 &= ~SYS_GPA_MFP0_PA0MFP_Msk;
        SYS->GPA_MFP0 |= SYS_GPA_MFP0_PA0MFP_DAC0_OUT;
        SYS->GPA_MFP0 &= ~SYS_GPA_MFP0_PA1MFP_Msk;
        SYS->GPA_MFP0 |= SYS_GPA_MFP0_PA1MFP_ADC0_CH1;
        break;
    }

    printf("+----------------------------------------------------------+\n");
    printf("|            DAC Driver Sample Code                        |\n");
    printf("+----------------------------------------------------------+\n");
    printf("Hit any key to start!\n");
    getchar();
    /* Set the timer 0 trigger,DAC and enable D/A converter */
    DAC_Open(DAC, 0, DAC_TIMER0_TRIGGER);

    /* The DAC conversion settling time is 1us */
    DAC_SetDelayTime(DAC, 1);

    /* Enable DAC pin output */
    DAC_ENABLE_OUTPUT(DAC);

    /* Select reference voltage from Vreff */
    DAC_VREFFSRC_AVDD(DAC);

    /* Set DAC 5-bit holding data */
    DAC_WRITE_DATA(DAC, 0, g_au16Sine[g_u32Index]);

    /* Clear the DAC conversion complete finish flag for safe */
    DAC_CLR_INT_FLAG(DAC, 0);

    /* Enable the DAC interrupt.  */
    DAC_ENABLE_INT(DAC, 0);
    NVIC_EnableIRQ(DAC0_IRQn);

    /* Configure Timer 0 frequency to 1000Hz and start it. */
    TIMER_Open(TIMER0, TIMER_PERIODIC_MODE, 1000);
    TIMER_SetTriggerTarget(TIMER0, TIMER_TRG_TO_DAC);
    TIMER_Start(TIMER0);

    while (1);

}

/*** (C) COPYRIGHT 2018 Nuvoton Technology Corp. ***/
