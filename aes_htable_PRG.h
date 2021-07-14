#include "aes.h"

#define NPRG 1 // Normal variant// with multi PRG
#define IPRG 0 // Increasing shares variant //with multiple PRG
#define RPRG 2
#define MPRG 3
#define N2 11

//byte N=10;

//typedef unsigned int word;

void gen_t_forall(int n,int choice,int type);//void gen_t_forall(void (*table_call) (int, int,int),int n,int choice);
void gen_t_all(int n,int choice,int type,int tab);

void htable_r(int n,int count,int choice);
void htable_m(int n,int count,int choice);

void locality_refresh_r(byte *a,int n);
void locality_refresh_m(byte *a,int n);

void subbyte_htable_r(byte *a,int n,int count);
void subbyte_htable_r_inc(byte *a,int n,int count);
void subbyte_htable_mprg(byte *a,int n,int count);
void subbyte_htable_inc_mprg(byte *a,int n,int count);

void mixcolumns_share_loc_table_prg(byte stateshare[16][N2],int n);
void mixcolumns_share_loc_table_mprg(byte stateshare[16][N2],int n);

void subbytestate_share1(byte stateshare[16][N2],int n,void (*subbyte_share_call)(byte *,int,int),int round);


