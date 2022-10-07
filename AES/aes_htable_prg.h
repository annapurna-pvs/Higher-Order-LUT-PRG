#include "../Util/common.h"
#include "../Util/share.h"

void gen_t_forall(int n,int choice,int type);//void gen_t_forall(void (*table_call) (int, int,int),int n,int choice);

void htable_r(int n,int count,int choice);
void htable_m(int n,int count,int choice);

void subbyte_htable_r(byte *a,int n,int count);
void subbyte_htable_r_inc(byte *a,int n,int count);
void subbyte_htable_mprg(byte *a,int n,int count);
void subbyte_htable_inc_mprg(byte *a,int n,int count);

void subbytestate_share_prg(byte stateshare[16][shares_N],int n,void (*subbyte_share_call)(byte *,int,int),int round);
/***********specific to third order**************/

void subbyte_htable_third(byte y[shares_N], int n, int ind, int choice);
void gen_t_forall_third(int n, int choice);
void subbytestate_share_third(byte stateshare[16][shares_N], int n, void(*subbyte_share_call)(byte *, int, int, int), int round, int choice);



