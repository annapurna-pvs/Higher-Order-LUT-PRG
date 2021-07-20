#include <stdio.h>

#include "aes.h"
#include "share.h"
#include "aes_htable_prg.h"
#include "prg3.h"
#include "rand_k64.h"
#include "aes_rp.h"

byte wshare[176][N];

void share_rnga(byte x,byte a[],int n) //Additive secret sharing
{
		int i;
		gen_rand(a,n-1);
		a[n-1]=x;

		for(i=0;i<n-1;i++)
			a[n-1]=a[n-1] ^ a[i];

}

//*************************Code from Coron's github************

void keyexpansion_share(byte key[16],int n)
{
  byte w[176];
  keyexpansion(key,w);

  for(int i=0;i<176;i++)
  {
    share_rnga(w[i],wshare[i],n);
  }

}

void addroundkey_share(byte stateshare[16][N],int round,int n)
{
  int i,j;
  for(i=0;i<16;i++)
    for(j=0;j<n;j++)
      stateshare[i][j]^=wshare[16*round+i][j];
}



void shiftrows_share(byte stateshare[16][N],int n)
{
  byte m;
  int i;
  for(i=0;i<n;i++)
  {
    m=stateshare[1][i];
    stateshare[1][i]=stateshare[5][i];
    stateshare[5][i]=stateshare[9][i];
    stateshare[9][i]=stateshare[13][i];
    stateshare[13][i]=m;

    m=stateshare[2][i];
    stateshare[2][i]=stateshare[10][i];
    stateshare[10][i]=m;
    m=stateshare[6][i];
    stateshare[6][i]=stateshare[14][i];
    stateshare[14][i]=m;

    m=stateshare[3][i];
    stateshare[3][i]=stateshare[15][i];
    stateshare[15][i]=stateshare[11][i];
    stateshare[11][i]=stateshare[7][i];
    stateshare[7][i]=m;
  }
}



void mixcolumns_share(byte stateshare[16][N],int n)
{
  byte ns[16];
  int i,j;
  for(i=0;i<n;i++)
  {
    for(j=0;j<4;j++)
    {
      ns[j*4]=multx(stateshare[j*4][i]) ^ multx(stateshare[j*4+1][i]) ^ stateshare[j*4+1][i] ^ stateshare[j*4+2][i] ^ stateshare[j*4+3][i];
      ns[j*4+1]=stateshare[j*4][i] ^ multx(stateshare[j*4+1][i]) ^ multx(stateshare[j*4+2][i]) ^ stateshare[j*4+2][i] ^ stateshare[j*4+3][i];
      ns[j*4+2]=stateshare[j*4][i] ^ stateshare[j*4+1][i] ^ multx(stateshare[j*4+2][i]) ^ multx(stateshare[j*4+3][i]) ^ stateshare[j*4+3][i];
      ns[j*4+3]=multx(stateshare[j*4][i]) ^ stateshare[j*4][i] ^ stateshare[j*4+1][i] ^ stateshare[j*4+2][i] ^ multx(stateshare[j*4+3][i]) ;
    }
    for(j=0;j<16;j++)
      stateshare[j][i]=ns[j];
  }
}




void aes_share_subkeys(byte in[16],byte out[16],int n,void (*subbyte_share_call)(byte *,int,int))
{
  int i;
  int round=0;

  byte stateshare[16][N];

  for(i=0;i<16;i++)
  {
    share_rnga(in[i],stateshare[i],n);
  }

  addroundkey_share(stateshare,0,n);

  for(round=1;round<10;round++)
  {
    subbytestate_share_prg(stateshare,n,subbyte_share_call,round-1);
    shiftrows_share(stateshare,n);
    mixcolumns_share(stateshare,n);
    addroundkey_share(stateshare,round,n);
  }

  subbytestate_share_prg(stateshare,n,subbyte_share_call,round-1);
  shiftrows_share(stateshare,n);
  addroundkey_share(stateshare,10,n);

  for(i=0;i<16;i++)
  {
    out[i]=decode(stateshare[i],n);
    //free(stateshare[i]);
  }
}


//**************************** AES with shares using robust PRG**************

void run_aes_share_rprg_table(byte in[16],byte out[16],byte key[16],byte *outex,int n,void (*subbyte_share_call)(byte *,int,int),int nt,int rprg)
{
  //int prgcount,i;
  int i;
  keyexpansion_share(key,n);

  for(i=0;i<nt;i++)
  {
    aes_share_subkeys(in,out,n,subbyte_share_call);
  }

}


//****************** AES share for multiple PRG ****************************

void run_aes_share_mprg_table(byte in[16],byte out[16],byte key[16],byte *outex,int n,void (*subbyte_share_call)(byte *,int,int),int nt,int ni)
{
  keyexpansion_share(key,n);

  for(int i=0;i<nt;i++)
    aes_share_subkeys(in,out,n,subbyte_share_call);

}

int rprg_AES(int n)
{
	int loc=1;
	loc=2*(n-1);
	return (n-1)*loc;
}

//*******************main*****************************************

int main_aes()
{
		int nt=10;
		byte n=N; // Number of input shares
		int ni=0;
		int i,k;

		//****************Test vectors********************

		byte keyex[16]={0x2b,0x7e,0x15,0x16,0x28,0xae,0xd2,0xa6,0xab,0xf7,0x15,0x88,0x09,0xcf,0x4f,0x3c};
		byte inex[16]={0x32,0x43,0xf6,0xa8,0x88,0x5a,0x30,0x8d,0x31,0x31,0x98,0xa2,0xe0,0x37,0x07,0x34};
		byte outex[16]={0x39,0x25,0x84,0x1d,0x02,0xdc,0x09,0xfb,0xdc,0x11,0x85,0x97,0x19,0x6a,0x0b,0x32};

		byte in[16],out[16],out1[16];
		byte key[16];
		int choice=NPRG; //Type of LUT construction. normal--> NPRG  Increasing shares--> IPRG
		int type=MPRG; //Type of PRG to generate randoms, either robust-->RPRG or multiple-->MPRG

		for(i=0;i<16;i++)
            key[i]=keyex[i];

		for(i=0;i<16;i++)
            in[i]=inex[i];//rand()%256;

		for(k=0;k<16;k++)
        {
                out[k]=0x0;
                out1[k]=0x0;
        }

		//TRNG initialisation
		//rand_in();


//*******************Un masked AES***************************
		run_aes(in,out,key,nt);
		//runaes(in,out,key,outex,nt,base);

//**********Higher-order AES with pre-processing using robustPRG T contains pre-processed tables

        if(type==RPRG)
        {
            int rprg=rprg_AES(n);
            printf("Inside main ...higher-order PRG with robust prg %d!!!",rprg);
            init_robprg3(rprg,n);

            printf("Pre-computation of 160 tables for AES-128");
            gen_t_forall(n,choice,type); //Pre-processing table T1 for all rounds

            printf("\n \n Online Phase\n\n\n");

            if(choice==NPRG)
            {
                /*robust PRG with normal variant*/
                run_aes_share_rprg_table(in,out1,key,outex,n,&subbyte_htable_r,nt,rprg);

            }

            if(choice==IPRG)
            {
                /*robust PRG with increasing shares variant*/
                run_aes_share_rprg_table(in,out1,key,outex,n,&subbyte_htable_r_inc,nt,rprg);

            }


            //printf("Total random bytes used are: %d\n",get_robprgcount3());

            free_robprg3(rprg,n);

        }

//**********************Multiple PRG variant**********************************

        if(type==MPRG)
        {

        printf("Inside MPRG\n");
            if(choice==NPRG)//Choice 1 for normal PRG 0 for Increasing shares
                ni=(n-1)*(n-1);
            else
                ni=n*(n-1)/2;


            init_mprg2(n,ni);
            printf("Pre-computation of 160 tables for AES-128\n");
            gen_t_forall(n,choice,type); //Pre-processing table T1 for all rounds

            printf("Online Phase\n\n\n");

            if(choice==NPRG)
            {
                /*multiple PRG with normal variant*/
                run_aes_share_mprg_table(in,out1,key,outex,n,&subbyte_htable_mprg,nt,ni);

            }

            if(choice==IPRG)
            {

                /*multi PRG with increasing shares variant*/
                run_aes_share_mprg_table(in,out1,key,outex,n,&subbyte_htable_inc_mprg,nt,ni);
            }

            //printf("Total random bytes used are: %d",get_robprgcount3());
            free_mprg2(n,ni);
        }

        //Check for correctness
        for(i=0;i<16;i++)
        {
            if(out[i]!=out1[i])
                break;
        }


        if(i==16)
            printf("Output of unmasked AES matches output of masked LUT using PRG!!!\n");

        else
            printf("Outputs mismatch :(\n");

        //rand_dein();
		return 0;
}

