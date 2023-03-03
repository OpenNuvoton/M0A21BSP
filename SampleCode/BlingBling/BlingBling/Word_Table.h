/**************************************************************************//**
 * @file     Word_Table.h
 * @version  V1.00
 * @brief    Word Table
 *
 * @copyright (C) 2023 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#ifndef __Word_Table_H__
#define __Word_Table_H__
#include "NuMicro.h"

#define showArrayRow            8
#define showArrayColumn     300

void put_string(char inputString[]);
void select_word(char inputChar);
void put_char(int8_t count, uint32_t Wordlen);


#endif  /* __Word_Table_H__ */

/*** (C) COPYRIGHT 2023 Nuvoton Technology Corp. ***/
