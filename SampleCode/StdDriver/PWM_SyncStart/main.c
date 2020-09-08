/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 2 $
 * $Date: 20/05/28 1:40p $
 * @brief    Demonstrate how to use PWM counter synchronous start function.
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "NuMicro.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Macro, type and constant definitions                                                                    */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/


void SYS_Init(void)
{

    /* Unlock protected registers */
    SYS_UnlockReg();

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

    /* Set PC multi-function pins for PWM0 Channel 2/3 */
    SYS->GPC_MFP0 = (SYS->GPC_MFP0 & ~(SYS_GPC_MFP0_PC0MFP_Msk | SYS_GPC_MFP0_PC1MFP_Msk)) |
                    (SYS_GPC_MFP0_PC0MFP_PWM0_CH2 | SYS_GPC_MFP0_PC1MFP_PWM0_CH3);
}

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{

    /* Unlock protected registers */
//    SYS_UnlockReg();

    /* Init System, IP clock and multi-function I/O */
    SYS_Init();

    /* Lock protected registers */
    SYS_LockReg();

    /* Reset UART0 module */
    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 baud rate */
    UART_Open(UART0, 115200);

    printf("+------------------------------------------------------------------------+\n");
    printf("|                          PWM Driver Sample Code                        |\n");
    printf("|                                                                        |\n");
    printf("+------------------------------------------------------------------------+\n");
    printf("  This sample code will output waveform with PWM0 channel 0~5 at the same time.\n");
    printf("  I/O configuration:\n");
    printf("\n waveform output pin: PWM0_CH0(PD.2), PWM0_CH1(PD.3), PWM0_CH2(PC.0), PWM0_CH3(PC.1), PWM0_CH4(PD.0), PWM0_CH5(PD.1)\n");

    /* PWM0 channel 0~5 frequency and duty configuration are as follows */
    PWM_ConfigOutputChannel(PWM0, 0, 1000, 50);
    PWM_ConfigOutputChannel(PWM0, 1, 1000, 50);
    PWM_ConfigOutputChannel(PWM0, 2, 1000, 50);
    PWM_ConfigOutputChannel(PWM0, 3, 1000, 50);
    PWM_ConfigOutputChannel(PWM0, 4, 1000, 50);
    PWM_ConfigOutputChannel(PWM0, 5, 1000, 50);

    /* Enable counter synchronous start function for PWM0 channel 0~5 */
    PWM_ENABLE_TIMER_SYNC(PWM0, 0x3F, PWM_SSCTL_SSRC_PWM0);

    /* Enable output of PWM0 channel 0~5 */
    PWM_EnableOutput(PWM0, 0x3F);

    printf("Press any key to start.\n");
    getchar();

    /* Trigger PWM counter synchronous start by PWM0 */
    PWM_TRIGGER_SYNC_START(PWM0);

    printf("Done.\n");
    while(1);

}
