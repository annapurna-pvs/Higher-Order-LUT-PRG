#include "stdio.h"
#include "../Util/common.h"
#include "bs_AES32_share.h"
#include "math.h"

static unsigned long x=123456789, y=362436069, z=521288629;
static unsigned int randcount=1;

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
  //return z;
  return z;//%256;//rand();
}
byte pow_cus(byte base,byte exp){
byte i,res=1;

for(i=0;i<exp;i++)
    res=res*base;
return res;
}
/*
void init_randcount()
{
  randcount=0;
}
*/
/*unsigned int get_randcount()
{
  return randcount;
}*/

void refresh_b(byte a[shares_N])
{
  int i,n=shares_N;
  for(i=1;i<n;i++)
  {
    byte tmp=xorshf96(); //rand();
    a[0]=a[0] ^ tmp;
    a[i]=a[i] ^ tmp;
  }
}

void share_b(byte x,byte a[shares_N])
{
  int i,n=shares_N;
  a[0]=x;
  for(i=1;i<n;i++)
    a[i]=0;
}



void refresh16(unsigned int a[],int n)
{
  int i;
  for(i=1;i<n;i++)
  {
    unsigned int tmp=xorshf96()%65536;//%(pow_cus(2,16));//xorshf96(); //rand();
   // printf("tmp is %d\n",tmp);
    a[0]=a[0] ^ tmp;
    a[i]=a[i] ^ tmp;
  }
}

void share16(unsigned int x,unsigned int a[],int n)
{
  int i;
  a[0]=x;
  for(i=1;i<n;i++)
    a[i]=0;
}

unsigned int reconstruct16(unsigned int a[],int n)
{
    unsigned int s=a[0];
    for(int i=1;i<n;i++)
        s=s^a[i];

    return s;
}



byte xorop1(byte a[],int n)
{
  int i;
  byte r=0;
  for(i=0;i<n;i++)
    r^=a[i];
  return r;
}

byte decode1(byte a[],int n)
{
  int i;
  //for(i=0;i<n;i++)
    //refresh(a,n);
  return xorop1(a,n);
}


/********************Custom code for compression*************************/

void gen_share(byte x, byte a[], int n){

  share_b(x, a);
  refresh_b(a);
}


byte first(byte var, byte l){

  // var = var(1) || var(2), this will return var(1), which is n-l bits long

  return var>>l;
}

byte second(byte var, byte l){

  // var = var(1) || var(2), this will return var(2), which is l bits long
  byte t_l=pow_cus(2,l);
  byte t_nl=pow_cus(2,(8-l));

  return var % t_l;
}

byte random_byte(){

  byte b = xorshf96(); //xorshf96();
  return b;
}

byte random_f(byte l){

  return first(random_byte(), l);
}

byte random_s(byte l){

  return second(random_byte(), l);
}

void gen_r(byte *r, int l){

    byte i;
    byte t_l=pow(2,4);
    byte t_nl=pow(2,4);

  for(i=0; i<t_l; i++){
    r[i] = random_f(l);
  }
}
/*  Old gen y1
byte gen_y1(byte y1){

	return random_byte();

}*/

void gen_y1(byte *y1,int l){

	byte i;
    byte t_l=pow_cus(2,l);
    byte t_nl=pow_cus(2,(8-l));

    for(i=0; i<t_nl; i++){
       y1[i] = random_byte();
  }



	//return random_byte();

}


void gen_y2(byte *y2, int l){

	byte i;
    byte t_l=pow_cus(2,l);
    byte t_nl=pow_cus(2,(8-l));

    for(i=0; i<t_l; i++){
     y2[i] = random_byte();
  }
}

byte xor(byte b1, byte b2){

  return b1^b2;
}


