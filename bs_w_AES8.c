#include <stdint.h>
#include "bs_withoutshares.h"
#include "aes.h"

const byte S1=8; //number of bits in input
const byte T1=8;//number of bits of register in use


/************8-bit Encodings**********************/

void encode_w8(byte *bit_array,byte *arr_b){ //Pack even and odd bits in different arrays
int i,k,t,j=T1-1,l=S1-1;

    for(k=0;k<8;k++){
           arr_b[2*k]=0;
           arr_b[2*k+1]=0;
           j=T1-1;

            for(i=0;i<8;i++){
                t=(1 & (bit_array[2*i] >> l));
                arr_b[2*k]=arr_b[2*k]+t*pow(2,j);

                t=(1 & (bit_array[2*i+1] >> l));
                arr_b[2*k+1]=arr_b[2*k+1]+t*pow(2,j);

                j--;
            }

            l--;
        }
}




void decode_w8(byte *bit_array,byte *arr_b){
int i,k,t,j=0,pos=T1-1,l=S1-1;

    for(i=0;i<8;i++){
        bit_array[2*i]=0;
        bit_array[2*i+1]=0;
        l=S1-1;
        for(k=0;k<8;k++){

        t=(1&(arr_b[2*k]>>pos));
        bit_array[2*i]=bit_array[2*i]+t*pow(2,l);

        t=(1&(arr_b[2*k+1]>>pos));
        bit_array[2*i+1]=bit_array[2*i+1]+t*pow(2,l);

        l--;
    }
    pos--;
    }
}


void encode_bskey_w8(byte *bit_array,byte *arr_b)
{

int i,k,t,j=7,l=0,row;
byte num=16;//Number of values in target bitslice
for(row=0;row<11;row++)
	{
	l=7;
    for(k=0;k<8;k++){
           arr_b[num*row+2*k]=0;
           arr_b[num*row+2*k+1]=0;
           j=7;

            for(i=0;i<8;i++){

                t=(1 & (bit_array[num*row+2*i] >> l));
                arr_b[num*row+2*k]=arr_b[num*row+2*k]+t*pow(2,j);

                t=(1 & (bit_array[num*row+2*i+1] >> l));
                arr_b[num*row+2*k+1]=arr_b[num*row+2*k+1]+t*pow(2,j);

                j--;
            }

            l--;
        }

	}
}


/*******************Primitive functions**************/
void swap8(byte *a,byte *b)
{
  byte m=*a;
  *a=*b;
  *b=m;
}

byte swapBits8(byte n, byte k1, byte k2)
{
    byte b1 =  (n >> k1) & 1;
    byte b2 =  (n >> k2) & 1;
    byte x = (b1 ^ b2);

    x = (x << k1) | (x << k2);
    byte result = (n ^ x)&(0xFF);//the mask depends on the number of bits in target representation
	return result;
}



void left_swap_rotate8(byte *state)
{
	byte temp,temp1,a;
	byte i;

	for(i=0;i<8;i++)
	{
		temp1=state[2*i];//odd bits
		temp=state[2*i+1];//even bits

		temp1=swapBits8(temp1,6,7);
		temp1=swapBits8(temp1,5,4);
		temp1=swapBits8(temp1,3,2);
		temp1=swapBits8(temp1,1,0);

		swap8(&temp,&temp1);

        state[2*i]=temp1;
        state[2*i+1]=temp;
	}
}

/*************Round functions***************/


void bs_addroundkey_w8(byte state[16],byte bs_key[176],byte round)
{
	int i;
	for(i=0; i<16; i++)
		state[i]=state[i]^bs_key[round*16+i];
}


void bs_shiftrows_w8(byte *X)
{
	byte i;
	for(i=0;i<8;i++)
	{//for each bit in a byte

        X[2*i+1]=swapBits8(X[2*i+1],1,7);//first
		X[2*i+1]=swapBits8(X[2*i+1],3,5);//first
		X[2*i+1]=swapBits8(X[2*i+1],7,3);//first

		X[2*i+1]=swapBits8(X[2*i+1],4,6);
		X[2*i+1]=swapBits8(X[2*i+1],0,2);
		X[2*i+1]=swapBits8(X[2*i+1],6,2);

	    X[2*i]=swapBits8(X[2*i],6,2);//second
		X[2*i]=swapBits8(X[2*i],4,0);//second*/

    }

}


void mixcol_w8(byte *state)
{
    byte temp[16],temp0,temp1,a;
	byte of_odd,of_even,i,t1;


	temp0=state[0];//odd
	temp1=state[1];//even
	temp0=swapBits8(temp0,6,7);
    temp0=swapBits8(temp0,4,5);
    temp0=swapBits8(temp0,2,3);
    temp0=swapBits8(temp0,0,1);

	swap8(&temp0,&temp1);

	of_odd= state[0]^temp0;
	of_even= state[1]^temp1;

	for(i=0;i<7;i++)
    {
        temp[2*i]=state[2*i+2];
        temp[2*i+1]=state[2*i+3];
    }

	left_swap_rotate8(state);//shift by 1

	for(i=0;i<8;i++)
	{
		temp[2*i]=temp[2*i]^state[((2*i+2)%16)];
        temp[2*i+1]=temp[2*i+1]^state[((2*i+3)%16)];
	}

    temp[6]=temp[6]^of_odd;
    temp[7]=temp[7]^of_even;

    temp[8]=temp[8]^of_odd;
    temp[9]=temp[9]^of_even;

    temp[12]=temp[12]^of_odd;
    temp[13]=temp[13]^of_even;

    temp[14]=of_odd;
    temp[15]=of_even;


	for(i=0;i<16;i++)
	{
        temp[i]=temp[i]^state[i];
	}

	left_swap_rotate8(state);//Shift by 2

	for(i=0;i<16;i++)
	{
		temp[i]=temp[i]^state[i];
	}

	left_swap_rotate8(state);//Shift by 3

	for(i=0;i<16;i++)
	{
		temp[i]=temp[i]^state[i];
	}

	for(i=0;i<16;i++)
        state[i]=temp[i];
}


void bs_sbox_w88(byte *X1)
{
	byte* Y=(byte*) malloc(22*sizeof(byte));
	byte* T=(byte*) malloc(68*sizeof(byte));
	byte* Z=(byte*) malloc(18*sizeof(byte));
	byte* S=(byte*) malloc(8*sizeof(byte));
	byte X[8];

 X[0]=X1[0];//compute S-box circuit on even bytes first
 X[1]=X1[2];
 X[2]=X1[4];
 X[3]=X1[6];
 X[4]=X1[8];
 X[5]=X1[10];
 X[6]=X1[12];
 X[7]=X1[14];

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

S[2] = (T[55] ^ T[67])^(0xFF);

T[48] = Z[5] ^ Z[13];
T[56] = Z[12] ^ T[48];
S[3] = T[53] ^ T[66];
S[1] = (T[64] ^ S[3])^(0xFF);
S[6] = (T[56]^T[62])^(0xFF);
S[7] = (T[48]^ T[60])^(0xFF);

X1[0] = S[0];
X1[2] = S[1];
X1[4] = S[2];
X1[6] = S[3];
X1[8] = S[4];
X1[10] = S[5];
X1[12] = S[6];
X1[14] = S[7];


 X[0]=X1[1];//compute S-box circuit on odd bytes next
 X[1]=X1[3];
 X[2]=X1[5];
 X[3]=X1[7];
 X[4]=X1[9];
 X[5]=X1[11];
 X[6]=X1[13];
 X[7]=X1[15];


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

S[2] = (T[55] ^ T[67])^(0xFF);

T[48] = Z[5] ^ Z[13];
T[56] = Z[12] ^ T[48];
S[3] = T[53] ^ T[66];
S[1] = (T[64] ^ S[3])^(0xFF);
S[6] = (T[56]^T[62])^(0xFF);
S[7] = (T[48]^ T[60])^(0xFF);


X1[1] = S[0];
X1[3] = S[1];
X1[5] = S[2];
X1[7] = S[3];
X1[9] = S[4];
X1[11] = S[5];
X1[13] = S[6];
X1[15] = S[7];


free(Y);
free(T);
free(S);
free(Z);

}
