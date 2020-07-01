/****************************************************************************
 * @file     main.c
 * @version  V3.0
 * $Revision: 4 $
 * $Date: 20/06/30 8:04p $
 * @brief    Demonstrate how to trigger DAC conversion by software method.
 * @note
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "stdio.h"
#include "NuMicro.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Define global variables and constants                                                                   */
/*---------------------------------------------------------------------------------------------------------*/
static uint16_t sine[] = {2047, 2251, 2453, 2651, 2844, 3028, 3202, 3365, 3515, 3650, 3769, 3871, 3954,
                          4019, 4064, 4088, 4095, 4076, 4040, 3984, 3908, 3813, 3701, 3573, 3429, 3272,
                          3102, 2921, 2732, 2536, 2335, 2132, 1927, 1724, 1523, 1328, 1141,  962,  794,
                          639,  497,  371,  262,  171,   99,   45,   12,    0,    7,   35,   84,  151,
                          238,  343,  465,  602,  754,  919, 1095, 1281, 1475, 1674, 1876
                         };

static uint32_t index = 0;
const uint32_t array_size = sizeof(sine) / sizeof(uint16_t);

/*---------------------------------------------------------------------------------------------------------*/
/* Define functions prototype                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void);
void DAC_FunctionTest(void);


void SYS_Init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Enable HIRC clock (Internal RC 22.1184MHz) */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);

    /* Wait for HIRC clock ready */
    CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);

    /* Select HCLK clock source as HIRC and and HCLK source divider as 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));

    /* Select HIRC as the clock source of UART0 */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));

    /* Enable UART peripheral clock */
    CLK_EnableModuleClock(UART0_MODULE);

    /* Enable UART module clock */
    CLK_EnableModuleClock(UART0_MODULE);

    /* Enable DAC module clock */
    CLK_EnableModuleClock(DAC0_MODULE);

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set PB multi-function pins for UART0 RXD=PB.6 and TXD=PB.4 */
    SYS->GPB_MFP1 = (SYS->GPB_MFP1 & ~(SYS_GPB_MFP1_PB4MFP_Msk | SYS_GPB_MFP1_PB6MFP_Msk)) |        \
                    (SYS_GPB_MFP1_PB4MFP_UART0_TXD | SYS_GPB_MFP1_PB6MFP_UART0_RXD);

}

void UART0_Init()
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init UART                                                                                               */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Reset UART module */
    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 baud rate */
    UART_Open(UART0, 115200);
}

/*---------------------------------------------------------------------------------------------------------*/
/* DAC function test                                                                                       */
/*---------------------------------------------------------------------------------------------------------*/
void DACFunctionTest(void)
{
    uint32_t ii;

    printf("\n");
    printf("+----------------------------------------------------------------------+\n");
    printf("|                      DAC software trigger test                       |\n");
    printf("+----------------------------------------------------------------------+\n");

    printf("\n\nPlease hit any key to start DAC output\n");
    getchar();

    /* modify the test pattern to 5-bit width */
    for(ii=0; ii<sizeof(sine)/2; ii++)
    {
        sine[ii] = (sine[ii]>>7)&0x1F;
    }

    /* Set the software trigger, enable DAC even trigger mode and enable D/A converter */
    DAC_Open(DAC, 0, DAC_SOFTWARE_TRIGGER);

    printf("\n\nPlease hit any key to start DAC output\n");

    /* The DAC conversion settling time is 8us */
    DAC_SetDelayTime(DAC, 8);

    /* Enable DAC pin output */
    DAC_ENABLE_OUTPUT(DAC);

    /* Select reference voltage from Vreff */
    DAC_VREFFSRC_AVDD(DAC);

    /* Set DAC 5-bit holding data */
    DAC_WRITE_DATA(DAC, 0, sine[index]);

    /* Clear the DAC conversion complete finish flag for safe */
    DAC_CLR_INT_FLAG(DAC, 0);

    /* Enable the DAC interrupt */
    DAC_ENABLE_INT(DAC, 0);
    NVIC_EnableIRQ(DAC0_IRQn);

    /* Start A/D conversion */
    DAC_START_CONV(DAC);

    printf("\nHit any key to quit!\n");

    while(1)
    {
        /* Start A/D conversion */
        DAC_START_CONV(DAC);

        if((DEBUG_PORT->FIFOSTS & UART_FIFOSTS_RXEMPTY_Msk) != 0)
            continue;
        else
            break;
    }

    return;
}

/*---------------------------------------------------------------------------------------------------------*/
/* DAC interrupt handler                                                                                   */
/*---------------------------------------------------------------------------------------------------------*/
void DAC0_IRQHandler(void)
{
    if(DAC_GET_INT_FLAG(DAC, 0))
    {

        if(index == array_size)
            index = 0;
        else
        {
            DAC_WRITE_DATA(DAC, 0, sine[index++]);

            /* Clear the DAC conversion complete finish flag */
            DAC_CLR_INT_FLAG(DAC, 0);

        }
    }
    return;
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

    /* Init UART0 for printf */
    UART0_Init();

    /*---------------------------------------------------------------------------------------------------------*/
    /* SAMPLE CODE                                                                                             */
    /*---------------------------------------------------------------------------------------------------------*/

    printf("\nSystem clock rate: %d Hz\n", SystemCoreClock);

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

    /* Single Mode test */
    DACFunctionTest();

    /* Reset DAC module */
    SYS_ResetModule(DAC0_RST);

    /* Disable DAC IP clock */
    CLK_DisableModuleClock(DAC0_MODULE);

    /* Disable External Interrupt */
    NVIC_DisableIRQ(DAC0_IRQn);

    printf("Exit DAC sample code\n");

    while(1);

}

