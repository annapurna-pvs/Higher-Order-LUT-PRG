
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License version 2 as published
// by the Free Software Foundation.

#ifndef __prg3_h__
#define __prg3_h__

#include "aes.h"

void init_robprg3(int d,int n);
byte get_robprg3(int n,unsigned int count);
void set_robprgcount3(void);
int get_robprgcount3(void);
void free_robprg3(int d,int n);


void init_mprg2(byte n,int ni);
byte get_mprg_lr(int index,int n, unsigned int val);
void set_mprg_lr_count(void);
unsigned int get_mprg_lr_count(int d);
void free_mprg2(int n,int ni);


#endif
