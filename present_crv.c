#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "share.h"
#include "present.h"
#include "aes_rp.h"

void crv_present_share(byte *,int );
void testCRV_present_share();


/*Encodings for the CRV method. For each S-box, there are p1,q1,p2. Irreducible polynomial for F_16: a^4 + a + 1.
L={0, 1, 2, 3, 4, 6, 8, 9, 12} */

byte CRV[27]=    {4, 5, 5, 15, 13, 5, 12, 1, 10, 2, 10, 1, 0, 14, 1, 4, 15, 13, 4, 9, 1, 2, 3, 13, 5, 0, 0};

/* Look up tables for the linear polynomials corresponding to each of p1, q1, p2, for each S-box. For each polynomial pi or qi, first is the linear polynomial of x, then that of x^3. */
byte LinP[96]={0, 1, 0, 1, 15, 14, 15, 14, 1, 0, 1, 0, 14, 15, 14, 15, 0, 1, 2, 3, 0, 1, 2, 3, 14, 15, 12, 13, 14, 15, 12, 13, 0, 1, 5, 4, 6, 7, 3, 2, 8, 9, 13, 12, 14, 15, 11, 10, 0, 3, 6, 5, 2, 1, 4, 7, 7, 4, 1, 2, 5, 6, 3, 0, 0, 14, 2, 12, 4, 10, 6, 8, 14, 0, 12, 2, 10, 4, 8, 6, 0, 15, 5, 10, 12, 3, 9, 6, 0, 15, 5, 10, 12, 3, 9, 6};

/****Mult tables*****/
byte multTab[256]={0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0, 2, 4, 6, 8, 10, 12, 14, 3, 1, 7, 5, 11, 9, 15, 13, 0, 3, 6, 5, 12, 15, 10, 9, 11, 8, 13, 14, 7, 4, 1, 2, 0, 4, 8, 12, 3, 7, 11, 15, 6, 2, 14, 10, 5, 1, 13, 9, 0, 5, 10, 15, 7, 2, 13, 8, 14, 11, 4, 1, 9, 12, 3, 6, 0, 6, 12, 10, 11, 13, 7, 1, 5, 3, 9, 15, 14, 8, 2, 4, 0, 7, 14, 9, 15, 8, 1, 6, 13, 10, 3, 4, 2, 5, 12, 11, 0, 8, 3, 11, 6, 14, 5, 13, 12, 4, 15, 7, 10, 2, 9, 1, 0, 9, 1, 8, 2, 11, 3, 10, 4, 13, 5, 12, 6, 15, 7, 14, 0, 10, 7, 13, 14, 4, 9, 3, 15, 5, 8, 2, 1, 11, 6, 12, 0, 11, 5, 14, 10, 1, 15, 4, 7, 12, 2, 9, 13, 6, 8, 3, 0, 12, 11, 7, 5, 9, 14, 2, 10, 6, 1, 13, 15, 3, 4, 8, 0, 13, 9, 4, 1, 12, 8, 5, 2, 15, 11, 6, 3, 14, 10, 7, 0, 14, 15, 1, 13, 3, 2, 12, 9, 7, 6, 8, 4, 10, 11, 5, 0, 15, 13, 2, 9, 6, 4, 11, 1, 14, 12, 3, 8, 7, 5, 10, };

//byte sq[16]={0, 1, 4, 5, 3, 2, 7, 6, 12, 13, 8, 9, 15, 14, 11, 10};

byte logt[16]={0 ,15 ,1 ,4 ,2 ,8 ,5 ,10 ,3 ,14 ,9 ,7 ,6 ,13 ,11 ,12 };

byte alogt[16]={1 ,2 ,4 ,8 ,3 ,6 ,12 ,11 ,5 ,10 ,7 ,14 ,15 ,13 ,9 ,1 };

byte multable(byte x,byte y)
{
  return multTab[16*x+y];
}



byte polyL(byte *pol,int d,byte x)
{
  int i;
  byte r=0, xL[9];

  xL[0]=1; xL[1]=x;
  xL[2]=multable(xL[1],xL[1]); xL[3]=multable(xL[2],xL[1]); xL[4]=multable(xL[2],xL[2]);
  xL[5]=multable(xL[3],xL[3]); xL[6]=multable(xL[4],xL[4]); xL[7]=multable(xL[3],xL[5]);
  xL[8]=multable(xL[5],xL[5]);

  for(i=0;i<d;i++)
    r^=multable(pol[i],xL[i]);

 return r;
}


byte polyCRV(byte *pol,byte x)
{
  int i;

  byte p1=polyL(pol,9,x);
  byte q1=polyL(pol+9,9,x);
  byte p2=polyL(pol+18,9,x);

  return multable(p1,q1)^p2;
}


void crv_present_share(byte *x,int n)
{
  int i,j,k,nmult;
  byte zi[3][n];
  byte v[3][n];
  byte temp[n];

  byte *pol=CRV;
  byte *lin = LinP;

  memcpy(zi[0],x,n);
  nmult=0;

  square_share(zi[0],zi[1],n);

  multshare(zi[0],zi[1],zi[2],n);//zi[2]=x^3

  nmult++;

  //compute p1, q1, p2

  for(i=0;i<3;i++) //3 polynomials
  {
    share4(pol[i*9],v[i],n);
    for(k=0;k<n;k++)
     v[i][k] ^= (lin[32*i+(zi[0][k])] ^ lin[32*i+16+(zi[2][k])]);
  }

  multshare(v[0],v[1],temp,n);
  nmult+=1;

  for(k=0;k<n;k++)
    x[k]=temp[k] ^  v[2][k];
}


void refresh4(byte a[],int n)
{
  int i;
  byte temp[2];
  gen_rand(temp,2);
  for(i=1;i<n;i++)
  {
    byte tmp=temp[i-1];
    a[0]=a[0] ^ tmp;
    a[i]=a[i] ^ tmp;
  }
}

void share4(byte x,byte a[],int n)
{
  int i;
  a[0]=x;
  for(i=1;i<n;i++)
    a[i]=0;
}

byte polyCRV_tab(byte *pol,byte *lin,byte x)
{
  byte i,j,k;
  byte r=0, p1=0, q1=0, p2=0, xL[3];

  xL[0]=x;
  xL[1]=multable(xL[0],xL[0]);
  xL[2]=multable(xL[1],xL[0]);

  p1 ^= pol[0] ^ lin[(xL[0])] ^ lin[16+(xL[2])];
  q1 ^= pol[9] ^ lin[32+(xL[0])] ^ lin[32+16+(xL[2])];
  p2 ^= pol[9*2] ^ lin[32*2+(xL[0])]^ lin[32*2+16+(xL[2])];

  j=multable(p1,q1)^p2;

  return j;
}


byte polyCRV_tab_find(byte *pol,byte *lin,byte x)
{
  byte i,j;
  byte r=0, p1=0, q1=0, p2=0, xL[3];

  xL[0]=x;
  xL[1]=multable(xL[0],xL[0]); xL[2]=multable(xL[1],xL[0]);

  for(j=0;j<9;j++)
  {
    p1 ^= pol[j+0] ^ lin[(xL[0])] ^ lin[16+xL[2]];
    q1 ^= pol[j+9] ^ lin[32+(xL[0])] ^ lin[32+16+(xL[2])];
    p2 ^= pol[j+9*2] ^ lin[32*2+(xL[0])] ^ lin[32*2+16+(xL[2])];

  }

  return multable(p1,q1)^p2;
}


void testCRV_present_share()
{
  int i,j,n=3;
  byte x,y;
  byte a[3];

  for(x=0;x<16;x++)
	{
	  share4(x,a,n);
	  refresh4(a,n);
	  crv_present_share(a,n);

	  y=decode(a,n);

   	  if(y!=sbox_p[x])
   	  {
            printf("Expected: %d and Obtained: %d and Output not matched\n",sbox_p[x],y);
      }
   	  else
      {
            printf("match %d %d\n",sbox_p[x],y);
       }

	}
}


void testlagrangeCRV()
{
  int i,j;
  byte x;


  for(x=0;x<16;x++)
  if((polyCRV(CRV,x)&15)!=sbox_p[(int)(x)])
  {
	printf(stderr,"\nIncorrect Lagrange interpolation \n");

	for(j=0;j<3;j++)
        printf("%d\n",polyL(CRV+9*j,9,x));

   }
   else
     printf("correct Interpolation\n");
}

void testlagrangeCRV_tab()
{
  int i,j;
  byte x;
  byte t1,t2;

   for(x=0;x<16;x++)
   {
      printf("test: %d\n",x);
      t1=polyCRV_tab(CRV,LinP,x)&15;
      t2=sbox_p[x];

      if(t1!=t2)				/*Output only 4 bits*/
     {
       printf("\nIncorrect Lagrange interpolation with tables  %d and %d \n",t1,t2);
	 }
	 else
       printf("correct\n");
     }

}


int main_crv()
{
 testCRV_present_share();
 //testlagrangeCRV();
 //testlagrangeCRV_tab();
 //test_gen_table();
 return 0;
}

void test_gen_table() //c-code to verify the linear polynomial computation
{
    byte i,j;
    byte r=0, xL[4],res[16],res1[16],x;

    byte p1c[4]={5,5,13,12};
    byte p2c[4]={9,1,3,5};
    byte q1c[4]={10,1,14,4};

    byte p31c[4]={15,5,10,1};
    byte p32c[4]={2,13,0,0};
    byte q31c[4]={0,1,13,15};

    for(i=0;i<16;i++)
    {
        res[i]=0;
        res1[i]=0;
    }

     for(x=0;x<16;x++)
     {
        xL[0]=x;//x^1
        xL[1]=multable(xL[0],xL[0]);//x^2
        xL[2]=multable(xL[1],xL[1]); //x^4
        xL[3]=multable(xL[2],xL[2]);//x^8

    for(j=0;j<4;j++)
        {
            res[x]^=multable(p1c[j],xL[j]);
            res1[x]^=multable(p31c[j],xL[j]);
        }


    for(i=0;i<16;i++)
        printf("%d, ",res[i]);


    for(i=0;i<16;i++)
        printf("%d, ",res1[i]);

      /**** q1's*/

      for(i=0;i<16;i++)
    {
        res[i]=0;
        res1[i]=0;
    }

    for(j=0;j<4;j++)
        {
            res[x]^=multable(q1c[j],xL[j]);
            res1[x]^=multable(q31c[j],xL[j]);
        }

    for(i=0;i<16;i++)
        printf("%d, ",res[i]);

    for(i=0;i<16;i++)
        printf("%d, ",res1[i]);

        /**** p2's*/

      for(i=0;i<16;i++)
    {
        res[i]=0;
        res1[i]=0;
    }

    for(j=0;j<4;j++)
    {
            res[x]^=multable(p2c[j],xL[j]);
            res1[x]^=multable(p32c[j],xL[j]);
    }

    for(i=0;i<16;i++)
        printf("%d, ",res[i]);

    for(i=0;i<16;i++)
        printf("%d, ",res1[i]);

     }

}
