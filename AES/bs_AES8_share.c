#include <stdint.h>
#include <time.h>


#if TRNG == 1
#include "MK64F12.h"
#else
#include <time.h>
#endif

#include "../Util/common.h"
#include "../Util/driver_functions.h"

#include "aes_htable_prg.h"


const byte Sn=8; //number of bits in input
const byte Tn=8;//number of bits of register in use

byte w[176];
byte wshare1[176][shares_N]; // for key bitslice representation should contain [3][176] input and [88][3] output
byte wshare_bs[176][shares_N];

byte stateshare[16][shares_N];
byte state_bs[16][shares_N];


//*************refresh******************



//*****Primitive functions*****

void MOV8n(byte dst[],byte src[],byte n)
{
    byte i;

	for(i=0;i<n;i++)
            dst[i]=src[i];
}

void XOR8n(byte c[],byte a[],byte b[],byte n)
{
    byte i;
    for(i=0;i<n;i++)
    {
        c[i] = a[i] ^ b[i];
    }

}

void AND8n(byte c[],byte a[],byte b[],byte n)
{
    byte i,j;

    for(i=0;i<n;i++)
        c[i]=a[i] & b[i];

    byte dim=(shares_N*(shares_N-1))/2;
    byte rand[dim];
    gen_rand(rand,dim);

    for(i=0;i<n;i++)
    {
        for(j=i+1;j<n;j++)
        {
            byte tmp=rand[i+j-1]; // rand();
            byte tmp2=(tmp ^ (a[i] & b[j]) ^ (a[j] & b[i]));
            c[i]^=tmp;
            c[j]^=tmp2;
        }
    }

		locality_refresh(c,n);
}


void NOT8n(byte a[])	{
    a[0] = a[0] ^ (0xFF);
}



void swap_share8n(byte a[],byte b[],byte n)
{
   byte i,m;
   for(i=0;i<n;i++)
   {
        m=a[i];
        a[i]=b[i];
        b[i]=m;
   }

}


void swapBits_share8n(byte *n, byte k1, byte k2,byte nshr)
{
    byte i;
    byte b1,b2,x;

    for(i=0;i<nshr;i++)
    {

        b1 =  (n[i] >> k1) & 1;
        b2 =  (n[i] >> k2) & 1;
        x = (b1 ^ b2);

        x = (x << k1) | (x << k2);
        n[i] = (n[i] ^ x)&(0xFF);//the mask depends on the number of bits in target representation
    }
}


void left_swap_rn(byte n)
{
	byte temp[n],temp1[n];
	int i;

	for(i=0;i<8;i++)
	{
		MOV8n(temp1,state_bs[2*i],n);//odd bits
		MOV8n(temp,state_bs[2*i+1],n);//even bits

		swapBits_share8n(temp1,6,7,n);
		swapBits_share8n(temp1,5,4,n);
		swapBits_share8n(temp1,3,2,n);
		swapBits_share8n(temp1,1,0,n);

		swap_share8n(temp,temp1,n);

        MOV8n(state_bs[2*i],temp1,n);
        MOV8n(state_bs[2*i+1],temp,n);
	}
}


//*****8-bit Encodings*******


//****Round functions******

void bs_addroundkey_share8n(byte round,byte n)
{
	int i;
	for(i=0; i<16; i++)
		XOR8n(state_bs[i],state_bs[i],wshare_bs[round*16+i],n);
}


void bs_shiftrows_share8n(byte n)
{
	byte i;
	for(i=0;i<8;i++)
	{//for each bit in a byte

    swapBits_share8n(state_bs[2*i+1],1,7,n);//first
	swapBits_share8n(state_bs[2*i+1],3,5,n);//first
	swapBits_share8n(state_bs[2*i+1],7,3,n);//first

	swapBits_share8n(state_bs[2*i+1],4,6,n);
	swapBits_share8n(state_bs[2*i+1],0,2,n);
	swapBits_share8n(state_bs[2*i+1],6,2,n);

	swapBits_share8n(state_bs[2*i],6,2,n);//second
	swapBits_share8n(state_bs[2*i],4,0,n);//second

    }

}


void bs_mixcol_share8n(byte n)
{
    byte temp[16][shares_N],temp0[shares_N],temp1[shares_N];
    byte of_odd[shares_N],of_even[shares_N],i;

	MOV8n(temp0,state_bs[0],n);//odd
	MOV8n(temp1,state_bs[1],n);//even

	swapBits_share8n(temp0,6,7,n);
    swapBits_share8n(temp0,4,5,n);
    swapBits_share8n(temp0,2,3,n);
    swapBits_share8n(temp0,0,1,n);

	swap_share8n(temp0,temp1,n);

	XOR8n(of_odd, state_bs[0],temp0,n);
	XOR8n(of_even, state_bs[1],temp1,n);

	for(i=0;i<7;i++)
		{
				MOV8n(temp[2*i],state_bs[2*i+2],n);
				MOV8n(temp[2*i+1],state_bs[2*i+3],n);
		}

	left_swap_rn(n);//shift by 1

	for(i=0;i<8;i++)
	{
        XOR8n(temp[2*i],temp[2*i],state_bs[((2*i+2)%16)], n);
        XOR8n(temp[2*i+1],temp[2*i+1],state_bs[((2*i+3)%16)], n);
	}

    XOR8n(temp[6],temp[6],of_odd, n);
    XOR8n(temp[7],temp[7],of_even, n);

    XOR8n(temp[8],temp[8],of_odd, n);
    XOR8n(temp[9],temp[9],of_even, n);

    XOR8n(temp[12],temp[12],of_odd, n);
    XOR8n(temp[13],temp[13],of_even, n);

    MOV8n(temp[14],of_odd, n);
    MOV8n(temp[15],of_even, n);


	for(i=0;i<16;i++)
	{
		XOR8n(temp[i],temp[i],state_bs[i], n);
	}

	left_swap_rn(n);//Shift by 2

	for(i=0;i<16;i++)
	{

		 XOR8n(temp[i],temp[i],state_bs[i], n);
	}

	left_swap_rn(n);//Shift by 3

	for(i=0;i<16;i++)
	{
		 XOR8n(temp[i],temp[i],state_bs[i], n);
	}

	 for(i=0;i<16;i++)
				MOV8n(state_bs[i],temp[i], n);

}


void bs_sbox_share8n(byte n)
{
	byte Y[22][shares_N];
	byte T[68][shares_N];
	byte Z[18][shares_N];
	byte S[8][shares_N];
	byte X[8][shares_N];


	MOV8n( X[0],state_bs[0], n);//compute S-box circuit on even bytes first
    MOV8n( X[1],state_bs[2], n);
    MOV8n( X[2],state_bs[4], n);
    MOV8n( X[3],state_bs[6], n);
    MOV8n( X[4],state_bs[8], n);
    MOV8n( X[5],state_bs[10], n);
    MOV8n( X[6],state_bs[12], n);
    MOV8n( X[7],state_bs[14], n);

    //top linear
    XOR8n(Y[14] , X[3] , X[5], n);
    XOR8n(Y[13] , X[0] , X[6], n);
    XOR8n(Y[12] , Y[13] , Y[14], n);
    XOR8n(Y[9] , X[0] , X[3], n);
    XOR8n(Y[8] , X[0] , X[5], n);
    XOR8n(T[0] , X[1] , X[2], n);
    XOR8n(Y[1] , T[0] , X[7], n);
    XOR8n(Y[4] , Y[1] , X[3], n);
    XOR8n(Y[2] , Y[1] , X[0], n);
    XOR8n(Y[5] , Y[1] , X[6], n);
    XOR8n(T[1] , X[4] , Y[12], n);
    XOR8n(Y[3] , Y[5] , Y[8], n);
    XOR8n(Y[15] , T[1] , X[5], n);
    XOR8n(Y[20] , T[1] , X[1], n);
    XOR8n(Y[6] , Y[15] , X[7], n);
    XOR8n(Y[10] , Y[15] , T[0], n);
    XOR8n(Y[11] , Y[20] , Y[9], n);
    XOR8n(Y[7] , X[7] , Y[11], n);
    XOR8n(Y[17] , Y[10] , Y[11], n);
    XOR8n(Y[19] , Y[10] , Y[8], n);
    XOR8n(Y[16] , T[0] , Y[11], n);
    XOR8n(Y[21] , Y[13] , Y[16], n);
    XOR8n(Y[18] , X[0] , Y[16], n);


    //middle non-linear

    AND8n(T[2] , Y[12] , Y[15], n);
    AND8n(T[3] , Y[3] , Y[6], n);
    AND8n(T[5] , Y[4] , X[7] , n);
    AND8n(T[7] , Y[13] , Y[16], n);
    AND8n(T[8] , Y[5] , Y[1] , n);
    AND8n(T[10] , Y[2] , Y[7] , n);
    AND8n(T[12] , Y[9] , Y[11] , n);
    AND8n(T[13] , Y[14] , Y[17], n);
    XOR8n(T[4] , T[3] , T[2] , n);
    XOR8n(T[6] , T[5] , T[2], n);
    XOR8n(T[9] , T[8] , T[7], n);
    XOR8n(T[11] , T[10] , T[7], n);
    XOR8n(T[14] , T[13] , T[12], n);
    XOR8n(T[17] , T[4] , T[14], n);
    XOR8n(T[19] , T[9] , T[14] , n);
    XOR8n(T[21] , T[17] , Y[20], n);

    XOR8n(T[23] , T[19] , Y[21] , n);
    AND8n(T[15] , Y[8] , Y[10], n);
    AND8n(T[26] , T[21] , T[23], n);
    XOR8n(T[16] , T[15] , T[12], n);
    XOR8n(T[18] , T[6] , T[16], n);
    XOR8n(T[20] , T[11] , T[16], n);
    XOR8n(T[24] , T[20] , Y[18], n);
    XOR8n(T[30] , T[23] , T[24], n);
    XOR8n(T[22] , T[18] , Y[19], n);
    XOR8n(T[25] , T[21] , T[22], n);
    XOR8n(T[27] , T[24] , T[26], n);
    XOR8n(T[31] , T[22] , T[26], n);
    AND8n(T[28] , T[25] , T[27], n);
    AND8n(T[32] , T[31] , T[30], n);
    XOR8n(T[29] , T[28] , T[22], n);
    XOR8n(T[33] , T[32] , T[24], n);
    XOR8n(T[34] , T[23] , T[33], n);
    XOR8n(T[35] , T[27] , T[33] , n);
    XOR8n(T[42] , T[29] , T[33], n);
    AND8n(Z[14] , T[29] , Y[2], n);
    AND8n(T[36] , T[24] , T[35], n);
    XOR8n(T[37] , T[36] , T[34], n);
    XOR8n(T[38] , T[27] , T[36], n);
    AND8n(T[39] , T[29] , T[38], n);
    AND8n(Z[5] , T[29] , Y[7], n);


    XOR8n(T[44] , T[33] , T[37] , n);
    XOR8n(T[40] , T[25] , T[39], n);
    XOR8n(T[41] , T[40] , T[37], n);
    XOR8n(T[43] , T[29] , T[40], n);
    XOR8n(T[45] , T[42] , T[41], n);
    AND8n(Z[0] , T[44] , Y[15], n);
    AND8n(Z[1] , T[37] , Y[6], n);

    AND8n(Z[2] , T[33] , X[7], n);
    AND8n(Z[3] , T[43] , Y[16], n);
    AND8n(Z[4] , T[40] , Y[1], n);
    AND8n(Z[6] , T[42] , Y[11], n);
    AND8n(Z[7] , T[45] , Y[17], n);
    AND8n(Z[8] , T[41] , Y[10], n);
    AND8n(Z[9] , T[44] , Y[12], n);
    AND8n(Z[10] , T[37] , Y[3], n);
    AND8n(Z[11] , T[33] , Y[4], n);
    AND8n(Z[12] , T[43] , Y[13], n);
    AND8n(Z[13] , T[40] , Y[5], n);
    AND8n(Z[15] , T[42] , Y[9], n);
    AND8n(Z[16] , T[45] , Y[14], n);
    AND8n(Z[17] , T[41] , Y[8], n);

    //bottom linear
    XOR8n(T[46] , Z[15] , Z[16], n);
    XOR8n(T[55] , Z[16] , Z[17] , n);
    XOR8n(T[52] , Z[7] , Z[8], n);
    XOR8n(T[54] , Z[6] , Z[7], n);
    XOR8n(T[58] , Z[4] , T[46], n);
    XOR8n(T[59] , Z[3] , T[54] , n);
    XOR8n(T[64] , Z[4] , T[59], n);
    XOR8n(T[47] , Z[10] , Z[11], n);

    XOR8n(T[49] , Z[9] , Z[10], n);
    XOR8n(T[63] , T[49] , T[58] , n);
    XOR8n(T[66] , Z[1], T[63], n);
    XOR8n(T[62] , T[52] , T[58], n);
    XOR8n(T[53] , Z[0] , Z[3], n);
    XOR8n(T[50] , Z[2] , Z[12] , n);
    XOR8n(T[57] , T[50] , T[53], n);
    XOR8n(T[60] , T[46] , T[57] , n);

    XOR8n(T[61] , Z[14] , T[57], n);
    XOR8n(T[65] , T[61] , T[62] , n);
    XOR8n(S[0] , T[59] , T[63], n);
    XOR8n(T[51] , Z[2] , Z[5] , n);
    XOR8n(S[4] , T[51] , T[66], n);
    XOR8n(S[5] , T[47] , T[65] , n);
    XOR8n(T[67] , T[64] , T[65], n);

    XOR8n(S[2] , T[55] , T[67], n);

    NOT8n(S[2]);

    XOR8n(T[48] , Z[5] , Z[13], n);
    XOR8n(T[56] , Z[12] , T[48], n);
    XOR8n(S[3] , T[53] , T[66], n);
    XOR8n(S[1] , T[64] , S[3], n);

    NOT8n(S[1]);
    XOR8n(S[6] , T[56], T[62], n);
    NOT8n(S[6]);
    XOR8n(S[7] , T[48], T[60], n);
    NOT8n(S[7]);

    MOV8n(state_bs[0] ,S[0], n);
    MOV8n(state_bs[2] , S[1], n);
    MOV8n(state_bs[4] , S[2], n);
    MOV8n(state_bs[6] , S[3], n);
    MOV8n(state_bs[8] , S[4], n);
    MOV8n(state_bs[10] , S[5], n);
    MOV8n(state_bs[12] , S[6], n);
    MOV8n(state_bs[14] , S[7], n);


	MOV8n(X[0],state_bs[1], n);//compute S-box circuit on odd bytes next
	MOV8n(X[1],state_bs[3], n);
	MOV8n(X[2],state_bs[5], n);
	MOV8n(X[3],state_bs[7], n);
	MOV8n(X[4],state_bs[9], n);
	MOV8n(X[5],state_bs[11], n);
	MOV8n(X[6],state_bs[13], n);
	MOV8n(X[7],state_bs[15], n);


    //top linear
    XOR8n(Y[14] , X[3] , X[5], n);
    XOR8n(Y[13] , X[0] , X[6], n);
    XOR8n(Y[12] , Y[13] , Y[14], n);
    XOR8n(Y[9] , X[0] , X[3], n);
    XOR8n(Y[8] , X[0] , X[5], n);
    XOR8n(T[0] , X[1] , X[2], n);
    XOR8n(Y[1] , T[0] , X[7], n);
    XOR8n(Y[4] , Y[1] , X[3], n);
    XOR8n(Y[2] , Y[1] , X[0], n);
    XOR8n(Y[5] , Y[1] , X[6], n);
    XOR8n(T[1] , X[4] , Y[12], n);
    XOR8n(Y[3] , Y[5] , Y[8], n);
    XOR8n(Y[15] , T[1] , X[5], n);
    XOR8n(Y[20] , T[1] , X[1], n);
    XOR8n(Y[6] , Y[15] , X[7], n);
    XOR8n(Y[10] , Y[15] , T[0], n);
    XOR8n(Y[11] , Y[20] , Y[9], n);
    XOR8n(Y[7] , X[7] , Y[11], n);
    XOR8n(Y[17] , Y[10] , Y[11], n);
    XOR8n(Y[19] , Y[10] , Y[8], n);
    XOR8n(Y[16] , T[0] , Y[11], n);
    XOR8n(Y[21] , Y[13] , Y[16], n);
    XOR8n(Y[18] , X[0] , Y[16], n);


    //middle non-linear

    AND8n(T[2] , Y[12] , Y[15], n);
    AND8n(T[3] , Y[3] , Y[6], n);
    AND8n(T[5] , Y[4] , X[7] , n);
    AND8n(T[7] , Y[13] , Y[16], n);
    AND8n(T[8] , Y[5] , Y[1] , n);
    AND8n(T[10] , Y[2] , Y[7] , n);
    AND8n(T[12] , Y[9] , Y[11] , n);
    AND8n(T[13] , Y[14] , Y[17], n);
    XOR8n(T[4] , T[3] , T[2] , n);
    XOR8n(T[6] , T[5] , T[2], n);
    XOR8n(T[9] , T[8] , T[7], n);
    XOR8n(T[11] , T[10] , T[7], n);
    XOR8n(T[14] , T[13] , T[12], n);
    XOR8n(T[17] , T[4] , T[14], n);
    XOR8n(T[19] , T[9] , T[14] , n);
    XOR8n(T[21] , T[17] , Y[20], n);

    XOR8n(T[23] , T[19] , Y[21] , n);
    AND8n(T[15] , Y[8] , Y[10], n);
    AND8n(T[26] , T[21] , T[23], n);
    XOR8n(T[16] , T[15] , T[12], n);
    XOR8n(T[18] , T[6] , T[16], n);
    XOR8n(T[20] , T[11] , T[16], n);
    XOR8n(T[24] , T[20] , Y[18], n);
    XOR8n(T[30] , T[23] , T[24], n);
    XOR8n(T[22] , T[18] , Y[19], n);
    XOR8n(T[25] , T[21] , T[22], n);
    XOR8n(T[27] , T[24] , T[26], n);
    XOR8n(T[31] , T[22] , T[26], n);
    AND8n(T[28] , T[25] , T[27], n);
    AND8n(T[32] , T[31] , T[30], n);
    XOR8n(T[29] , T[28] , T[22], n);
    XOR8n(T[33] , T[32] , T[24], n);
    XOR8n(T[34] , T[23] , T[33], n);
    XOR8n(T[35] , T[27] , T[33] , n);
    XOR8n(T[42] , T[29] , T[33], n);
    AND8n(Z[14] , T[29] , Y[2], n);
    AND8n(T[36] , T[24] , T[35], n);
    XOR8n(T[37] , T[36] , T[34], n);
    XOR8n(T[38] , T[27] , T[36], n);
    AND8n(T[39] , T[29] , T[38], n);
    AND8n(Z[5] , T[29] , Y[7], n);


    XOR8n(T[44] , T[33] , T[37] , n);
    XOR8n(T[40] , T[25] , T[39], n);
    XOR8n(T[41] , T[40] , T[37], n);
    XOR8n(T[43] , T[29] , T[40], n);
    XOR8n(T[45] , T[42] , T[41], n);
    AND8n(Z[0] , T[44] , Y[15], n);
    AND8n(Z[1] , T[37] , Y[6], n);

    AND8n(Z[2] , T[33] , X[7], n);
    AND8n(Z[3] , T[43] , Y[16], n);
    AND8n(Z[4] , T[40] , Y[1], n);
    AND8n(Z[6] , T[42] , Y[11], n);
    AND8n(Z[7] , T[45] , Y[17], n);
    AND8n(Z[8] , T[41] , Y[10], n);
    AND8n(Z[9] , T[44] , Y[12], n);
    AND8n(Z[10] , T[37] , Y[3], n);
    AND8n(Z[11] , T[33] , Y[4], n);
    AND8n(Z[12] , T[43] , Y[13], n);
    AND8n(Z[13] , T[40] , Y[5], n);
    AND8n(Z[15] , T[42] , Y[9], n);
    AND8n(Z[16] , T[45] , Y[14], n);
    AND8n(Z[17] , T[41] , Y[8], n);

    //bottom linear
    XOR8n(T[46] , Z[15] , Z[16], n);
    XOR8n(T[55] , Z[16] , Z[17] , n);
    XOR8n(T[52] , Z[7] , Z[8], n);
    XOR8n(T[54] , Z[6] , Z[7], n);
    XOR8n(T[58] , Z[4] , T[46], n);
    XOR8n(T[59] , Z[3] , T[54] , n);
    XOR8n(T[64] , Z[4] , T[59], n);
    XOR8n(T[47] , Z[10] , Z[11] , n);

    XOR8n(T[49] , Z[9] , Z[10], n);
    XOR8n(T[63] , T[49] , T[58] , n);
    XOR8n(T[66] , Z[1], T[63], n);
    XOR8n(T[62] , T[52] , T[58], n);
    XOR8n(T[53] , Z[0] , Z[3], n);
    XOR8n(T[50] , Z[2] , Z[12] , n);
    XOR8n(T[57] , T[50] , T[53], n);
    XOR8n(T[60] , T[46] , T[57] , n);

    XOR8n(T[61] , Z[14] , T[57], n);
    XOR8n(T[65] , T[61] , T[62] , n);
    XOR8n(S[0] , T[59] , T[63], n);
    XOR8n(T[51] , Z[2] , Z[5] , n);
    XOR8n(S[4] , T[51] , T[66], n);
    XOR8n(S[5] , T[47] , T[65] , n);
    XOR8n(T[67] , T[64] , T[65], n);

    XOR8n(S[2] , T[55] , T[67], n);

    NOT8n(S[2]);

    XOR8n(T[48] , Z[5] , Z[13], n);
    XOR8n(T[56] , Z[12] , T[48], n);
    XOR8n(S[3] , T[53] , T[66], n);
    XOR8n(S[1] , T[64] , S[3], n);

    NOT8n(S[1]);
    XOR8n(S[6] , T[56], T[62], n);
    NOT8n(S[6]);
    XOR8n(S[7] , T[48], T[60], n);
    NOT8n(S[7]);


    MOV8n(state_bs[1] , S[0], n);
    MOV8n(state_bs[3] , S[1], n);
    MOV8n(state_bs[5] , S[2], n);
    MOV8n(state_bs[7] , S[3], n);
    MOV8n(state_bs[9] , S[4], n);
    MOV8n(state_bs[11] , S[5], n);
    MOV8n(state_bs[13] , S[6], n);
    MOV8n(state_bs[15] , S[7], n);

}


//****************************encodings******************

void encode_bskey_share8n(byte n) // To encode AES key into bitslice format
{

    byte i,k,t,j=7,l=0,row,m;
    byte num=16;//Number of values in target bitslice
    for(m=0;m<n;m++)
    {
        for(row=0;row<11;row++)
        {
            l=7;
            for(k=0;k<8;k++)
            {
                wshare_bs[num*row+2*k][m]=0;
                wshare_bs[num*row+2*k+1][m]=0;
                j=7;

                for(i=0;i<8;i++)
                {

                    t=(1 & (wshare1[num*row+2*i][m] >> l));
                    wshare_bs[num*row+2*k][m]=wshare_bs[num*row+2*k][m]+t*pow_cust(2,j);

                    t=(1 & (wshare1[num*row+2*i+1][m] >> l));
                    wshare_bs[num*row+2*k+1][m]=wshare_bs[num*row+2*k+1][m]+t*pow_cust(2,j);

                    j--;
                }

                l--;
            }

        }
    }
}




void encode_share8n(byte n) //Pack even and odd bits in different arrays. To encode shared-input to AES into bitslice format.
{
    int i,k,t,j=Tn-1,l=Sn-1,m;

    for(m=0;m<n;m++)
    {
        l=Sn-1;
        for(k=0;k<8;k++)
        {
           state_bs[2*k][m]=0;
           state_bs[2*k+1][m]=0;
           j=Tn-1;

            for(i=0;i<8;i++)
            {
                t=(1 & (stateshare[2*i][m] >> l));
                state_bs[2*k][m]=state_bs[2*k][m]+t*pow_cust(2,j);

                t=(1 & (stateshare[2*i+1][m] >> l));
                state_bs[2*k+1][m]=state_bs[2*k+1][m]+t*pow_cust(2,j);

                j--;
            }

            l--;
        }
    }
}



void decode_share8n(byte n) //To convert back the bitsliced input to bytes.
{
    byte i,k,t,pos=Tn-1,l=Sn-1,m;
    for(m=0;m<n;m++)
    {
        pos=Tn-1;
        for(i=0;i<8;i++)
        {

            stateshare[2*i][m]=0;
            stateshare[2*i+1][m]=0;
            l=Sn-1;
            for(k=0;k<8;k++)
            {

                t=(1&(state_bs[2*k][m]>>pos));
                stateshare[2*i][m]=stateshare[2*i][m]+t*pow_cust(2,l);

                t=(1&(state_bs[2*k+1][m]>>pos));
                stateshare[2*i+1][m]=stateshare[2*i+1][m]+t*pow_cust(2,l);

                l--;
            }
            pos--;
        }
    }
}




//********************Share 8-bit bitslice****************************


void aes_share_subkeys_bitslice8(byte in[16],byte out[16],byte n) // AES block cipher on bitslice encoded data.
{
  byte i;

  for(i=0;i<16;i++)
  {
    share_rnga(in[i],stateshare[i],n);
    //refresh(stateshare[i],n);
  }
    encode_share8n(n);
    bs_addroundkey_share8n(0,n);

	// AES Round Transformations (x 9)
	//------------------------------------------------------
	for(i=1; i<10; i++)
	{
      bs_sbox_share8n(n);//(state_bs,n);
      bs_shiftrows_share8n(n);
    	bs_mixcol_share8n(n);
	    bs_addroundkey_share8n(i,n);
    }

	// AES Last Round
	//------------------------------------------------------
	bs_sbox_share8n(n);
	bs_shiftrows_share8n(n);
	bs_addroundkey_share8n(10,n);

	decode_share8n(n);

    for(i=0;i<16;i++)
    {
        out[i]=decode(stateshare[i],n);
     }

}

double run_aes_share_bitslice8(byte in[16],byte out[16],byte key[16],byte n,int nt) // Driver function for bitslice
{

	byte i;
	double time_bs=0.0,temp=0.0;

	#if TRNG==0
	struct timespec begin, end;
	#endif

	unsigned int begin1,end1;

	long sec,nsec;

	keyexpansion(key,w);

	for(i=0;i<176;i++)
	{
		share_rnga(w[i],wshare1[i],n);
	}

	encode_bskey_share8n(n);

    #if TRNG==0
    clock_gettime(CLOCK_REALTIME, &begin);
    #endif // TRNG

    #if TRNG>0
    reset_systick();
    begin1 = SysTick->VAL; // Obtains the start time
    #endif // TRNG


	for(i=0;i<nt;i++)
	{
		aes_share_subkeys_bitslice8(in,out,n);
	}


    #if TRNG==0
    clock_gettime(CLOCK_REALTIME, &end);
    sec = end.tv_sec - begin.tv_sec;
    nsec = end.tv_nsec - begin.tv_nsec;
    temp = sec + nsec*1e-9;

    time_bs = temp*UNIT/nt;
    #endif // TRNG

    #if TRNG==1
    end1 = SysTick->VAL; // Obtains the stop time
    time_bs = ((double)(begin1-end1))/nt; // Calculates the time taken
    #endif // TRNG


  return time_bs; //The time required for bitslice execution

}

