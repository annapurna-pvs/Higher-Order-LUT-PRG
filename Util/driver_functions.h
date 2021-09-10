#include "../AES/aes.h"

void run_aes_shares_prg(byte *in,byte *out,byte *key,int n,int choice,int type,double *time,int nt);
void run_present_shares_prg(byte *in,byte*out,byte *key,int n,double *time,int nt);

double run_aes_share_bitslice8(byte in[16],byte out[16],byte key[16],byte n,int nt);
double run_present_shares_crv(byte *in,byte*out,byte *key,int n,int nt);
