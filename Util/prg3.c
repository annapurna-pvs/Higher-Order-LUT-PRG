
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License version 2 as published
// by the Free Software Foundation.
#include <stdio.h>

#include "share.h"
#include "common.h"
#include "prg3.h"

#include "../AES/aes_rp.h"


#define num_R 2*(shares_N-1)*(shares_N-1) //Value of parameter r for robust PRG

#if variant==NPRG
#define num_M (shares_N-1)*(shares_N-1)  //Value of r for multi normal PRG
#else
#define num_M (shares_N)*(shares_N-1)/2  //Value of r for multi increasing PRG
#endif // variant


/**********robust PRG for AES htable*************/

void mult_gf24(byte *a,byte *b,byte *c,int n)  // irreducible polynomial of x^3+x+1
{
    byte mul[9];
    int i,j;

    for(i=0;i<3;i++)
        for(j=0;j<3;j++)
            mul[3*i+j] = multtable(a[i],b[j]);

    for(i=2;i<8;i=i+2)
        c[0]=c[0]^mul[i];
    c[1] = mul[8];

    for(i=1;i<8;i=i+2)
        c[1]=c[1]^mul[i];

    c[2]=mul[0]^mul[5]^mul[7];

}


void eval_poly_gf24(byte *pol,int d,byte *x,byte *r,int n)//d is (n-1)*loc
{
      int i,j;

      for(i=0;i<3;i++)
        r[i]=0;

      for(i=d-1;i>=0;i--)
      {
        byte t[3];

        for(j=0;j<3;j++)
        t[j]=0;

        mult_gf24(r,x,t,n);

        for(j=0;j<3;j++)
            r[j] = t[j] ^pol[3*i+j];

      }
}


typedef struct
{
  unsigned int prgcount;
  int dprg;
  byte pol[3*num_R];
  int prgflag;
  byte r[3];
  byte x[3];

} tprg3;

tprg3 prg;

typedef struct
{
  tprg3 vprg[shares_N];
} trobprg3;

trobprg3 robprg;

void init_prg_p3(tprg3 *prg,int d)
{
    prg->dprg=d;
  //prg->pol[3*R];//=(byte *) malloc(3*R*sizeof(byte));
	byte a[3*num_R];
	gen_rand(a,3*num_R);
    prg->prgcount=0;
    for(int i=0;i<(3*num_R);i++)
        prg->pol[i]=a[i];
    prg->prgflag=0;
}


void init_robprg3(int d,int n)
{
  //robprg.vprg=malloc(N2*sizeof(tprg3));
  for(int i=0;i<n;i++)
    init_prg_p3(&robprg.vprg[i],d);
}

byte get_prg_p3(tprg3 *prg,unsigned int count)
{
   byte x1[3];

   unsigned int temp1=count/3;
   x1[0] = temp1 & 0xff;
   x1[1] = (temp1 >> 8)&0xff;
   x1[2] = (temp1 >> 16)&0xff;

   if ((count%3!=0) && (prg->x[0]==x1[0])&& (prg->x[1]==x1[1])&& (prg->x[2]==x1[2]))
   {
             return prg->r[count%3];
   }

  unsigned int temp=count/3;
  prg->x[0] = temp & 0xff;
  prg->x[1] = (temp >> 8)&0xff;
  prg->x[2] = (temp >> 16)&0xff;

  eval_poly_gf24(prg->pol,prg->dprg,prg->x,prg->r,3);
  return prg->r[count%3];
}


byte get_robprg3(int n,unsigned int count)
{
  byte res=0;

  for(int i=0;i<n;i++)
    res^=get_prg_p3(&robprg.vprg[i],count);
  return res;
}

void set_robprgcount3()
{
    robprg.vprg[0].prgcount++;
}

int get_robprgcount3()
{
  return robprg.vprg[0].prgcount;
}

void free_robprg3(int d,int n)
{
  /*for(int i=0;i<n;i++)
    free(robprg.vprg[i].pol);
  free(robprg.vprg);*/
}


/*********************multiple PRG for  AES htable***********/


void mult_gf2(byte *a,byte *b,byte *c)
{
  byte a1b1=multtable(a[1],b[1]);
  c[0]=multtable(a[0],b[0]) ^ multtable(32,a1b1);
  c[1]=multtable(a[1],b[0]) ^ multtable(a[0],b[1]) ^ a1b1;
}


void eval_poly_gf2(byte *pol,int d,byte *x,byte *r)//d is (n-1)*loc
{
  int i;
  r[0]=0;r[1]=0;
  for(i=d-1;i>=0;i--)
  {
    byte t[2];
    mult_gf2(r,x,t);
    r[0]=t[0] ^ pol[2*i];
    r[1]=t[1] ^ pol[2*i+1];
  }
}


typedef struct
{
  unsigned int prgcount;
  byte pol[2*(shares_N-1)];
  byte x[2];
  byte r[2];
} tprg1;

tprg1 prg1;


typedef struct
{
  tprg1 lrprg[num_M];//MI for increasing shares and MN for normal variant
} tmprg;

tmprg mprg;

void init_prg_p2(tprg1 *prg,int d)
{
  //prg->pol=(byte *) malloc(2*d*sizeof(byte));
    prg->prgcount=0;
    byte a[2*(shares_N-1)];
	gen_rand(a,2*d);
    for(int i=0;i<2*d;i++)
    {
       prg->pol[i]=a[i];
    }
    prg->x[0]=0;
    prg->x[1]=0;

}




void init_mprg2(int n,int ni)
{
  int i;

  for(i=0;i<ni;i++)
    init_prg_p2(&mprg.lrprg[i],n-1);

}


byte get_prg_p2(tprg1 *prg,int n,unsigned int count)
{
   byte x1[2];
   unsigned int temp=count/2;

   x1[0] = temp & 0xff;
   x1[1] = (temp >> 8)&0xff;

   if ((count%2!=0) && (prg->x[0]==x1[0])&& (prg->x[1]==x1[1]))
   {
             return prg->r[count%2];
   }


  prg->x[0] = x1[0];
  prg->x[1] = x1[1];

  eval_poly_gf2(prg->pol,n,prg->x,prg->r);
  return prg->r[count%2];
}


byte get_mprg_lr(int index,int n, unsigned int val)
{
  return get_prg_p2(&mprg.lrprg[index],n,val);
}


/******Code for PRESENT***********/



byte get_prg_p2_present(tprg1 *prg,int n,unsigned int count)
{
   byte x1[2];
   unsigned int temp=count/2;

   x1[0] = temp & 0xff;
   x1[1] = (temp >> 8)&0xff;

   if ((count%2!=0) && (prg->x[0]==x1[0])&& (prg->x[1]==x1[1]))
   {
             return prg->r[count%2];
   }


  prg->x[0] = x1[0];
  prg->x[1] = x1[1];

  eval_poly_gf2(prg->pol,n,prg->x,prg->r);
  return prg->r[count%2];
}


byte get_mprg_lr_present(int index,int n, unsigned int val)
{
  return get_prg_p2_present(&mprg.lrprg[index],n,val);
}


void set_mprg_lr_count()
{
    mprg.lrprg[0].prgcount++;
}

unsigned int get_mprg_lr_count(int d)
{
  return 2*d * mprg.lrprg[0].prgcount;
}

void free_mprg2(int n,int ni)
{
    /*int i;

    for(i=0;i<ni;i++)
     free(mprg.lrprg[i].pol);

    free(mprg.lrprg);*/
}



