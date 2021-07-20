// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License version 2 as published
// by the Free Software Foundation.

#include "share.h"
#include "prg3.h"
/*#include "board.h"
#include "fsl_rnga.h"
#include "fsl_debug_console.h"

#include "rand_k64.h"

RNG_Type *const base =(RNG_Type *) ((char *)0+ 0x40029000u); //RNG_base register initialisation*/



static unsigned long x=123456789, y=362436069, z=521288629;
static unsigned int randcount=0;

unsigned long xorshf96(void) {
  unsigned long t;
  randcount++;

  x ^= x << 16;
  x ^= x >> 5;
  x ^= x << 1;

  t = x;
  x = y;
  y = z;
  z = t ^ x ^ y;
  return z;
}

void init_randcount()
{
  randcount=0;
}

unsigned int get_randcount()
{
  return randcount;
}

void set_randcount(unsigned int randc)
{
  randcount=randc;
}

void refresh(byte a[],int n)
{
  int i;
  for(i=1;i<n;i++)
  {
    byte tmp=xorshf96()%256;
    a[0]=a[0] ^ tmp;
    a[i]=a[i] ^ tmp;
  }
}


void share(byte x,byte a[],int n)
{
  int i;
  a[0]=x;
  for(i=1;i<n;i++)
    a[i]=0;
}

byte xorop(byte a[],int n)
{
  int i;
  byte r=0;
  for(i=0;i<n;i++)
    r^=a[i];
  return r;
}

byte decode(byte a[],int n)
{
  int i;
  for(i=0;i<n;i++) // actually this is not needed for AES
    refresh(a,n);
  return xorop(a,n);
}


/*********Operation for 32-bit operation******/

unsigned int gen_rand32()
{
    byte a[4];
    byte temp;
    unsigned rand=0;

    for(int i=0;i<4;i++)
    {
        a[i]=xorshf96()%256;
        rand=rand+(a[i]<<(8*i));

    }

  return rand;

}

void gen_rand(byte *a,int n)
{
    for(int i=0;i<n;i++)
        a[i]=xorshf96()%256;
}

/*void rand_in(){

		//RNGA initialisation code
		RNGA_Init(base);
		rnga_mode_t mode;
		status_t flag;
		mode=0U;
		RNGA_SetMode(base,mode);

}

void gen_rand(char *arr,int size){ //Populate arr with required number of random bytes using RNGA

		int j;
		status_t flag;

		for(j=0;j<size;j++){
			arr[j]=0;
			}

		flag=RNGA_GetRandomData(base,arr,size);
}

void rand_dein()
{
			RNGA_Deinit(base);
}*/
