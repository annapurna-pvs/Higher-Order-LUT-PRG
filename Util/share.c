// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License version 2 as published
// by the Free Software Foundation.
#include <stdio.h>

#include "common.h"
#include "share.h"

int pow_cust(byte base,byte exp) // Power function
{
    int i,res=1;

    for(i=0;i<exp;i++)
        res=res*base;

    return res;
}


void share_rnga(byte x,byte a[],int n) //Additive secret sharing
{
		int i;
		gen_rand(a,n-1);
		a[n-1]=x;

		for(i=0;i<n-1;i++)
			a[n-1]=a[n-1] ^ a[i];

}



void locality_refresh(byte *a,int n)
{
    byte t=a[0];
	byte b[n-1];

    gen_rand(b,n-1);
    for(int i=1;i<n;i++)
    {

        t=t^ b[i-1] ^ a[i];
        a[i]=b[i-1];
    }
    a[0]=t;
}


void locality_refresh4(byte *a,int n)
{
    byte t=a[0];
	byte b[n-1];

    gen_rand(b,n-1);
    for(int i=1;i<n;i++)
    {

        t=t ^ (b[i-1]%16) ^ a[i];
        a[i]= (b[i-1]%16);
    }
    a[0]=t;
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
    locality_refresh(a,n);
    return xorop(a,n);
}


/*********Operation for 32-bit operation******/

unsigned int gen_rand32()
{
    byte a[4];
    unsigned rand=0;
    gen_rand(a,4);

    for(int i=0;i<4;i++)
    {
        rand=rand+(a[i]<<(8*i));

    }

  return rand;

}



