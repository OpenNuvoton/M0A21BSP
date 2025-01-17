/**************************************************************************//**
 * @file     main.c
 * @version  V3.00
 * @brief    Perform A/D Conversion with ADC burst mode.
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "NuMicro.h"


/*---------------------------------------------------------------------------------------------------------*/
/* Define global variables and constants                                                                   */
/*---------------------------------------------------------------------------------------------------------*/
#define CONV_TOTAL_COUNT    20


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

    /* Enable ADC module clock */
    CLK_EnableModuleClock(ADC_MODULE);

    /* ADC clock source is HIRC, set divider to 3 */
    CLK_SetModuleClock(ADC_MODULE, CLK_CLKSEL2_ADCSEL_HIRC, CLK_CLKDIV0_ADC(3));

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate PllClock, SystemCoreClock and CycylesPerUs automatically. */
    SystemCoreClockUpdate();

    /*----------------------------------------------------------------------*/
    /* Init I/O Multi-function                                              */
    /*----------------------------------------------------------------------*/
    /* Set PB multi-function pins for UART0 RXD=PB.6 and TXD=PB.4 */
    SYS->GPB_MFP1 = (SYS->GPB_MFP1 & ~(SYS_GPB_MFP1_PB4MFP_Msk | SYS_GPB_MFP1_PB6MFP_Msk)) |
                    (SYS_GPB_MFP1_PB4MFP_UART0_TXD | SYS_GPB_MFP1_PB6MFP_UART0_RXD);


    /* Set PA.2, PC.0 to input mode */
    GPIO_SetMode(PA, BIT2, GPIO_MODE_INPUT);
    GPIO_SetMode(PC, BIT0, GPIO_MODE_INPUT);

    /* Configure the PA.2, PC.0 ADC analog input pins.  */
    SYS->GPA_MFP0 = (SYS->GPA_MFP0 & ~(SYS_GPA_MFP0_PA2MFP_Msk)) |
                    (SYS_GPA_MFP0_PA2MFP_ADC0_CH2);
    SYS->GPC_MFP0 = (SYS->GPC_MFP0 & ~(SYS_GPC_MFP0_PC0MFP_Msk)) |
                    (SYS_GPC_MFP0_PC0MFP_ADC0_CH3);

    /* Disable the PA.2, PC.0 digital input path to avoid the leakage current. */
    GPIO_DISABLE_DIGITAL_PATH(PA, BIT2);
    GPIO_DISABLE_DIGITAL_PATH(PC, BIT0);

    /* Lock protected registers */
    SYS_LockReg();
}

void ADC_FunctionTest()
{
    uint8_t  u8Option;
    uint32_t u32ConvCount;
    int32_t  i32ConversionData[CONV_TOTAL_COUNT];

    printf("\n");
    printf("+----------------------------------------------------------------------+\n");
    printf("|                     ADC burst mode sample code                       |\n");
    printf("+----------------------------------------------------------------------+\n");

    for(u32ConvCount = 0; u32ConvCount < CONV_TOTAL_COUNT; u32ConvCount++)
    {
        i32ConversionData[u32ConvCount] = 0;
    }

    /* Enable ADC converter */
    ADC_POWER_ON(ADC);

    while(1)
    {
        printf("Select input mode:\n");
        printf("  [1] Single end input (channel 2 only)\n");
        printf("  [2] Differential input (channel pair 1 only)\n");
        printf("  Other keys: exit burst mode test\n");
        u8Option = getchar();

        if(u8Option == '1')
        {
            /* Set input mode as single-end, burst mode, and select channel 2 */
            ADC_Open(ADC, ADC_ADCR_DIFFEN_SINGLE_END, ADC_ADCR_ADMD_BURST, BIT2);

            /* Clear the A/D interrupt flag for safe */
            ADC_CLR_INT_FLAG(ADC, ADC_ADF_INT);

            /* Reset the ADC interrupt indicator and trigger sample module to start A/D conversion */
            u32ConvCount = 0;
            ADC_START_CONV(ADC);

            while(1)
            {
                /* Wait ADC conversion completed */
                while (ADC_GET_INT_FLAG(ADC, ADC_ADF_INT)==0);
                ADC_CLR_INT_FLAG(ADC, ADC_ADF_INT); /* clear ADF interrupt flag */

                /* Get the conversion result until VALIDF turns to 0 */
                while(ADC->ADSR0 & ADC_ADSR0_VALIDF_Msk)
                {
                    /* Get the conversion result from ADC channel 0 always */
                    i32ConversionData[u32ConvCount++] = ADC_GET_CONVERSION_DATA(ADC, 0);
                    if(u32ConvCount == CONV_TOTAL_COUNT)
                        break;
                }

                if(u32ConvCount == CONV_TOTAL_COUNT)
                    break;
            }

            /* Stop A/D conversion */
            ADC_STOP_CONV(ADC);

            /* Show the conversion result */
            for(u32ConvCount = 0; u32ConvCount < CONV_TOTAL_COUNT; u32ConvCount++)
            {
                printf("Conversion result of channel 2 [#%d]: 0x%X (%d)\n", u32ConvCount+1, i32ConversionData[u32ConvCount], i32ConversionData[u32ConvCount]);
            }

            /* Clear remaining data in FIFO that got before stop ADC */
            while (ADC_IS_DATA_VALID(ADC, 0))
            {
                i32ConversionData[0] = ADC_GET_CONVERSION_DATA(ADC, 0);
            }
        }
        else if(u8Option == '2')
        {
            /* Set input mode as differential, burst mode, and select channel 2 */
            ADC_Open(ADC, ADC_ADCR_DIFFEN_DIFFERENTIAL, ADC_ADCR_ADMD_BURST, BIT2);

            /* Clear the A/D interrupt flag for safe */
            ADC_CLR_INT_FLAG(ADC, ADC_ADF_INT);

            /* Reset the ADC interrupt indicator and trigger sample module to start A/D conversion */
            u32ConvCount = 0;
            ADC_START_CONV(ADC);

            while(1)
            {
                /* Wait ADC conversion completed */
                while (ADC_GET_INT_FLAG(ADC, ADC_ADF_INT)==0);
                ADC_CLR_INT_FLAG(ADC, ADC_ADF_INT); /* clear ADF interrupt flag */

                /* Get the conversion result until VALIDF turns to 0 */
                while(ADC->ADSR0 & ADC_ADSR0_VALIDF_Msk)
                {
                    /* Get the conversion result from ADC channel 0 always */
                    i32ConversionData[u32ConvCount++] = ADC_GET_CONVERSION_DATA(ADC, 0);
                    if(u32ConvCount == CONV_TOTAL_COUNT)
                        break;
                }

                if(u32ConvCount == CONV_TOTAL_COUNT)
                    break;
            }

            /* Stop A/D conversion */
            ADC_STOP_CONV(ADC);

            /* Show the conversion result */
            for(u32ConvCount = 0; u32ConvCount < CONV_TOTAL_COUNT; u32ConvCount++)
            {
                printf("Conversion result of channel pair 1 [#%d]: 0x%X (%d)\n", u32ConvCount+1, i32ConversionData[u32ConvCount], i32ConversionData[u32ConvCount]);
            }

            /* Clear remaining data in FIFO that got before stop ADC */
            while (ADC_IS_DATA_VALID(ADC, 0))
            {
                i32ConversionData[0] = ADC_GET_CONVERSION_DATA(ADC, 0);
            }
        }
        else
            return;

        printf("\n");
    }
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

    printf("Exit ADC sample code\n");

    while(1);
}
