/******************************************************************************
 * @file     ARGB.h
 * @brief    Nuvoton ARGB basic setting header file
 *
 * @note
 * Copyright (C) 2023 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#ifndef __ARGB_H__
#define __ARGB_H__
#include "NuMicro.h"
#include "stdio.h"

//=============================================== Setting Block =============================================================//
//Basic Setting:
#define Dout_Port                       PD                                  //GPIO port of the using Digital output
#define Dout_PinMask                    BIT5                                //GPIO pin mask of the using Digital output
#define Dout_Pin                        PD5                                 //Digital output pin
#define Dout_IRQn                       GPD_IRQn                            //GPIO Interrupt
#define Dout_IRQn_Pin                   5                                   //GPIO pin Interrupt

#define Strip_NUM                       1                                  //ARGB2 strip's number
#define PIXEL_NUM                       64                                  //LED's number in ARGB2 strip
#define GRB                             24                                  //Total bits of RGB
//===========================================================================================================================//

//================================================ Don't remove =============================================================//
//bitwise operation
#define setbit(x, bit)                  x |= (1 << bit)                     //set bit
#define clrbit(x, bit)                  x &= ~(1 << bit)                    //reset bit
#define getbit(x, bit)                  ((x & (1 << bit)) >> bit)           //get bit

#define BIT1_HIGH                       1                                   //Denote BIT1
#define BIT0_LOW                        0                                   //Denote BIT0

void ARGB_init(void);
void ARGB_sendBIT0(void);
void ARGB_sendBIT1(void);
void ARGB_sendReset(void);
void ARGB_closeAll(void);
void ARGB_sendOne(int);
void ARGB_showString(void);

void getData_tmpArray(int16_t scanCount);
void outputData_LED(void);

#endif  /* __ARGB_H__ */

/*** (C) COPYRIGHT 2023 Nuvoton Technology Corp. ***/
