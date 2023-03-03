/**************************************************************************//**
 * @file     ARGB.c
 * @version  V1.00
 * @brief    All ARGB2 strip function
 *
 * @copyright (C) 2023 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "NuMicro.h"
#include "stdio.h"
#include <string.h>

#include "ARGB.h"
#include "Word_Table.h"


uint8_t pixelBuffer[PIXEL_NUM][GRB];
uint8_t tmpArray[8][8];

extern uint8_t receiveData;

extern uint8_t Run_column;
extern uint8_t color;
extern uint8_t showArray[showArrayRow][showArrayColumn];

/**
  * @brief  initialize ARGB GPIO
  * @param  None
  * @retval None
  */
void ARGB_init()
{
    GPIO_SetMode(Dout_Port, Dout_PinMask, GPIO_MODE_OUTPUT);
    Dout_Pin = 0;
}

/**
  * @brief  Send BIT0
  * @param  None
  * @retval None
  */
void ARGB_sendBIT0()
{
    Dout_Pin = 1;
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    Dout_Pin = 0;
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
}

/**
  * @brief  Send BIT1
  * @param  None
  * @retval None
  */
void ARGB_sendBIT1()
{
    Dout_Pin = 1;
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    Dout_Pin = 0;

}

/**
  * @brief  Send Trst
  * @param  None
  * @retval None
  */
void ARGB_sendReset()
{
    int i = 0;
    Dout_Pin = 0;

    for (i = 0; i < 3200; i++)
    {
        __NOP();
    }
}


/**
  * @brief  Send one data to a LED
  * @param  24 bits data (Range: 0x000000 to 0xffffff)
  * @retval None
  */
void ARGB_sendOne(int dat)
{
    uint8_t i;
    unsigned char bit;

    for (i = 24; i > 0; i--)
    {
        bit = ((dat >> (i - 1)) & 0x01);

        if (bit == BIT1_HIGH)
        {
            ARGB_sendBIT1();
        }
        else
        {
            ARGB_sendBIT0();
        }
    }
}

/**
  * @brief  Show string
  * @param  None
  * @retval None
  */
void ARGB_showString()
{
    int16_t scanCount = 0;

    while(scanCount + 8 <= Run_column)
    {
        if(receiveData == TRUE)
            break;

        getData_tmpArray(scanCount);

        outputData_LED();

        TIMER_Delay(TIMER0, 150000);

        scanCount++;
    }
}

/**
  * @brief  Get data to tmpArray that use to show 8x8 data
  * @param  scanCount
  * @retval None
  */
void getData_tmpArray(int16_t scanCount)
{
    uint8_t i, j;

    if(Run_column == 8)             //process 1 char
    {
        for(i = 0; i < 8; i++)
        {
            for(j = 0; j < 5; j++)
            {
                tmpArray[i][j] = showArray[i][scanCount + j];
            }
        }

        for(i = 0; i < 8; i++)
        {
            for(j = 5; j < 8; j++)
            {
                tmpArray[i][j] = 0;
            }
        }
    }
    else
    {
        for(i = 0; i < 8; i++)
        {
            for(j = 0; j < 8; j++)
            {
                tmpArray[i][j] = showArray[i][scanCount + j];
            }
        }
    }
}

/**
  * @brief  Output data to every LED
  * @param  None
  * @retval None
  */
void outputData_LED()
{
    int8_t i, j;

    for(i = 0; i < 8; i++)
    {
        if(i % 2 == 0)
        {
            for(j = 7; j >=0; j--)
            {
                if(tmpArray[i][j] == 1)
                {
                    if(color == 0)
                    {
                        ARGB_sendOne(0x0A0000);
                    }
                    else if(color == 1)
                    {
                        ARGB_sendOne(0x000A00);
                    }
                    else
                    {
                        ARGB_sendOne(0x00000A);
                    }

                }
                else
                {
                    ARGB_sendOne(0x000000);
                }
            }
        }
        else
        {
            for(j = 0; j < 8; j++)
            {
                if(tmpArray[i][j] == 1)
                {
                    if(color == 0)
                    {
                        ARGB_sendOne(0x0A0000);
                    }
                    else if(color == 1)
                    {
                        ARGB_sendOne(0x000A00);
                    }
                    else
                    {
                        ARGB_sendOne(0x00000A);
                    }
                }
                else
                {
                    ARGB_sendOne(0x000000);
                }
            }
        }
    }
}
