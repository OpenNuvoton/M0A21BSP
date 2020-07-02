/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 5 $
 * $Date: 20/07/02 5:45p $
 * @brief    Demonstrate how to use PWM Dead Zone function.
 * @note
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (C) 2017-2020 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include "NuMicro.h"


/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/

/**
 * @brief       PWM0 IRQ Handler
 *
 * @param       None
 *
 * @return      None
 *
 * @details     ISR to handle PWM0 interrupt event
 */
void PWM0_IRQHandler(void)
{
    static uint32_t cnt;
    static uint32_t out;

    /* Channel 0 frequency is 6,000 Hz, every 1 second enter this IRQ handler 6,000 times. */
    if(++cnt == 6000)
    {
        if(out)   
            PWM_EnableOutput(PWM0, PWM_CH_0_MASK|PWM_CH_1_MASK|PWM_CH_4_MASK|PWM_CH_5_MASK);
        else
            PWM_DisableOutput(PWM0, PWM_CH_0_MASK|PWM_CH_1_MASK|PWM_CH_4_MASK|PWM_CH_5_MASK);

        out ^= 1;
        cnt = 0;
    }
    /* Clear channel 0 period interrupt flag */
    PWM_ClearPeriodIntFlag(PWM0, 0);
}

void SYS_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Enable HIRC clock */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);

    /* Waiting for HIRC clock ready */
    CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);

    /* Switch HCLK clock source to HIRC and HCLK source divide 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));

    /* Select HIRC as the clock source of UART0 */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));

    /* Enable UART peripheral clock */
    CLK_EnableModuleClock(UART0_MODULE);

    /* Enable PWM0 module clock */
    CLK_EnableModuleClock(PWM0_MODULE);

    /* Reset PWM0 module */
    SYS_ResetModule(PWM0_RST);

    /* Update System Core Clock */
    SystemCoreClockUpdate();

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set PB multi-function pins for UART0 RXD=PB.6 and TXD=PB.4 */
    SYS->GPB_MFP1 = (SYS->GPB_MFP1 & ~(SYS_GPB_MFP1_PB4MFP_Msk | SYS_GPB_MFP1_PB6MFP_Msk)) |        \
                    (SYS_GPB_MFP1_PB4MFP_UART0_TXD | SYS_GPB_MFP1_PB6MFP_UART0_RXD);

    /* Set PD multi-function pins for PWM0 Channel 0/1, 4/5 */
    SYS->GPD_MFP0 = (SYS->GPD_MFP0 & ~(SYS_GPD_MFP0_PD0MFP_Msk | SYS_GPD_MFP0_PD1MFP_Msk)) |
                    (SYS_GPD_MFP0_PD0MFP_PWM0_CH4 | SYS_GPD_MFP0_PD1MFP_PWM0_CH5);

	SYS->GPD_MFP0 = (SYS->GPD_MFP0 & ~(SYS_GPD_MFP0_PD2MFP_Msk | SYS_GPD_MFP0_PD3MFP_Msk)) |
                    (SYS_GPD_MFP0_PD2MFP_PWM0_CH0 | SYS_GPD_MFP0_PD3MFP_PWM0_CH1);
}


/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{

    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Init System, IP clock and multi-function I/O */
    SYS_Init();

    /* Lock protected registers */
    SYS_LockReg();

    /* Reset UART module */
    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 baud rate */
    UART_Open(UART0, 115200);

    printf("+------------------------------------------------------------------------+\n");
    printf("|                          PWM Driver Sample Code                        |\n");
    printf("|                                                                        |\n");
    printf("+------------------------------------------------------------------------+\n");
    printf("  This sample code will output PWM0 channel 0/1, 4/5 with different \n");
    printf("  frequency and duty, enable dead zone function of all PWM0 pairs.\n");
    printf("  And also enable/disable PWM output every 1 second.\n");
    printf("  I/O configuration:\n");
    printf("    waveform output pin: PWM0_CH0(PD.2), PWM0_CH1(PD.3), PWM0_CH4(PD.0), PWM0_CH5(PD.1)\n");

    /* Set Pwm mode as complementary mode */
    PWM_ENABLE_COMPLEMENTARY_MODE(PWM0);

    /* PWM0 channel 0 frequency is 6,000 Hz, duty 30% */
    PWM_ConfigOutputChannel(PWM0, 0, 6000, 30);
    SYS_UnlockReg();
    PWM_EnableDeadZone(PWM0, 0, 400);
    SYS_LockReg();

    /* PWM0 channel 4 frequency is 3,000 Hz, duty 50% */
    PWM_ConfigOutputChannel(PWM0, 4, 3000, 50);
    SYS_UnlockReg();
    PWM_EnableDeadZone(PWM0, 4, 200);
    SYS_LockReg();

    /* Enable output of PWM0 channel 0/1, 4/5 */
	PWM_EnableOutput(PWM0, PWM_CH_0_MASK|PWM_CH_1_MASK|PWM_CH_4_MASK|PWM_CH_5_MASK);

    /* Enable PWM0 channel 0 period interrupt, use channel 0 to measure time. */
    PWM_EnablePeriodInt(PWM0, 0, 0);
    NVIC_EnableIRQ(PWM0_IRQn);

    /* Start */
    PWM_Start(PWM0, PWM_CH_0_MASK|PWM_CH_1_MASK|PWM_CH_4_MASK|PWM_CH_5_MASK);

    while(1);
}
