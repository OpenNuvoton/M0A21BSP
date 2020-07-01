/****************************************************************************//**
 * @file     main.c
 * @version  V0.10
 * @brief    Demonstrate how to use FMC CRC32 ISP command to calculate the CRC32 checksum of APROM and LDROM.
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include "NuMicro.h"

#define FMC_MIN_LDROM_SIZE  0x800

void SYS_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Enable HIRC clock */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);

    /* Wait for HIRC clock ready */
    CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);

    /* Select HCLK clock source as HIRC and and HCLK source divider as 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));

    /* Enable UART module clock */
    CLK_EnableModuleClock(UART0_MODULE);

    /* Select UART module clock source as HIRC and UART module clock divider as 1 */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));

    /* Set PB multi-function pins for UART0 RXD=PB.6 and TXD=PB.4 */
    SYS->GPB_MFP1 = (SYS->GPB_MFP1 & ~(SYS_GPB_MFP1_PB4MFP_Msk | SYS_GPB_MFP1_PB6MFP_Msk)) |        \
                    (SYS_GPB_MFP1_PB4MFP_UART0_TXD | SYS_GPB_MFP1_PB6MFP_UART0_RXD);
}

int32_t main(void)
{
    uint32_t    u32Data, u32ChkSum;    /* temporary data */

    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Init System, IP clock and multi-function I/O. */
    SYS_Init();

    /* Lock protected registers */
    SYS_LockReg();

    /* Configure UART0: 115200, 8-bit word, no parity bit, 1 stop bit. */
    UART_Open(UART0, 115200);

    /*---------------------------------------------------------------------------------------------------------*/
    /* SAMPLE CODE                                                                                             */
    /*---------------------------------------------------------------------------------------------------------*/
    printf("\n");
    printf("+------------------------------------+\n");
    printf("|   M0A21 FMC CRC32 Sample Demo      |\n");
    printf("+------------------------------------+\n");

    SYS_UnlockReg();                   /* Unlock protected registers */

    /* Enable FMC ISP function */

    FMC_Open();

    /* FMC_ReadCID */
    u32Data = FMC_ReadCID();
    printf("  Company ID ............................ [0x%08X]\n", u32Data);

    /* FMC_ReadPID */
    u32Data = FMC_ReadPID();
    printf("  Product ID ............................ [0x%08X]\n", u32Data);

    /* Read User Configuration CONFIG0 */
    printf("  User Config 0 ......................... [0x%08X]\n", FMC_Read(FMC_CONFIG_BASE));
    /* Read User Configuration CONFIG1 */
    printf("  User Config 1 ......................... [0x%08X]\n", FMC_Read(FMC_CONFIG_BASE + 4));

    printf("\nLDROM (0x100000 ~ 0x%x) CRC32 checksum =>  ", (FMC_LDROM_BASE + FMC_MIN_LDROM_SIZE));

    /* Erase the first page of LDROM */
    FMC_ENABLE_LD_UPDATE();

    FMC_Erase(0x100000);

    /* Write one word on LD */
    FMC_Write(0x100000, 0x55AABBCC);

    /*
     *  Request FMC hardware to run CRC32 calculation on LDROM.
     */
    u32ChkSum = FMC_GetChkSum(FMC_LDROM_BASE, FMC_MIN_LDROM_SIZE);
    if (u32ChkSum == 0xFFFFFFFF)
    {
        printf("Failed on calculating LDROM CRC32 checksum!\n");
        goto lexit;                    /* failed */
    }
    printf("0x%X\n", u32ChkSum);       /* print out LDROM CRC32 check sum value */

    printf("\nAPROM (0x000000 ~ 0x004000) CRC32 checksum =>  ");
    /*
     *  Request FMC hardware to run CRC32 calculation on APROM address 0x000000 ~ 0x004000.
     *  Note that FMC CRC32 checksum calculation area must page alignment.
     */
    u32ChkSum = FMC_GetChkSum(FMC_APROM_BASE, 0x4000);
    if (u32ChkSum == 0xFFFFFFFF)
    {
        printf("Failed on calculating APROM bank0 CRC32 checksum!\n");
        goto lexit;
    }
    printf("0x%X\n", u32ChkSum);       /* print out APROM CRC32 check sum value */

    printf("\nAPROM (0x004000 ~ 0x008000) CRC32 checksum =>  ");
    /*
     *  Request FMC hardware to run CRC32 calculation on APROM address 0x004000 ~ 0x008000.
     *  Note that FMC CRC32 checksum calculation area must page alignment.
     */
    u32ChkSum = FMC_GetChkSum(FMC_APROM_BASE + 0x4000, 0x4000);
    if (u32ChkSum == 0xFFFFFFFF)
    {
        printf("Failed on calculating APROM CRC32 checksum!\n");
        goto lexit;
    }
    printf("0x%X\n", u32ChkSum);       /* print out APROM CRC32 check sum value */

    printf("\nFMC CRC32 checksum test done.\n");
lexit:
    FMC_Close();                       /* Disable FMC ISP function */
    SYS_LockReg();                     /* Lock protected registers */

    while (1);
}
/*** (C) COPYRIGHT 2020 Nuvoton Technology Corp. ***/


