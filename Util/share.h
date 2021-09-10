// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License version 2 as published
// by the Free Software Foundation.

//Author Annapurna Valiveti.

#ifndef __share_h__
#define __share_h__

#include "common.h"
//#include "board.h"
//#include "fsl_rnga.h"

int pow_cust(byte base,byte exp);

void locality_refresh(byte *a,int n);
void share(byte x,byte a[],int n);
byte xorop(byte a[],int n);
byte decode(byte a[],int n);

void share_rnga(byte x,byte a[],int n);

void locality_refresh4(byte *a,int n);

#endif
