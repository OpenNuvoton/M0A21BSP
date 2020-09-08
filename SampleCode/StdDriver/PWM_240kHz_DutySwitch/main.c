/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 5 $
 * $Date: 20/07/02 5:44p $
 * @brief
 *           Demonstrate how to set PWM0 channel 0 outputs 240 kHz waveform and
 *           swtich duty in each 0.5%.
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "NuMicro.h"


#define DEBUG_MSG printf
/*---------------------------------------------------------------------------------------------------------*/
/* Macro, type and constant definitions                                                                    */
/*---------------------------------------------------------------------------------------------------------*/
uint32_t sysGetNum(void)
{
    uint8_t cInputTemp=0x00, InputString[16]= {0};
    uint32_t nLoop = 0;
    while(cInputTemp != 0x0D)
    {
        cInputTemp = getchar();
        if(cInputTemp == 27) /* 27: ESC key */
        {
            return cInputTemp;
        }
        if(cInputTemp == 'x' || cInputTemp == 'X' || cInputTemp == 'f'||
                cInputTemp == 'F' || cInputTemp == 'r' || cInputTemp == 'R')
        {
            return cInputTemp;
        }
        if(cInputTemp == '-')
        {
            InputString[nLoop] = cInputTemp;
            DEBUG_MSG("%c",cInputTemp);
            nLoop++;
        }
        else if(cInputTemp >= '0' && cInputTemp <= '9')
        {
            InputString[nLoop] = cInputTemp;
            DEBUG_MSG("%c",cInputTemp);
            nLoop++;
        }
    }
    DEBUG_MSG("\n");
    //return atoi(InputString);
    return atoi((const char *)InputString);
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

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{
    uint8_t  u8Duty;

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
	
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("|                          PWM Driver Sample Code                                   |\n");
    printf("|                                                                                   |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("  This sample code will use PWM0 channel 0 to output 240 kHz waveform.               \n");
    printf("  And user can change duty in each 0.5 %%.                                           \n");
    printf("  I/O configuration:\n");
    printf("    waveform output pin: PWM0 channel 0(PB.5)\n");

    /*
      Configure PWM0 channel 0 init period and duty(down counter type).
      Period is HIRC / (prescaler * (CNR + 1))
      Duty ratio = CMR / (CNR + 1)
      Period = 48 MHz / (1 * (199 + 1)) = 240000 Hz
      Duty ratio = 100 / (199 + 1) = 50%
    */

    /* PWM0 channel 0 frequency prescaler to 1 */
    PWM_SET_PRESCALER(PWM0, 0, 1-1);

    /* PWM0 channel 0 frequency period to 199 */
    PWM_SET_CNR(PWM0, 0, 199);

    /* PWM0 channel 0 frequency comparator to 100 */
    PWM_SET_CMR(PWM0, 0, 100);

    /* ZeroLevel: Hihg, CmpUpLevel: Low, PeriodLevel: nothing, CmpDownLevel: nothing */
    PWM_SET_OUTPUT_LEVEL(PWM0, PWM_CH_0_MASK, PWM_OUTPUT_HIGH, PWM_OUTPUT_LOW, PWM_OUTPUT_NOTHING, PWM_OUTPUT_NOTHING);

    /* Enable output of PWM0 channel 0 */
    PWM_EnableOutput(PWM0, PWM_CH_0_MASK);

    /* Start PWM counter */
    PWM_Start(PWM0, PWM_CH_0_MASK);

    while(1)
    {
        printf("\nDuty is equal to (duty level * 0.5) %% \n");
        printf("Please enter new duty level (0 ~ 200): \n");
        u8Duty = sysGetNum();

        if(u8Duty > 200)
            /* Get invalid value */
            printf("\nEnter value out of range\n");
        else
        {
            printf("Output Duty is %d.%d %%.\n", u8Duty/2, (u8Duty%2)*5);
		    PWM_SET_CMR(PWM0, 0, u8Duty);	
        }
    }
}
