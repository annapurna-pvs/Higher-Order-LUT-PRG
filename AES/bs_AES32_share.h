
#ifndef __bs_AES32_share_h__
#define __bs_AES32_share_h__
#include "../Util/common.h"

typedef unsigned char byte;

unsigned long xorshf96(void);
//void init_randcount();
//unsigned int get_randcount();

void refresh_b(byte a[shares_N]);
void share_b(byte x,byte a[shares_N]);
byte xorop1(byte a[],int n);
byte decode1(byte a[],int n);

byte pow_cus(byte base,byte exp);
void gen_r(byte *r, int l);

void refresh16(unsigned int a[],int n);
void share16(unsigned int x,unsigned int a[],int n);
unsigned int reconstruct16(unsigned int a[],int n);

#endif
