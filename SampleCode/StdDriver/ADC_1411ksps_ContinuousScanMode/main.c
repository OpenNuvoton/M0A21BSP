/**************************************************************************//**
 * @file     main.c
 * @version  V3.00
 * @brief    Demonstrate how to use HIRC as ADC clock source to achieve 1411 ksps ADC conversion rate.
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "NuMicro.h"


/*---------------------------------------------------------------------------------------------------------*/
/* Define global variables and constants                                                                   */
/*---------------------------------------------------------------------------------------------------------*/
volatile uint32_t g_u32AdcIntFlag;


void SYS_Init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Enable HIRC clock (Internal RC 48 MHz) */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);

    /* Wait for HIRC clock ready */
    CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);

    /* Select HCLK clock source as HIRC and HCLK source divider as 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));

    /* Enable UART module clock */
    CLK_EnableModuleClock(UART0_MODULE);

    /* Switch UART0 clock source to HIRC */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));

    /* Enable ADC module clock */
    CLK_EnableModuleClock(ADC_MODULE);

    /* ADC clock source is HCLK = PCLK1 = 48MHz, set divider to 2, ADC clock is 48/2 MHz */
    CLK_SetModuleClock(ADC_MODULE, CLK_CLKSEL2_ADCSEL_PCLK1, CLK_CLKDIV0_ADC(2));

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate PllClock, SystemCoreClock and CycylesPerUs automatically. */
    SystemCoreClockUpdate();

    /*----------------------------------------------------------------------*/
    /* Init I/O Multi-function                                              */
    /*----------------------------------------------------------------------*/
    /* Set PB multi-function pins for UART0 RXD=PB.6 and TXD=PB.4 */
    SYS->GPB_MFP1 = (SYS->GPB_MFP1 & ~(SYS_GPB_MFP1_PB4MFP_Msk | SYS_GPB_MFP1_PB6MFP_Msk)) |
                    (SYS_GPB_MFP1_PB4MFP_UART0_TXD | SYS_GPB_MFP1_PB6MFP_UART0_RXD);

    /* Set PA.2, PC.0 ~ PC.2 to input mode */
    GPIO_SetMode(PA, BIT2, GPIO_MODE_INPUT);
    GPIO_SetMode(PC, BIT0|BIT1|BIT2, GPIO_MODE_INPUT);

    /* Configure the PA.2, PC.0 ~ PC.2 ADC analog input pins.  */
    SYS->GPA_MFP0 = (SYS->GPA_MFP0 & ~(SYS_GPA_MFP0_PA2MFP_Msk)) |
                    (SYS_GPA_MFP0_PA2MFP_ADC0_CH2);
    SYS->GPC_MFP0 = (SYS->GPC_MFP0 & ~(SYS_GPC_MFP0_PC0MFP_Msk | SYS_GPC_MFP0_PC1MFP_Msk |
                                       SYS_GPC_MFP0_PC2MFP_Msk)) |
                    (SYS_GPC_MFP0_PC0MFP_ADC0_CH3 | SYS_GPC_MFP0_PC1MFP_ADC0_CH4 |
                     SYS_GPC_MFP0_PC2MFP_ADC0_CH5);

    /* Disable the PA.2, PC.0 ~ PC.2 digital input path to avoid the leakage current. */
    GPIO_DISABLE_DIGITAL_PATH(PA, BIT2);
    GPIO_DISABLE_DIGITAL_PATH(PC, BIT0|BIT1|BIT2);

    /* Lock protected registers */
    SYS_LockReg();
}

void ADC_FunctionTest()
{
    uint32_t u32ChannelCount;
    int32_t  i32ConversionData;

    printf("\n");
    printf("+----------------------------------------------------------------------+\n");
    printf("|           ADC 1411 ksps continuous scan mode sample code             |\n");
    printf("+----------------------------------------------------------------------+\n\n");

    printf("+----------------------------------------------------------------------+\n");
    printf("|   ADC clock source -> PCLK1  = 48 MHz                                |\n");
    printf("|   ADC clock divider          = 2                                     |\n");
    printf("|   ADC clock                  = 48 MHz / 2 = 24 MHz                   |\n");
    printf("|   ADC extended sampling time = 0                                     |\n");
    printf("|   ADC conversion time = 17 + ADC extended sampling time = 17         |\n");
    printf("|   ADC conversion rate = 24 MHz / 17 = 1411 ksps                      |\n");
    printf("+----------------------------------------------------------------------+\n");

    /* Enable ADC converter */
    ADC_POWER_ON(ADC);

    while(1)
    {
        printf(" Press any key to start the continuous scan mode test\n");
        getchar();
        /* Set the ADC operation mode as continuous scan, input mode as single-end and
             enable the analog input channel 2, 3, 4, and 5 */
        ADC_Open(ADC, ADC_ADCR_DIFFEN_SINGLE_END, ADC_ADCR_ADMD_CONTINUOUS, BIT2|BIT3|BIT4|BIT5);

        /* Clear the A/D interrupt flag for safe */
        ADC_CLR_INT_FLAG(ADC, ADC_ADF_INT);

        /* Enable the sample module interrupt */
        ADC_ENABLE_INT(ADC, ADC_ADF_INT);  // Enable sample module A/D interrupt.
        NVIC_EnableIRQ(ADC_IRQn);

        /* Reset the ADC interrupt indicator and trigger sample module 0 to start A/D conversion */
        g_u32AdcIntFlag = 0;
        ADC_START_CONV(ADC);

        /* Wait ADC interrupt (g_u32AdcIntFlag will be set at IRQ_Handler function) */
        while(g_u32AdcIntFlag == 0);

        /* Get the conversion result */
        for(u32ChannelCount = 2; u32ChannelCount <= 5; u32ChannelCount++)
        {
            i32ConversionData = ADC_GET_CONVERSION_DATA(ADC, u32ChannelCount);
            printf("Conversion result of channel %d: 0x%X (%d)\n", u32ChannelCount, i32ConversionData, i32ConversionData);
        }

        printf("\n");

        /* Stop A/D conversion */
        ADC_STOP_CONV(ADC);

        /* Disable the sample module interrupt */
        ADC_DISABLE_INT(ADC, ADC_ADF_INT);
    }
}


void ADC_IRQHandler(void)
{
    g_u32AdcIntFlag = 1;
    ADC_CLR_INT_FLAG(ADC, ADC_ADF_INT); /* Clear the A/D interrupt flag */
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

int32_t main(void)
{
    /* Init System, IP clock and multi-function I/O. */
    SYS_Init();

    /* Init UART0 for printf */
    UART0_Init();

    printf("\nSystem clock rate: %d Hz", SystemCoreClock);

    /* ADC function test */
    ADC_FunctionTest();

    /* Disable ADC IP clock */
    CLK_DisableModuleClock(ADC_MODULE);

    /* Disable External Interrupt */
    NVIC_DisableIRQ(ADC_IRQn);

    printf("Exit ADC sample code\n");

    while(1);
}
