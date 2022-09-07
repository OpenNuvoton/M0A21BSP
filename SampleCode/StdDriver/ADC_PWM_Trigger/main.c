/**************************************************************************//**
 * @file     main.c
 * @version  V3.00
 * @brief    Demonstrate how to trigger ADC by PWM.
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "NuMicro.h"


/*---------------------------------------------------------------------------------------------------------*/
/* Define global variables and constants                                                                   */
/*---------------------------------------------------------------------------------------------------------*/
volatile uint32_t g_u32AdcIntFlag, g_u32COVNUMFlag = 0;


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

    /* Enable PWM0 module clock */
    CLK_EnableModuleClock(PWM0_MODULE);

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

    /* Set PD multi-function pins for PWM0 Channel 0 */
    SYS->GPD_MFP0 = (SYS->GPD_MFP0 & ~(SYS_GPD_MFP0_PD2MFP_Msk)) |
                    (SYS_GPD_MFP0_PD2MFP_PWM0_CH0);

    /* Lock protected registers */
    SYS_LockReg();
}

void PWM0_Init()
{
    /* Set PWM0 timer clock prescaler */
    PWM_SET_PRESCALER(PWM0, 0, 10);

    /* Set up counter type */
    PWM0->CTL1 &= ~PWM_CTL1_CNTTYPE0_Msk;

    /* Set PWM0 timer duty */
    PWM_SET_CMR(PWM0, 0, 1000);

    /* Set PWM0 timer period */
    PWM_SET_CNR(PWM0, 0, 2000);

    /* PWM period point trigger ADC enable */
    PWM_EnableADCTrigger(PWM0, 0, PWM_TRIGGER_ADC_EVEN_PERIOD_POINT);

    /* Set output level at zero, compare up, period(center) and compare down of specified channel */
    PWM_SET_OUTPUT_LEVEL(PWM0, BIT0, PWM_OUTPUT_HIGH, PWM_OUTPUT_LOW, PWM_OUTPUT_NOTHING, PWM_OUTPUT_NOTHING);

    /* Enable output of PWM0 channel 0 */
    PWM_EnableOutput(PWM0, BIT0);
}

void ADC_FunctionTest()
{
    uint8_t  u8Option;
    int32_t  i32ConversionData[6] = {0};

    printf("\n");
    printf("+----------------------------------------------------------------------+\n");
    printf("|                      ADC trigger by PWM test                         |\n");
    printf("+----------------------------------------------------------------------+\n");

    printf("\nIn this test, software will get 6 conversion result from the specified channel.\n");

    /* Enable ADC converter */
    ADC_POWER_ON(ADC);

    while(1)
    {
        printf("Select input mode:\n");
        printf("  [1] Single end input (channel 2 only)\n");
        printf("  [2] Differential input (channel pair 1 only)\n");
        printf("  Other keys: exit single mode test\n");
        u8Option = getchar();
        if(u8Option == '1')
        {
            /* Set input mode as single-end, Single mode, and select channel 2 */
            ADC_Open(ADC, ADC_ADCR_DIFFEN_SINGLE_END, ADC_ADCR_ADMD_SINGLE, BIT2);

            /* Configure the sample module and enable PWM0 trigger source */
            ADC_EnableHWTrigger(ADC, ADC_ADCR_TRGS_PWM, 0);

            /* Clear the A/D interrupt flag for safe */
            ADC_CLR_INT_FLAG(ADC, ADC_ADF_INT);

            /* Enable the sample module interrupt */
            ADC_ENABLE_INT(ADC, ADC_ADF_INT);  /* Enable sample module A/D interrupt. */
            NVIC_EnableIRQ(ADC_IRQn);

            printf("Conversion result of channel 2:\n");

            /* Reset the ADC indicator and enable PWM0 channel 0 counter */
            g_u32AdcIntFlag = 0;
            g_u32COVNUMFlag = 0;
            PWM_Start(PWM0, PWM_CH_0_MASK); /* PWM0 channel 0 counter start running. */

            while(1)
            {
                /* Wait ADC interrupt (g_u32AdcIntFlag will be set at IRQ_Handler function) */
                while(g_u32AdcIntFlag == 0);

                /* Reset the ADC interrupt indicator */
                g_u32AdcIntFlag = 0;

                /* Get the conversion result of the ADC channel 2 */
                i32ConversionData[g_u32COVNUMFlag - 1] = ADC_GET_CONVERSION_DATA(ADC, 2);

                if(g_u32COVNUMFlag >= 6)
                    break;
            }

            /* Disable PWM0 channel 0 counter */
            PWM_ForceStop(PWM0, BIT0);  /* PWM0 counter stop running. */

            for(g_u32COVNUMFlag = 0; (g_u32COVNUMFlag) < 6; g_u32COVNUMFlag++)
                printf("                                0x%X (%d)\n", i32ConversionData[g_u32COVNUMFlag], i32ConversionData[g_u32COVNUMFlag]);
        }
        else if(u8Option == '2')
        {
            /* Set input mode as differential, Single mode, and select channel 2 */
            ADC_Open(ADC, ADC_ADCR_DIFFEN_DIFFERENTIAL, ADC_ADCR_ADMD_SINGLE, BIT2);

            /* Configure the sample module and enable PWM0 trigger source */
            ADC_EnableHWTrigger(ADC, ADC_ADCR_TRGS_PWM, 0);

            /* Clear the A/D interrupt flag for safe */
            ADC_CLR_INT_FLAG(ADC, ADC_ADF_INT);

            /* Enable the sample module interrupt */
            ADC_ENABLE_INT(ADC, ADC_ADF_INT);  /* Enable sample module A/D interrupt. */
            NVIC_EnableIRQ(ADC_IRQn);

            printf("Conversion result of channel 2:\n");

            /* Reset the ADC indicator and enable PWM0 channel 0 counter */
            g_u32AdcIntFlag = 0;
            g_u32COVNUMFlag = 0;
            PWM_Start(PWM0, PWM_CH_0_MASK); /* PWM0 channel 0 counter start running. */

            while(1)
            {
                /* Wait ADC interrupt (g_u32AdcIntFlag will be set at IRQ_Handler function) */
                while(g_u32AdcIntFlag == 0);

                /* Reset the ADC interrupt indicator */
                g_u32AdcIntFlag = 0;

                /* Get the conversion result of the ADC channel 2 */
                i32ConversionData[g_u32COVNUMFlag - 1] = ADC_GET_CONVERSION_DATA(ADC, 2);

                if(g_u32COVNUMFlag >= 6)
                    break;
            }

            /* Disable PWM0 channel 0 counter */
            PWM_ForceStop(PWM0, BIT0);  /* PWM0 counter stop running. */

            for(g_u32COVNUMFlag = 0; (g_u32COVNUMFlag) < 6; g_u32COVNUMFlag++)
                printf("                                0x%X (%d)\n", i32ConversionData[g_u32COVNUMFlag], i32ConversionData[g_u32COVNUMFlag]);
        }
        else
            return ;
    }
}

void ADC_IRQHandler(void)
{
    ADC_CLR_INT_FLAG(ADC, ADC_ADF_INT); /* Clear the A/D interrupt flag */
    g_u32AdcIntFlag = 1;
    g_u32COVNUMFlag++;
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

    /* Init PWM for ADC */
    PWM0_Init();

    printf("\nSystem clock rate: %d Hz", SystemCoreClock);

    /* ADC function test */
    ADC_FunctionTest();

    /* Disable ADC IP clock */
    CLK_DisableModuleClock(ADC_MODULE);

    /* Disable PWM0 IP clock */
    CLK_DisableModuleClock(PWM0_MODULE);

    /* Disable External Interrupt */
    NVIC_DisableIRQ(ADC_IRQn);

    printf("Exit ADC sample code\n");

    while(1);
}
