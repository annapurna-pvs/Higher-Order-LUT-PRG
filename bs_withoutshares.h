#include "aes.h"

unsigned int leftRotate(int n, unsigned int d);
void bs_addroundkey_w(unsigned int state[8], unsigned int bs_key[88],int round);
unsigned int swapBits(unsigned int n, int p1, int p2);
void bitslice(unsigned int *X);
void bs_sbox_w(unsigned int *X);
void bs_shiftrows_c1(unsigned int *X);
void mixcol_c9(unsigned int state[8]);
void mixcol_c1(unsigned int state[8]);
void encode_168_816(byte *bit_array,unsigned int *arr_b);
void decode_8168_168(byte *bit_array,unsigned int *arr_b);
void encode_bskey_w(byte *bit_array,unsigned int *arr_b);
void encode_bs_w(byte *bit_array,unsigned int *arr_b);
void decode_bs_w(byte *bit_array,unsigned int *arr_b);
void bitslice(unsigned int *X);
