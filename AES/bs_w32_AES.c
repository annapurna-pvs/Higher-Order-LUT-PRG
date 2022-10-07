#include<stdio.h>
#include<stdlib.h>
#include "../AES/aes.h"
#include "../Util/share.h"
#include "bs_AES32_share.h"
#if TRNG == 1
#include "MK64F12.h"
#else
#include <time.h>
#endif

byte w[176];
 unsigned int w_bs[88];
  byte wshare1[176][shares_N];
  unsigned int wshare_bs[88][shares_N];
  byte stateshare[16][shares_N];
  unsigned int state_bs[8][shares_N];
  #define lenn 16
  unsigned int S1[8][shares_N], Z1[18][shares_N],T1[68][shares_N];;


//bit_array is w and arr_b is w_bs



unsigned int swapBits(unsigned int n, int p1, int p2)
{
    /* Move p1'th to rightmost side */
    unsigned int bit1 =  (n >> p1) & 1;

    /* Move p2'th to rightmost side */
    unsigned int bit2 =  (n >> p2) & 1;

    /* XOR the two bits */
    unsigned int x = (bit1 ^ bit2);

    /* Put the xor bit back to their original positions */
    x = (x << p1) | (x << p2);

    /* XOR 'x' with the original number so that the
       two sets are swapped */
    unsigned int result = (n ^ x)&(0xFFFF);
	return result;
}

void left_swap_rotate(unsigned int state[8])
{
	unsigned int temp;
	int i;

	for(i=0;i<8;i++)
	{
		temp=state[i];
		temp=swapBits(temp,3,15);
		temp=swapBits(temp,3,11);
		temp=swapBits(temp,3,7);
		state[i]=((temp<<1)|(temp>>15));
		//printf("state: %d\n",state[i]);
		state[i]&=(65535);
	}

}

void mixcol_c1(unsigned int state[8])
{
    unsigned int temp[8],temp1;
	unsigned int of,i,t1;

	temp1=state[0];
	temp1=swapBits(temp1,3,15);
	temp1=swapBits(temp1,3,11);
	temp1=swapBits(temp1,3,7);
	t1=((temp1)<<1)|((temp1)>>15);


	of= state[0];
	of=of ^ t1;//state[0]^15+state[0]^14

	for(i=0;i<7;i++)
		temp[i]=state[((i+1)%8)];

	left_swap_rotate(state);//shift by 1

	for(i=0;i<8;i++)
	{
		temp[i]=temp[i]^state[((i+1)%8)];
	}

    temp[3]=temp[3]^of;
	temp[4]=temp[4]^of;
	temp[6]=temp[6]^of;
	temp[7]=of;


	for(i=0;i<8;i++)
		temp[i]=temp[i]^state[i];

	left_swap_rotate(state);//Shift by 2

	for(i=0;i<8;i++)
		temp[i]=temp[i]^state[i];

	left_swap_rotate(state);//Shift by 3

	for(i=0;i<8;i++)
		temp[i]=temp[i]^state[i];

	for(i=0;i<8;i++)
		state[i]=temp[i];
}



void bs_shiftrows_c1(unsigned int X[8])
{
	int i;
	for(i=0;i<8;i++)
	{//for each bit in a byte
		X[i]=swapBits(X[i],2,14);
		X[i]=swapBits(X[i],6,10);
		X[i]=swapBits(X[i],14,6);

		X[i]=swapBits(X[i],8,12);
		X[i]=swapBits(X[i],0,4);
		X[i]=swapBits(X[i],12,4);

		X[i]=swapBits(X[i],13,5);
		X[i]=swapBits(X[i],9,1);
    }
}


void bitslice(unsigned int X[8]){//(unsigned int *X,unsigned int *S) {
	/*unsigned int* Y=(unsigned int*) malloc(22*sizeof(unsigned int));
	unsigned int* T=(unsigned int*) malloc(68*sizeof(unsigned int));
	unsigned int* Z=(unsigned int*) malloc(18*sizeof(unsigned int));
	unsigned int* S=(unsigned int*) malloc(8*sizeof(unsigned int));*/
	unsigned int Y[22], T[68], Z[18], S[8];

    int i=0;

	//top linear
	Y[14] = X[3] ^ X[5];
	Y[13] = X[0] ^ X[6];
	Y[12] = Y[13] ^ Y[14];
	Y[9] = X[0] ^ X[3];
	Y[8] = X[0] ^ X[5];
	T[0] = X[1] ^ X[2];
	Y[1] = T[0] ^ X[7];
	Y[4] = Y[1] ^ X[3];
	Y[2] = Y[1] ^ X[0];
	Y[5] = Y[1] ^ X[6];
	T[1] = X[4] ^ Y[12];
	Y[3] = Y[5] ^ Y[8];
	Y[15] = T[1] ^ X[5];
	Y[20] = T[1] ^ X[1];
	Y[6] = Y[15] ^ X[7];
	Y[10] = Y[15] ^ T[0];
	Y[11] = Y[20] ^ Y[9];
	Y[7] = X[7] ^ Y[11];
	Y[17] = Y[10] ^ Y[11];
	Y[19] = Y[10] ^ Y[8];
	Y[16] = T[0] ^ Y[11];
	Y[21] = Y[13] ^ Y[16];
	Y[18] = X[0] ^ Y[16];


	//middle non-linear

	T[2] = Y[12] & Y[15];
	T[3] = Y[3] & Y[6];
	T[5] = Y[4] & X[7] ;
	T[7] = Y[13] & Y[16];
	T[8] = Y[5] & Y[1] ;
	T[10] = Y[2] & Y[7] ;
	T[12] = Y[9] & Y[11] ;
	T[13] = Y[14] & Y[17];
	T[4] = T[3] ^ T[2] ;
	T[6] = T[5] ^ T[2];
	T[9] = T[8] ^ T[7];
	T[11] = T[10] ^ T[7];
	T[14] = T[13] ^ T[12];
	T[17] = T[4] ^ T[14];
	T[19] = T[9] ^ T[14] ;
	T[21] = T[17] ^ Y[20];

	T[23] = T[19] ^ Y[21] ;
	T[15] = Y[8] & Y[10];
	T[26] = T[21] & T[23];
	T[16] = T[15] ^ T[12];
	T[18] = T[6] ^ T[16];
	T[20] = T[11] ^ T[16];
	T[24] = T[20] ^ Y[18];
	T[30] = T[23] ^ T[24];
	T[22] = T[18] ^ Y[19];
	T[25] = T[21] ^ T[22];
	T[27] = T[24] ^ T[26];
	T[31] = T[22] ^ T[26];
	T[28] = T[25] & T[27];
	T[32] = T[31] & T[30];
	T[29] = T[28] ^ T[22];
	T[33] = T[32] ^ T[24];
	T[34] = T[23] ^ T[33];
	T[35] = T[27] ^ T[33] ;
	T[42] = T[29] ^ T[33];
	Z[14] = T[29] & Y[2];
	T[36] = T[24] & T[35];
	T[37] = T[36] ^ T[34];
	T[38] = T[27] ^ T[36];
	T[39] = T[29] & T[38];
	Z[5] = T[29] & Y[7];
	T[44] = T[33] ^ T[37] ;
	T[40] = T[25] ^ T[39];
	T[41] = T[40] ^ T[37];
	T[43] = T[29] ^ T[40];
	T[45] = T[42] ^ T[41];
	Z[0] = T[44] & Y[15];
	Z[1] = T[37] & Y[6];

	Z[2] = T[33] & X[7];
	Z[3] = T[43] & Y[16];
	Z[4] = T[40] & Y[1];
	Z[6] = T[42] & Y[11];
	Z[7] = T[45] & Y[17];
	Z[8] = T[41] & Y[10];
	Z[9] = T[44] & Y[12];
	Z[10] = T[37] & Y[3];
	Z[11] = T[33] & Y[4];
	Z[12] = T[43] & Y[13];
	Z[13] = T[40] & Y[5];
	Z[15] = T[42] & Y[9];
	Z[16] = T[45] & Y[14];
	Z[17] = T[41] & Y[8];

	//bottom linear
	T[46] = Z[15] ^ Z[16];
	T[55] = Z[16] ^ Z[17] ;
	T[52] = Z[7] ^ Z[8];
	T[54] = Z[6] ^ Z[7];
	T[58] = Z[4] ^ T[46];
	T[59] = Z[3] ^ T[54] ;
	T[64] = Z[4] ^ T[59];
	T[47] = Z[10] ^ Z[11] ;

	T[49] = Z[9] ^ Z[10];
	T[63] = T[49] ^ T[58] ;
	T[66] = Z[1]^ T[63];
	T[62] = T[52] ^ T[58];
	T[53] = Z[0] ^ Z[3];
	T[50] = Z[2] ^ Z[12] ;
	T[57] = T[50] ^ T[53];
	T[60] = T[46] ^ T[57] ;

	T[61] = Z[14] ^ T[57];
	T[65] = T[61] ^ T[62] ;
	S[0] = T[59] ^ T[63];
	T[51] = Z[2] ^ Z[5] ;
	S[4] = T[51] ^ T[66];
	S[5] = T[47] ^ T[65] ;
	T[67] = T[64] ^ T[65];

	S[2] = (T[55] ^ T[67])^(65535);//S[2] = (T[55] ^ T[67])^65535;//(65535);xnor(T[55], T[67]);

	T[48] = Z[5] ^ Z[13];
	T[56] = Z[12] ^ T[48];
	S[3] = T[53] ^ T[66];
	S[1] = (T[64] ^ S[3])^(65535);//xnor(T[64],S[3]);//
	S[6] = (T[56]^T[62])^(65535);//xn(T[56],T[62]);//
	S[7] = (T[48]^ T[60])^(65535);


	X[0] = S[0];
	X[1] = S[1];
	X[2] = S[2];
	X[3] = S[3];
	X[4] = S[4];
	X[5] = S[5];
	X[6] = S[6];
	X[7] = S[7];
/*
	free(Y);
	free(T);
	free(S);
	free(Z);*/

}




void bs_addroundkey_w(unsigned int state[8],int round)
{
	int i;

	for(i=0; i<8; i++)
	{	//state[i]=state[i]^bs_key[round*8+i];
    state[i]=state[i]^w_bs[round*8+i];
  }
}

void encode_168_816(byte bit_array[16],unsigned int arr_b[8]){
int i,k,t,j=15,l=7;

    for(k=0;k<8;k++){
           arr_b[k]=0;
           j=15;
            //printf("l is %d\n",l);
            for(i=0;i<16;i++){
                t=(1 & (bit_array[i] >> l));//(1 && (bit_array[i] >> (k+1 )));
                arr_b[k]=arr_b[k]+t*pow_cus(2,j);
                j--;
            }

            l--;
        }

}

void decode_8168_168(byte bit_array[16],unsigned int arr_b[8]){
int i,k,t,j=0,pos=15,l=7;

    for(i=0;i<16;i++){
        bit_array[i]=0;
        l=7;
        for(k=0;k<8;k++){

        t=(1&(arr_b[k]>>pos));
        bit_array[i]=bit_array[i]+t*pow_cus(2,l);
        l--;
    }
    pos--;
    }
}

void aes_w(byte in[16],byte out[16])
{
  int i,j;
  int round=0;
  byte state[16],state1[16];
  unsigned int state_bs[8],state_bs1[8];
  byte temp;

  for(i=0;i<16;i++)
    state[i]=in[i];

  encode_168_816(state,state_bs);

   bs_addroundkey_w(state_bs,0);

	// AES Round Transformations (x 9)
	//------------------------------------------------------
	for(i=1; i<10; i++)
	{

        bitslice(state_bs);
  	    bs_shiftrows_c1(state_bs);
		    mixcol_c1(state_bs);
        bs_addroundkey_w(state_bs,i);

    }

	// AES Last Round
	//------------------------------------------------------
	bitslice(state_bs);
	bs_shiftrows_c1(state_bs);
	bs_addroundkey_w(state_bs,10);

	decode_8168_168(state,state_bs);

    for(i=0;i<16;i++)
        out[i]=state[i];
}

void encode_bskey_w()
{

int i,k,t,j=15,l=0,row;
for(row=0;row<11;row++)
	{
	l=7;
    for(k=0;k<8;k++){
           //arr_b[8*row+k]=0;
           w_bs[8*row+k]=0;
           j=15;

            for(i=0;i<16;i++){
                //t=(1 & (bit_array[16*row+i] >> l));//(1 && (bit_array[i] >> (k+1 )));
                t=(1 & (w[16*row+i] >> l));//(1 && (bit_array[i] >> (k+1 )));
                //arr_b[8*row+k]=arr_b[8*row+k]+t*pow_cus(2,j);
                w_bs[8*row+k]=w_bs[8*row+k]+t*pow_cus(2,j);
                j--;
            }

            l--;
        }

	}
}


void run_aes_w(byte in[16],byte out[16],byte key[16],int nt)
{
  int i;
 
  //unsigned int

  keyexpansion(key,w);
  encode_bskey_w();

  for(i=0;i<nt;i++)
    aes_w(in,out);

}

void run_bitslice(byte in[16],byte out[16],byte key[16],int nt)
{
    run_aes_w(in,out,key,nt);
}

//********************with shares*******************//
 //byte w[176];
 //unsigned int w_bs[88];
 //byte wshare[176][shares_N];
 // unsigned int wshare_bs[88][shares_N];
 //byte stateshare[16][shares_N];

  //unsigned int state_bs[8][shares_N];
//bit_array is wshare and whare_bs is arr_b
 void encode_bskeyn()
{
   int i,row=0,k,t,j=15,m,l=0,tmp,tmp1,tmp2;
  int n=shares_N;
  for(m=0;m<n;m++)
  {
	  for(row=0;row<11;row++)
		{
			l=7;
			for(k=0;k<8;k++)
	     	{
		      tmp=8*row;
			  wshare_bs[tmp+k][m]=0;
			  j=15;
			  for(i=0;i<16;i++)
			  {
				 tmp1=16*row;
                 t=(wshare1[tmp1+i][m] >> l);
				 t=t & 1;
                 tmp2=t*pow_cus(2,j);
				 wshare_bs[tmp+k][m]=wshare_bs[tmp+k][m]+tmp2;
				 j--;
			  }
			l--;
		    }
		}
  }
  /*
	for(m=0;m<n;m++)
	{
		for(row=0;row<11;row++)
		{
		l=7;
		for(k=0;k<8;k++)
		{
		   tmp=8*row;
           wshare_bs[tmp+k][m]=0;
           j=15;
           /*
            for(i=0;i<16;i++)
			{
                //t=(1 & (bit_array[16*row+i][m] >> l));//(1 && (bit_array[i] >> (k+1 )));
                 t=(1 & (wshare[16*row+i][m] >> l));//(1 && (bit_array[i] >> (k+1 )));
                //arr_b[8*row+k][m]=arr_b[8*row+k][m]+t*pow_cus(2,j);
                wshare_bs[8*row+k][m]=wshare_bs[8*row+k][m]+t*pow_cus(2,j);
                j--;
            }
            *//*
            l--;
		}

		}
	}*/
}
//byte stateshare[16][shares_N];

  //unsigned int state_bs[8][shares_N];
  //staeshare is bit_array state_bs is arr_b
void encode_bsn(){
byte i,k,t,j=15,l=7;
int m;
int n=shares_N;
    for(m=0;m<n;m++)
    {
        j=15,l=7;

        for(k=0;k<8;k++)
        {
            //arr_b[k][m]=0;
            state_bs[k][m]=0;
            j=15;

            for(i=0;i<16;i++)
            {
                //t=(1 & (bit_array[i][m] >> l));
                t=(1 & (stateshare[i][m] >> l));
                //arr_b[k][m]=arr_b[k][m]+t*pow_cus(2,j);
                state_bs[k][m]=state_bs[k][m]+t*pow_cus(2,j);
                j--;
            }

            l--;
        }

    }

}

void decode_bsn()
{
int i,k,t,j=0,m,pos=15,l=7,n=shares_N;

    for(m=0;m<n;m++)
    {
        pos=15,l=7;

        for(i=0;i<16;i++)
        {
            //bit_array[i][m]=0;
            stateshare[i][m]=0;
            l=7;
            for(k=0;k<8;k++)
            {
                //t=(1&(arr_b[k][m]>>pos));
                t=(1&(state_bs[k][m]>>pos));
                //bit_array[i][m]=bit_array[i][m]+t*pow_cus(2,l);
                stateshare[i][m]=stateshare[i][m]+t*pow_cus(2,l);
                
                l--;
            }
            pos--;
        }

    }
}

void XOR1n(unsigned int c[shares_N],unsigned int a[shares_N],unsigned int b[shares_N])
{
    int i,n=shares_N;
    for(i=0;i<n;i++)
        c[i] = a[i] ^ b[i];

}

void ANDn(unsigned int c[shares_N],unsigned int a[shares_N],unsigned int b[shares_N])
{
    int i,j,n=shares_N;
	unsigned int tmp,tmp2;

    for(i=0;i<n;i++)
        c[i]=a[i] & b[i];

    for(i=0;i<n;i++)
    {
        for(j=i+1;j<n;j++)
        {
            tmp=gen_rand32(); // rand();
            tmp2=(tmp ^ (a[i] & b[j]));
			tmp2=tmp2 ^ (a[j] & b[i]);
            c[i]=c[i]^tmp;
            c[j]=c[j]^tmp2;
        }
    }
}

void fullrefresh(unsigned int a[shares_N])
{
  int i,j,n=shares_N;
  for(i=0;i<n;i++)
  {
  for(j=i+1;j<n;j++)
  {
    unsigned int tmp=gen_rand32(); //rand();
    a[i]=a[i] ^ tmp;
    a[j]=a[j] ^ tmp;
  }
  }
}

void AND4(unsigned int c1[shares_N],unsigned int a1[shares_N],unsigned int b1[shares_N],unsigned int c2[shares_N],unsigned int a2[shares_N],unsigned int b2[shares_N])
{
        int n=shares_N;
    unsigned int temp[n],temp1[n],temp2[n];
    for(int i=0;i<n;i++)
    {
        temp1[i]=0;
        temp2[i]=0;
        temp1[i]=(a1[i]<<16)^a2[i];
        temp2[i]=(b1[i]<<16)^b2[i];
    }
    ANDn(temp,temp1,temp2);
	fullrefresh(temp);
	//fullrefresh on unpack
    ////unpacking
    for(int i=0;i<n;i++)
    {
       c1[i]=temp[i]>>16;
       c2[i]=temp[i] & 0xFFFF;
    }

}

void NOTn(unsigned int a[shares_N])	{
    a[0] = a[0] ^ (0xFFFF);
}

void MOVn(unsigned int dst[shares_N],unsigned int src[shares_N])
{
    int i,n=shares_N;

    for(i=0;i<n;i++)
	{
       dst[i] = src[i];
	}

}

//state_bs and wshare_bs
void bs_addroundkeyn(int round)
{
	int i;
  int n=shares_N;
	for(i=0; i<8; i++)
		XOR1n(state_bs[i],state_bs[i],wshare_bs[round*8+i]);
}
//state_bs is X
void bs_bitslicen()
{
	int i,n=shares_N;
	unsigned int Y[22][shares_N];



	XOR1n(Y[14] , state_bs[3] , state_bs[5]);
	XOR1n(Y[13] , state_bs[0] , state_bs[6]);
	XOR1n(Y[12] , Y[13] , Y[14]);
	XOR1n(Y[9] , state_bs[0] , state_bs[3]);
	XOR1n(Y[8] , state_bs[0] , state_bs[5]);
	XOR1n(T1[0] , state_bs[1] , state_bs[2]);
	XOR1n(Y[1] , T1[0] , state_bs[7]);
	XOR1n(Y[4] , Y[1] , state_bs[3]);
	XOR1n(Y[2] , Y[1] , state_bs[0]);
	XOR1n(Y[5] , Y[1] , state_bs[6]);
	XOR1n(T1[1] , state_bs[4] , Y[12]);
	XOR1n(Y[3] , Y[5] , Y[8]);
	XOR1n(Y[15] , T1[1] , state_bs[5]);
	XOR1n(Y[20] , T1[1] , state_bs[1]);
	XOR1n(Y[6] , Y[15] , state_bs[7]);
	XOR1n(Y[10] , Y[15] , T1[0]);
	XOR1n(Y[11] , Y[20] , Y[9]);
	XOR1n(Y[7] , state_bs[7] , Y[11]);
	XOR1n(Y[17] , Y[10] , Y[11]);
	XOR1n(Y[19] , Y[10] , Y[8]);
	XOR1n(Y[16] , T1[0] , Y[11]);
	XOR1n(Y[21] , Y[13] , Y[16]);
	XOR1n(Y[18] , state_bs[0] , Y[16]);


	//middle non-linear
   
	//ANDn(T[2] , Y[12] , Y[15]);
	//ANDn(T[3] , Y[3] , Y[6]);
    AND4(T1[2] , Y[12] , Y[15],T1[3] , Y[3] , Y[6]);
    //printf("done\n");
	//ANDn(T[5] , Y[4] , state_bs[7]);
	//ANDn(T[7] , Y[13] , Y[16]);
    AND4(T1[5] , Y[4] , state_bs[7] ,T1[7] , Y[13] , Y[16]);
	//ANDn(T[8] , Y[5] , Y[1]);
	//ANDn(T[10] , Y[2] , Y[7]);
    AND4(T1[8] , Y[5] , Y[1], T1[10] , Y[2] , Y[7]);
	//ANDn(T[12] , Y[9] , Y[11]);
	//ANDn(T[13] , Y[14] , Y[17]);
    AND4(T1[12] , Y[9] , Y[11],T1[13] , Y[14] , Y[17]);
	XOR1n(T1[4] , T1[3] , T1[2]);
	XOR1n(T1[6] , T1[5] , T1[2]);
	XOR1n(T1[9] , T1[8] , T1[7]);
	XOR1n(T1[11] , T1[10] , T1[7]);
	XOR1n(T1[14] , T1[13] , T1[12]);
	XOR1n(T1[17] , T1[4] , T1[14]);
	XOR1n(T1[19] , T1[9] , T1[14]);
	XOR1n(T1[21] , T1[17] , Y[20]);
    
	XOR1n(T1[23] , T1[19] , Y[21]);
	//ANDn(T[15] , Y[8] , Y[10]);
	//ANDn(T[26] , T[21] , T[23]);
    AND4(T1[15] , Y[8] , Y[10],T1[26] , T1[21] , T1[23]);
	XOR1n(T1[16] , T1[15] , T1[12]);
	XOR1n(T1[18] , T1[6] , T1[16]);
	XOR1n(T1[20] , T1[11] , T1[16]);
	XOR1n(T1[24] , T1[20] , Y[18]);
	XOR1n(T1[30] , T1[23] , T1[24]);
	XOR1n(T1[22] , T1[18] , Y[19]);
	XOR1n(T1[25] , T1[21] , T1[22]);
	XOR1n(T1[27] , T1[24] , T1[26]);
	XOR1n(T1[31] , T1[22] , T1[26]);
	//ANDn(T[28] , T[25] , T[27]);
	//ANDn(T[32] , T[31] , T[30]);
    AND4(T1[28] , T1[25] , T1[27],T1[32] ,T1[31], T1[30]);
	XOR1n(T1[29] , T1[28] , T1[22]);
	XOR1n(T1[33] , T1[32] , T1[24]);
	XOR1n(T1[34] , T1[23] , T1[33]);
	XOR1n(T1[35] , T1[27] , T1[33]);
	XOR1n(T1[42] , T1[29] , T1[33]);
	//ANDn(Z[14] , T[29] , Y[2]);
	//ANDn(T[36] , T[24] , T[35]);
    AND4(Z1[14] , T1[29] , Y[2],T1[36] , T1[24] , T1[35]);
	XOR1n(T1[37] , T1[36] , T1[34]);
	XOR1n(T1[38] , T1[27] , T1[36]);
	//ANDn(T[39] , T[29] , T[38]);
	//ANDn(Z[5] , T[29] , Y[7]);
    AND4(T1[39] , T1[29] , T1[38],Z1[5] , T1[29] , Y[7]);


	XOR1n(T1[44] , T1[33] , T1[37]);
	XOR1n(T1[40] , T1[25] , T1[39]);
	XOR1n(T1[41] , T1[40] , T1[37]);
	XOR1n(T1[43] , T1[29] , T1[40]);
	XOR1n(T1[45] , T1[42] , T1[41]);
	
	//ANDn(Z[0] , T[44] , Y[15]);
	//ANDn(Z[1] , T[37] , Y[6]);
    AND4(Z1[0] , T1[44] , Y[15],Z1[1] , T1[37] , Y[6]);

	//ANDn(Z[2] , T[33] , state_bs[7]);
	//ANDn(Z[3] , T[43] , Y[16]);
    AND4(Z1[2] , T1[33] , state_bs[7],Z1[3] , T1[43] , Y[16]);
	//ANDn(Z[4] , T[40] , Y[1]);
	//ANDn(Z[6] , T[42] , Y[11]);
    AND4(Z1[4] , T1[40] , Y[1],Z1[6] , T1[42] , Y[11]);
	//ANDn(Z[7] , T[45] , Y[17]);
	//ANDn(Z[8] , T[41] , Y[10]);
    AND4(Z1[7] , T1[45] , Y[17],Z1[8] , T1[41] , Y[10]);
	//ANDn(Z[9] , T[44] , Y[12]);
	//ANDn(Z[10] , T[37] , Y[3]);
    AND4(Z1[9] , T1[44] , Y[12],Z1[10] , T1[37] , Y[3]);
	//ANDn(Z[11] , T[33] , Y[4]);
	//ANDn(Z[12] , T[43] , Y[13]);
    AND4(Z1[11] , T1[33] , Y[4],Z1[12] , T1[43] , Y[13]);
	//ANDn(Z[13] , T[40] , Y[5]);
	//ANDn(Z[15] , T[42] , Y[9]);
    AND4(Z1[13] , T1[40] , Y[5],Z1[15] , T1[42] , Y[9]);
	//ANDn(Z[16] , T[45] , Y[14]);
	//ANDn(Z[17] , T[41] , Y[8]);
    AND4(Z1[16] , T1[45] , Y[14], Z1[17] ,T1[41] , Y[8]);
    
	//bottom linear
	
	XOR1n(T1[46] , Z1[15] , Z1[16]);
	XOR1n(T1[55] , Z1[16] , Z1[17]);
	XOR1n(T1[52] , Z1[7] , Z1[8]);
	XOR1n(T1[54] , Z1[6] , Z1[7]);
	XOR1n(T1[58] , Z1[4] , T1[46]);
	XOR1n(T1[59] , Z1[3] , T1[54]);
	XOR1n(T1[64] , Z1[4] , T1[59]);
	XOR1n(T1[47] , Z1[10] , Z1[11]);

	XOR1n(T1[49] , Z1[9] , Z1[10]);
	XOR1n(T1[63] , T1[49] , T1[58]);
	XOR1n(T1[66] , Z1[1], T1[63]);
	XOR1n(T1[62] , T1[52] , T1[58]);
	XOR1n(T1[53] , Z1[0] , Z1[3]);
	XOR1n(T1[50] , Z1[2] , Z1[12]);
	XOR1n(T1[57] , T1[50] , T1[53]);
	XOR1n(T1[60] , T1[46] , T1[57]);
    
	XOR1n(T1[61] , Z1[14] , T1[57]);
	XOR1n(T1[65] , T1[61] , T1[62]);
	
	XOR1n(S1[0] , T1[59] , T1[63]);
	XOR1n(T1[51] , Z1[2] , Z1[5]);
	XOR1n(S1[4] , T1[51] , T1[66]);
	XOR1n(S1[5] , T1[47] , T1[65]);
	XOR1n(T1[67] , T1[64] , T1[65]);

	XOR1n(S1[2] , T1[55] , T1[67]);

	NOTn(S1[2]);

	XOR1n(T1[48] , Z1[5] , Z1[13]);
	XOR1n(T1[56] , Z1[12] , T1[48]);
	XOR1n(S1[3] , T1[53] , T1[66]);
	XOR1n(S1[1] , T1[64] , S1[3]);

	NOTn(S1[1]);
	XOR1n(S1[6] , T1[56], T1[62]);
	NOTn(S1[6]);
	XOR1n(S1[7] , T1[48], T1[60]);
	NOTn(S1[7]);

//throwing hardfalut error
/*	MOVn(state_bs[0], S1[0]);
	MOVn(state_bs[1], S1[1]);
	MOVn(state_bs[2], S1[2]);
	MOVn(state_bs[3], S1[3]);
	MOVn(state_bs[4], S1[4]);
//	MOVn(state_bs[5], S1[5]);
//	MOVn(state_bs[6], S1[6]);
//	MOVn(state_bs[7], S1[7]);*/

	for(i=0;i<8;i++)
	{
		for(int j=0;j<n;j++)
		{
			state_bs[i][j]=S1[i][j];
		}
	}

	/*for(i=0;i<22;i++)
	{
		free(Y[i]);
	}

	for(i=0;i<68;i++)
	{
		free(T[i]);s
	}
	for(i=0;i<18;i++)
	{
		free(Z[i]);
	}
	for(i=0;i<8;i++)
	{
		free(S[i]);
	}*/


}


void swapBits_sharen(unsigned int n[shares_N], int k1, int k2)
{
    int i=0,nshr=shares_N;

    for(i=0;i<nshr;i++)
    {
        unsigned int b1 =  (n[i] >> k1) & 1;
        unsigned int b2 =  (n[i] >> k2) & 1;
        unsigned int x = (b1 ^ b2);
        x = (x << k1) | (x << k2);
        n[i] = (n[i] ^ x)&(0xFFFF);
    }
}

void bs_shiftrowsn()
{
	int i,n=shares_N;
	for(i=0;i<8;i++)
	{//for each int in BS array
		swapBits_sharen(state_bs[i],2,14);
		swapBits_sharen(state_bs[i],6,10);
		swapBits_sharen(state_bs[i],14,6);

		swapBits_sharen(state_bs[i],8,12);
		swapBits_sharen(state_bs[i],0,4);
		swapBits_sharen(state_bs[i],12,4);

		swapBits_sharen(state_bs[i],13,5);
		swapBits_sharen(state_bs[i],9,1);
    }
}

void rotate_sharen(unsigned int dst[shares_N],unsigned int src[shares_N],unsigned int bits)	{

    int i,n=shares_N;
	unsigned int tmp,tmp1;
    for(i=0;i<n;i++)
    {
		tmp=(src[i] << bits);
		tmp1=(src[i] >> (lenn - bits));
        dst[i] = tmp | tmp1;
        dst[i]= (dst[i] & 0xFFFF);
    }

}

void left_swap_rotate_sharen()
{
	unsigned int temp[shares_N];
	int i,n=shares_N;

	//temp=(unsigned int*) malloc(n*sizeof(unsigned int));
	for(i=0;i<8;i++)
	{
		MOVn(temp,state_bs[i]);
		swapBits_sharen(temp,3,15);
		swapBits_sharen(temp,3,11);
		swapBits_sharen(temp,3,7);
		rotate_sharen(state_bs[i],temp,1);
	}
}

void bs_mixColumnsn()
{
    unsigned int temp[8][shares_N],temp1[shares_N];
    int n=shares_N,i;
	unsigned int of[shares_N],t1;

/*	for(i=0;i<8;i++)
         temp[i]=(unsigned int*) malloc(n*sizeof(unsigned int));

    temp1=(unsigned int*) malloc(n*sizeof(unsigned int));
    of=(unsigned int*) malloc(n*sizeof(unsigned int));*/

	MOVn(temp1,state_bs[0]);
	swapBits_sharen(temp1,3,15);
	swapBits_sharen(temp1,3,11);
	swapBits_sharen(temp1,3,7);
    rotate_sharen(temp1,temp1,1);


	MOVn(of, state_bs[0]);
	XOR1n(of,of, temp1);

	for(i=0;i<7;i++)
		MOVn(temp[i],state_bs[((i+1)%8)]);
   	left_swap_rotate_sharen();//shift by 1


	for(i=0;i<8;i++)
	{
		XOR1n(temp[i],temp[i],state_bs[((i+1)%8)]);
	}

	XOR1n(temp[3],temp[3],of);
	XOR1n(temp[4],temp[4],of);
	XOR1n(temp[6],temp[6],of);
	MOVn(temp[7],of);

	for(i=0;i<8;i++)
		XOR1n(temp[i],temp[i],state_bs[i]);

	left_swap_rotate_sharen();//Shift by 2

	for(i=0;i<8;i++)
		XOR1n(temp[i],temp[i],state_bs[i]);

	left_swap_rotate_sharen();//Shift by 3

	for(i=0;i<8;i++)
		XOR1n(temp[i],temp[i],state_bs[i]);

	for(i=0;i<8;i++)
		MOVn(state_bs[i],temp[i]);
}


int flag=1;
void aes_share_subkeys_bitslice(byte in[16],byte out[16])
{
  int i,k,j;
  int round=0;
  int n=shares_N;
  byte tmp[1];

 
  for(i=0;i<16;i++)
  {
   
	//since going nt hard fault doing without calling the funtcion

	//share(in[i],stateshare[i],n);
	stateshare[i][0]=in[i]; //x
	for(int j=1;j<n;j++)
	stateshare[i][j]=0;
	//refresh
	//refresh(stateshare[i],n);
    for(int j=1;j<n;j++)
	{
		gen_rand(tmp,1);
		stateshare[i][0]=stateshare[i][0]^tmp[0];
		stateshare[i][j]=stateshare[i][j]^tmp[0];

	}
	
  }

  //for(i=0;i<8;i++)
	  //state_bs[i]=(unsigned int*) malloc(n*sizeof(unsigned int));

    encode_bsn();
    bs_addroundkeyn(0);

	// AES Round Transformations (x 9)
	//------------------------------------------------------
	for(i=1; i<10; i++)
	{
        bs_bitslicen();//(state_bs,n);
        bs_shiftrowsn();
        bs_mixColumnsn();
        bs_addroundkeyn(i);
	}

	// AES Last Round
	//------------------------------------------------------

	bs_bitslicen();
	bs_shiftrowsn();
	bs_addroundkeyn(10);

	decode_bsn();

    for(i=0;i<16;i++)
    {
        out[i]=decode1(stateshare[i],n);
        //free(stateshare[i]);
    }


}

int run_aes_share_bitslice(byte in[16],byte out[16],byte key[16],int nt){

  int i,k,n=shares_N;
	byte x;
  
  keyexpansion(key,w);
  for(i=0;i<176;i++)
  {
    //share doing it manually as its showing hard fault error
    x=w[i];
    wshare1[i][0]=x;
    for(int j=1;j<n;j++)
    wshare1[i][j]=0;
    //refresh also
    for(int j=1;j<n;j++)
    {
      byte tmp[1];
	  gen_rand(tmp,1);
      wshare1[i][0]=wshare1[i][0]^tmp[0];
      wshare1[i][j]=wshare1[i][j]^tmp[0];
    }
  
    
   }


	encode_bskeyn();

  
    aes_share_subkeys_bitslice(in,out);
  

  
  return (double) (0.00) ;

}



void run_bitslice_shares(byte in[16],byte out[16],byte key[16],int nt,double *time_b)
{
	 
	unsigned int begin1=0, end1=0;
	long sec,nsec;
    double temp=0.0;
	#if TRNG==0
    struct timespec begin, end;
	
	#endif
	#if TRNG==0
        clock_gettime(CLOCK_REALTIME, &begin);
        #endif // TRNG
	 #if TRNG==1
            reset_systick();
            begin1 = SysTick->VAL; // Obtains the start time
    #endif // TRNG
   for(int i=0;i<nt;i++)
   run_aes_share_bitslice(in,out,key,nt);

    #if TRNG==1
        end1 = SysTick->VAL; // Obtains the stop time
        time_b[0] = ((double) (begin1-end1))/nt; // Calculates the time taken
    #endif // TRNG
	#if TRNG==0
        clock_gettime(CLOCK_REALTIME, &end);
        sec = end.tv_sec - begin.tv_sec;
        nsec = end.tv_nsec - begin.tv_nsec;
        temp = sec + nsec*1e-9;

        time_b[0] = temp*UNIT/nt;
        #endif // TRNG

}