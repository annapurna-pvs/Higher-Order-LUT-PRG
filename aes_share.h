#ifndef __aes_share_h__
#define __aes_share_h__

#include "aes.h"

void shiftrows_share(byte *stateshare[16],int n);
void mixcolumns_share(byte *stateshare[16],int n);
void addroundkey_share(byte *stateshare[16],byte *wshare[176],int round,int n);

int run_aes_share_compress_t1_once(byte in[16],byte out[16],byte key[16],int n,int l,byte* t1,byte* r,byte* y1,void (*subbyte_share_call_compress)(byte *,byte,byte,byte *,byte *,byte*),int nt);
int run_aes_share_compress_t1_all(byte in[16],byte out[16],byte key[16],int n,int l,byte *t1,byte *r,byte* y1,byte *x_all,void (*subbyte_share_call_compress)(byte *,byte,byte,byte *,byte *,byte *,byte *,int),int nt);

/**********functions renamed for second-order compression******/

void addroundkey_share1(byte stateshare[16][3],byte wshare[176][3],int round,int n);
void shiftrows_share1(byte stateshare[16][3],int n);
void mixcolumns_share1(byte stateshare[16][3],int n);


#endif
