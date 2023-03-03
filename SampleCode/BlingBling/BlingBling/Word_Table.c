/**************************************************************************//**
 * @file     Word_Table.c
 * @version  V1.00
 * @brief    Word table function
 *
 * @copyright (C) 2023 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "NuMicro.h"
#include "stdio.h"
#include <string.h>

#include "ARGB.h"
#include "Word_Table.h"

uint8_t showArray_row = 0;
uint16_t showArray_column= 0;
uint8_t WordTable_row;
uint8_t color = 0;
uint16_t Run_column;

uint8_t showArray[showArrayRow][showArrayColumn] = {0};

const uint8_t WordTable[77][5] = {0x7F, 0x88, 0x88, 0x88, 0x7F, //  A
                                  0xFF, 0x91, 0x91, 0x91, 0x6E, //    B
                                  0x7E, 0x81, 0x81, 0x81, 0x42, //  C
                                  0xFF, 0x81, 0x81, 0x42, 0x3C, //  D
                                  0xFF, 0x91, 0x91, 0x91, 0x81, //  E
                                  0xFF, 0x90, 0x90, 0x90, 0x80, //  F
                                  0x7E, 0x81, 0x89, 0x89, 0x4E, //  G
                                  0xFF, 0x10, 0x10, 0x10, 0xFF, //  H
                                  0x81, 0x81, 0xFF, 0x81, 0x81, //  I
                                  0x06, 0x01, 0x01, 0x01, 0xFE, //  J
                                  0xFF, 0x18, 0x24, 0x42, 0x81, //  K
                                  0xFF, 0x01, 0x01, 0x01, 0x01, //  L
                                  0xFF, 0x40, 0x30, 0x40, 0xFF, //  M
                                  0xFF, 0x40, 0x30, 0x08, 0xFF, //  N
                                  0x7E, 0x81, 0x81, 0x81, 0x7E, //  O
                                  0xFF, 0x88, 0x88, 0x88, 0x70, //  P
                                  0x7E, 0x81, 0x85, 0x82, 0x7D, //  Q
                                  0xFF, 0x88, 0x8C, 0x8A, 0x71, //  R
                                  0x61, 0x91, 0x91, 0x91, 0x8E, //  S
                                  0x80, 0x80, 0xFF, 0x80, 0x80, //  T
                                  0xFE, 0x01, 0x01, 0x01, 0xFE, //  U
                                  0xF0, 0x0C, 0x03, 0x0C, 0xF0, //  V
                                  0xFF, 0x02, 0x0C, 0x02, 0xFF, //  W
                                  0xC3, 0x24, 0x18, 0x24, 0xC3, //  X
                                  0xE0, 0x10, 0x0F, 0x10, 0xE0, //  Y
                                  0x83, 0x85, 0x99, 0xA1, 0xC1, //  Z
                                  0x06, 0x29, 0x29, 0x29, 0x1F, //  a
                                  0xFF, 0x09, 0x11, 0x11, 0x0E, //  b
                                  0x1E, 0x21, 0x21, 0x21, 0x12, //  c
                                  0x0E, 0x11, 0x11, 0x09, 0xFF, //  d
                                  0x0E, 0x15, 0x15, 0x15, 0x0C, //  e
                                  0x08, 0x7F, 0x88, 0x80, 0x40, //  f
                                  0x30, 0x49, 0x49, 0x49, 0x7E, //  g
                                  0xFF, 0x08, 0x10, 0x10, 0x0F, //  h
                                  0x00, 0x00, 0x5F, 0x00, 0x00, //  i
                                  0x02, 0x01, 0x21, 0xBE, 0x00, //  j
                                  0xFF, 0x04, 0x0A, 0x11, 0x00, //  k
                                  0x00, 0x81, 0xFF, 0x01, 0x00, //  l
                                  0x3F, 0x20, 0x18, 0x20, 0x1F, //  m
                                  0x3F, 0x10, 0x20, 0x20, 0x1F, //  n
                                  0x0E, 0x11, 0x11, 0x11, 0x0E, //  o
                                  0x3F, 0x24, 0x24, 0x24, 0x18, //  p
                                  0x10, 0x28, 0x28, 0x18, 0x3F, //  q
                                  0x1F, 0x08, 0x10, 0x10, 0x08, //  r
                                  0x09, 0x15, 0x15, 0x15, 0x02, //  s
                                  0x20, 0xFE, 0x21, 0x01, 0x02, //  t
                                  0x1E, 0x01, 0x01, 0x02, 0x1F, //  u
                                  0x1C, 0x02, 0x01, 0x02, 0x1C, //  v
                                  0x1E, 0x01, 0x0E, 0x01, 0x1E, //  w
                                  0x11, 0x0A, 0x04, 0x0A, 0x11, //  x
                                  0x00, 0x39, 0x05, 0x05, 0x3E, //  y
                                  0x11, 0x13, 0x15, 0x19, 0x11, //  z
                                  0x00, 0x41, 0xFF, 0x01, 0x00, //  1
                                  0x43, 0x85, 0x89, 0x91, 0x61, //  2
                                  0x42, 0x81, 0x91, 0x91, 0x6E, //  3
                                  0x18, 0x28, 0x48, 0xFF, 0x08, //  4
                                  0xF2, 0x91, 0x91, 0x91, 0x8E, //  5
                                  0x1E, 0x29, 0x49, 0x89, 0x86, //  6
                                  0x80, 0x8F, 0x90, 0xA0, 0xC0, //  7
                                  0x6E, 0x91, 0x91, 0x91, 0x6E, //  8
                                  0x70, 0x89, 0x89, 0x8A, 0x7C, //  9
                                  0x60, 0x80, 0x8D, 0x90, 0x60, //  ?
                                  0x00, 0x00, 0xFD, 0x00, 0x00, //  !
                                  0x7E, 0x89, 0x91, 0xA1, 0x7E, //  0
                                  0x66, 0x89, 0x8F, 0x81, 0x7E, //  @
                                  0x24, 0xFF, 0x24, 0xFF, 0x24, //  #
                                  0x76, 0x89, 0x95, 0x62, 0x05, //  &
                                  0x00, 0x3C, 0x42, 0x81, 0x00, //  (
                                  0x00, 0x81, 0x42, 0x3C, 0x00, //  )
                                  0x08, 0x08, 0x3E, 0x08, 0x08, //  "+"
                                  0x08, 0x08, 0x08, 0x08, 0x08, //  -
                                  0x14, 0x14, 0x14, 0x14, 0x14, //  =
                                  0x12, 0x2A, 0x7F, 0x2A, 0x24, //  $
                                  0x00, 0x00, 0x00, 0x00, 0x00, //  null
                                  0x01, 0x01, 0x01, 0x01, 0x01,   //  _
                                  0x00, 0x00, 0x01, 0x00, 0x00,   //  .
                                  0x00, 0x00, 0x03, 0x00, 0x00,   //  ,
                                 };


/**
  * @brief  Select Word
  * @param  None
  * @retval None
  */
void select_word(char inputChar)
{
    switch (inputChar)
    {
    case 'A':
        WordTable_row = 0;
        break;

    case 'B':
        WordTable_row = 1;
        break;

    case 'C':
        WordTable_row = 2;
        break;

    case 'D':
        WordTable_row = 3;
        break;

    case 'E':
        WordTable_row = 4;
        break;

    case 'F':
        WordTable_row = 5;
        break;

    case 'G':
        WordTable_row = 6;
        break;

    case 'H':
        WordTable_row = 7;
        break;

    case 'I':
        WordTable_row = 8;
        break;

    case 'J':
        WordTable_row = 9;
        break;

    case 'K':
        WordTable_row = 10;
        break;

    case 'L':
        WordTable_row = 11;
        break;

    case 'M':
        WordTable_row = 12;
        break;

    case 'N':
        WordTable_row = 13;
        break;

    case 'O':
        WordTable_row = 14;
        break;

    case 'P':
        WordTable_row = 15;
        break;

    case 'Q':
        WordTable_row = 16;
        break;

    case 'R':
        WordTable_row = 17;
        break;

    case 'S':
        WordTable_row = 18;
        break;

    case 'T':
        WordTable_row = 19;
        break;

    case 'U':
        WordTable_row = 20;
        break;

    case 'V':
        WordTable_row = 21;
        break;

    case 'W':
        WordTable_row = 22;
        break;

    case 'X':
        WordTable_row = 23;
        break;

    case 'Y':
        WordTable_row = 24;
        break;

    case 'Z':
        WordTable_row = 25;
        break;

    case 'a':
        WordTable_row = 26;
        break;

    case 'b':
        WordTable_row = 27;
        break;

    case 'c':
        WordTable_row = 28;
        break;

    case 'd':
        WordTable_row = 29;
        break;

    case 'e':
        WordTable_row = 30;
        break;

    case 'f':
        WordTable_row = 31;
        break;

    case 'g':
        WordTable_row = 32;
        break;

    case 'h':
        WordTable_row = 33;
        break;

    case 'i':
        WordTable_row = 34;
        break;

    case 'j':
        WordTable_row = 35;
        break;

    case 'k':
        WordTable_row = 36;
        break;

    case 'l':
        WordTable_row = 37;
        break;

    case 'm':
        WordTable_row = 38;
        break;

    case 'n':
        WordTable_row = 39;
        break;

    case 'o':
        WordTable_row = 40;
        break;

    case 'p':
        WordTable_row = 41;
        break;

    case 'q':
        WordTable_row = 42;
        break;

    case 'r':
        WordTable_row = 43;
        break;

    case 's':
        WordTable_row = 44;
        break;

    case 't':
        WordTable_row = 45;
        break;

    case 'u':
        WordTable_row = 46;
        break;

    case 'v':
        WordTable_row = 47;
        break;

    case 'w':
        WordTable_row = 48;
        break;

    case 'x':
        WordTable_row = 49;
        break;

    case 'y':
        WordTable_row = 50;
        break;

    case 'z':
        WordTable_row = 51;
        break;

    case '1':
        WordTable_row = 52;
        break;

    case '2':
        WordTable_row = 53;
        break;

    case '3':
        WordTable_row = 54;
        break;

    case '4':
        WordTable_row = 55;
        break;

    case '5':
        WordTable_row = 56;
        break;

    case '6':
        WordTable_row = 57;
        break;

    case '7':
        WordTable_row = 58;
        break;

    case '8':
        WordTable_row = 59;
        break;

    case '9':
        WordTable_row = 60;
        break;

    case '?':
        WordTable_row = 61;
        break;

    case '!':
        WordTable_row = 62;
        break;

    case '0':
        WordTable_row = 63;
        break;

    case '@':
        WordTable_row = 64;
        break;

    case '#':
        WordTable_row = 65;
        break;

    case '&':
        WordTable_row = 66;
        break;

    case '(':
        WordTable_row = 67;
        break;

    case ')':
        WordTable_row = 68;
        break;

    case '+':
        WordTable_row = 69;
        break;

    case '-':
        WordTable_row = 70;
        break;

    case '=':
        WordTable_row = 71;
        break;

    case '$':
        WordTable_row = 71;
        break;

    case 32:
        WordTable_row = 73;
        break;

    case '_':
        WordTable_row = 74;

    case '.':
        WordTable_row = 75;

    case ',':
        WordTable_row = 76;

    default:
        break;
    }
}

/**
  * @brief  Put String
  * @param  None
  * @retval None
  */
void put_string(char inputString[])
{
    int8_t count;

    for(count = 0; count < strlen(inputString); count++)
    {
        select_word(inputString[count]);

        put_char(count, strlen(inputString));
    }

    if(showArray_column >8)
        Run_column = showArray_column;
    else
        Run_column = 8;

    ARGB_showString();

    TIMER_Delay(TIMER0, 1000000);

    color++;

    if(color == 3)
    {
        color = 0;
    }

    showArray_row = 0;
    showArray_column = 0;
}

/**
  * @brief  Put Char
  * @param  None
  * @retval None
  */
void put_char(int8_t count, uint32_t Wordlen)
{
    int8_t i, j, tmpbit;

    for(i = 0; i < 5; i++)
    {
        for(j = 7; j >= 0; j--)
        {
            tmpbit = getbit(WordTable[WordTable_row][i], j);
            showArray[showArray_row++][showArray_column] = tmpbit;
        }
        showArray_row = 0;
        showArray_column++;
    }

//      if(count != (Wordlen - 1))
//      {
    for(j = 0; j < 8; j++)
    {
        showArray[showArray_row++][showArray_column] = 0;
    }
//      }
//      else
//      {
//              for(i = 0; i < 3; i++)
//              {
//                      for(j = 0; j < 8; j++)
//                      {
//                              showArray[showArray_row++][showArray_column] = 0;
//                      }
//
//                      showArray_row = 0;
//                      showArray_column++;
//              }
//      }

    showArray_row = 0;
    showArray_column++;
}
