/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 4 $
 * $Date: 20/07/02 5:46p $
 * @brief
 *           Change duty cycle of output waveform by configured period.
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "NuMicro.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/


uint32_t CalNewDutyCMR(PWM_T *pwm, uint32_t u32ChannelNum, uint32_t u32DutyCycle, uint32_t u32CycleResolution);

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
}

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

    /* Set PB multi-function pins for PWM0 Channel 0 */
    SYS->GPB_MFP1 = (SYS->GPB_MFP1 & (~SYS_GPB_MFP1_PB5MFP_Msk)) |
                    SYS_GPB_MFP1_PB5MFP_PWM0_CH0;
}

/**
 * @brief       Calculate the comparator value of new duty by configured period
 *
 * @param       pwm                  The pointer of the specified PWM module
 *
 * @param       u32ChannelNum        PWM channel number. Valid values are between 0~5
 *
 * @param       u32DutyCycle         Target generator duty cycle percentage. Valid range are between 0 ~ u32CycleResolution.
 *                                   If u32CycleResolution is 100, and u32DutyCycle is 10 means 10%, 20 means 20% ...
 *
 * @param       u32CycleResolution   Target generator duty cycle resolution. The value in general is 100.
 *
 * @return      The compatator value by new duty cycle
 */
uint32_t CalNewDutyCMR(PWM_T *pwm, uint32_t u32ChannelNum, uint32_t u32DutyCycle, uint32_t u32CycleResolution)
{
//    return (u32DutyCycle * (PWM_GET_CNR(pwm, u32ChannelNum) + 1) / u32CycleResolution);
    return (u32DutyCycle * (PWM_GET_CNR(pwm, u32ChannelNum)) / u32CycleResolution);
}

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{
    uint8_t  u8Option;
    uint32_t u32NewCMR = 0;

    /* Unlock protected registers */
//    SYS_UnlockReg();

    /* Init System, IP clock and multi-function I/O */
    SYS_Init();

    /* Lock protected registers */
    SYS_LockReg();

    /* Reset UART module */
    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 baud rate */
    UART_Open(UART0, 115200);

    printf("+-----------------------------------------------------------------------------------+\n");
    printf("|                          PWM Driver Sample Code                                   |\n");
    printf("|                                                                                   |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("  This sample code will use PWM0 channel 0 to output waveform, and switch duty cycle.\n");
    printf("  I/O configuration:\n");
    printf("    waveform output pin: PWM0 channel 0(PB.5)\n");
    printf("\nOutput waveform is 1200 Hz and it's duty is 50%.\n");

    /*
      Configure PWM0 channel 0 init period and duty(up counter type).
      Period is HIRC / (prescaler * (CNR + 1))
      Duty ratio = (CMR) / (CNR + 1)
      Period = 48 MHz / (1 * (39,999 + 1)) = 1200 Hz
      Duty ratio = (40,000) / (19,999 + 1) = 50%
    */

    /* PWM0 channel 0 frequency is 1200 Hz, duty 50%, */
    PWM_ConfigOutputChannel(PWM0, 0, 1200, 50);

    /* Enable output of PWM0 channel 0 */
    PWM_EnableOutput(PWM0, PWM_CH_0_MASK);

    /* ZeroLevel: High, CmpUpLevel: nothing, PeriodLevel: nothing, CmpDownLevel: Low */
    PWM_SET_OUTPUT_LEVEL(PWM0, PWM_CH_0_MASK, PWM_OUTPUT_NOTHING, PWM_OUTPUT_NOTHING, PWM_OUTPUT_HIGH, PWM_OUTPUT_LOW);

    /* Start PWM counter */
    PWM_Start(PWM0, PWM_CH_0_MASK);

    while(1)
    {
        printf("\nSelect new duty: \n");
        printf("[1] 100%% \n");
        printf("[2] 75%% \n");
        printf("[3] 25%% \n");
        printf("[4] 0%% \n");
        printf("[Other] Exit \n");
        u8Option = getchar();

        if(u8Option == '1')
        {
            u32NewCMR = PWM_GET_CNR(PWM0, 0) + 1;
        }
        else if(u8Option == '2')
        {
            u32NewCMR = PWM_GET_CNR(PWM0, 0);
            u32NewCMR = u32NewCMR * (100-75)/100;
        }
        else if(u8Option == '3')
        {
            u32NewCMR = PWM_GET_CNR(PWM0, 0);
            u32NewCMR = u32NewCMR * (100-25)/100;
        }
        else if(u8Option == '4')
        {
            u32NewCMR = PWM_GET_CNR(PWM0, 0);
        }
        else
        {
            printf("Exit\n");
            break;
        }
        /* Set new comparator value to register */
        PWM_SET_CMR(PWM0, 0, u32NewCMR);
    }

    /* Stop PWM counter */
    PWM_Stop(PWM0, PWM_CH_0_MASK);
    /* Disable output of PWM0 channel 0 */
    PWM_DisableOutput(PWM0, PWM_CH_0_MASK);

    while(1);

}
