/*****************************************************************************
 *   dactest.c:  main C entry file for NXP LPC17xx Family Microprocessors
 *
 *   Copyright(C) 2009, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2009.05.25  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/
#include <lpc17xx.h>
#include "type.h"
#include "dac.h"

/*****************************************************************************
**   Main Function  main()
******************************************************************************/
int main (void)
{
  uint32_t i = 0, m;
  SystemInit();
  /* Initialize DAC  */
  DACInit();
  while ( 1 )
  {
        DAC -> DACR = (i << 6) | DAC_BIAS;
        i++;
        for(m = 1000; m > 1; m--);
        if ( i == 1024 )
        {
          i = 0;
        }
  }
}

/*****************************************************************************
**                            End Of File
*****************************************************************************/

