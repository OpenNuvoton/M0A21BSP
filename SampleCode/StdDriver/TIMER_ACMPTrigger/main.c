/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * @brief    Use ACMP to trigger Timer0 counter reset mode.
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/
#include <stdio.h>
#include "NuMicro.h"


void TMR0_IRQHandler(void)
{
    printf("ACMP triggered timer reset while counter is at %d\n", TIMER_GetCaptureData(TIMER0));
    /* Clear timer capture interrupt flag. */
    TIMER_ClearCaptureIntFlag(TIMER0);
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
    CLK->PCLKDIV = (CLK_PCLKDIV_APB0DIV_DIV2 | CLK_PCLKDIV_APB1DIV_DIV2);

    /* Switch UART0 clock source to HIRC */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));

    /* Enable UART peripheral clock */
    CLK_EnableModuleClock(UART0_MODULE);

    /* Enable IP clock */
    CLK_EnableModuleClock(TMR0_MODULE);
    CLK_EnableModuleClock(ACMP01_MODULE);

    /* Select IP clock source */
    CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_HIRC, 0);

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate PllClock, SystemCoreClock and CycylesPerUs automatically. */
    SystemCoreClockUpdate();

    /*----------------------------------------------------------------------*/
    /* Init I/O Multi-function                                              */
    /*----------------------------------------------------------------------*/
    /* Set PB multi-function pins for UART0 RXD=PB.6 and TXD=PB.4 */
    SYS->GPB_MFP1 = (SYS->GPB_MFP1 & ~(SYS_GPB_MFP1_PB4MFP_Msk | SYS_GPB_MFP1_PB6MFP_Msk)) |
                    (SYS_GPB_MFP1_PB4MFP_UART0_TXD | SYS_GPB_MFP1_PB6MFP_UART0_RXD);

    /* Set PC.0 multi-function pin for ACMP1_P0 positive input pin */
    SYS->GPC_MFP0 = (SYS->GPC_MFP0 & ~(SYS_GPC_MFP0_PC0MFP_Msk)) |
                    (SYS_GPC_MFP0_PC0MFP_ACMP1_P0);
    /* Disable digital input path of analog pin ACMP1_P0 to prevent leakage */
    GPIO_DISABLE_DIGITAL_PATH(PC, BIT0);
    /* Set PC.0 to input mode for ACMP analog input pins */
    GPIO_SetMode(PC, BIT0, GPIO_MODE_INPUT);

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
}

int main(void)
{
    int volatile i;

    /* Init System, IP clock and multi-function I/O. */
    SYS_Init();

    /* Init UART0 for printf */
    UART0_Init();

    PC1 = 1;    /* Set init state to high */
    GPIO_SetMode(PC, BIT1, GPIO_MODE_OUTPUT);

    printf("\nThis sample code demonstrate ACMP trigger timer counter reset mode.\n");
    printf("Please connect PC.1 with ACMP0 positive input pin PC.0,  press any key to continue\n");
    getchar();

    /* Give a dummy target frequency here. Will over write capture resolution with macro */
    TIMER_Open(TIMER0, TIMER_PERIODIC_MODE, 1000000);

    /* Update prescale to set proper resolution. */
    TIMER_SET_PRESCALE_VALUE(TIMER0, 0);

    /* Set compare value as large as possible, so don't need to worry about counter overrun too frequently. */
    TIMER_SET_CMP_VALUE(TIMER0, TIMER_CMP_MAX_VALUE);

    /* Configure Timer 0 free counting mode */
    TIMER_EnableCapture(TIMER0, TIMER_CAPTURE_COUNTER_RESET_MODE, TIMER_CAPTURE_EVENT_RISING);
    /* Set capture source from Internal event */
    TIMER0->CTL = (TIMER0->CTL & ~(TIMER_CTL_CAPSRC_Msk)) | TIMER_CAPSRC_INTERNAL;
    /* Set capture source from Internal event ACMP1 */
    TIMER0->EXTCTL = (TIMER0->EXTCTL & ~(TIMER_EXTCTL_INTERCAPSEL_Msk)) | TIMER_INTERCAPSEL_ACMP1;

    /* Start Timer 0 */
    TIMER_Start(TIMER0);

    /* Select VDDA as CRV source */
    ACMP_SELECT_CRV_SRC(ACMP01, ACMP_VREF_CRVSSEL_VDDA);
    /* Select CRV level: VDDA * 9 / 24 */
    ACMP_CRV_SEL(ACMP01, 5);
    /* Configure ACMP1. Enable ACMP1 and select CRV voltage as the source of ACMP negative input. */
    ACMP_Open(ACMP01, 1, ACMP_CTL_NEGSEL_CRV, ACMP_CTL_HYSTERESIS_DISABLE);
    /* Select P0 as ACMP1 positive input channel */
    ACMP_SELECT_P(ACMP01, 1, ACMP_CTL_POSSEL_P0);

    /* Enable timer interrupt */
    TIMER_EnableCaptureInt(TIMER0);
    NVIC_EnableIRQ(TMR0_IRQn);

    while(1)
    {
        PC1 = 0;
        CLK_SysTickDelay(10000);
        PC1 = 1;
        CLK_SysTickDelay(10000);
    }
}
