#include <stdio.h>

#include "aes.h"
#include "share.h"
#include "aes_share.h"
#include "aes_htable_prg.h"
#include "prg3.h"
#include "rand_k64.h"
#include "aes_rp.h"

#define TSIZE 256 //16 for PRESENT

//N2 represents the number of shares
byte x_shares[160*(N2-1)]; //Shares of x used as part of pre-processing.  496 *(N2-1) fpr PRESENT
byte T[160*TSIZE]; //Size of pre-computed tables for 10*16=160 S-box calls for AES-128, 496*TSIZE for PRESENT.


//************Functions for Normal variant using robust PRG *******************


//*************off-line functions******

void loc_refresh_table_r(byte *Tp,byte a,int n,int ind,int count)
{
    unsigned int pre_val=count*(n-1)*(n-1)*TSIZE;


    for(int k=0;k<TSIZE;k++)
    {
		for(int i=1;i<n;i++)
		{
            unsigned int val=pre_val+(ind*(n-1)*TSIZE)+(k*(n-1))+(i-1); //randoms needed per shift + per column + per row
            byte tmp=get_robprg3(n,val);
            set_robprgcount3();
						Tp[k]=Tp[k] ^ tmp;
     	}


        if(ind>0)
        {
            for(int i=1;i<n;i++)
            {
                int previous_val=pre_val+((ind-1)*(n-1)*TSIZE)+((k^a)*(n-1))+(i-1);
                byte tmp=get_robprg3(n,previous_val);
                Tp[k]=Tp[k] ^ tmp;
            }
        }

    }
}



void loc_refresh_table_r_inc(byte *Tp,byte a,int n,int ind,int count)
{
    unsigned int pre_val=count*n*(n-1)*TSIZE/2;

    for(int k=0;k<TSIZE;k++)
    {
		for(int i=0;i<=ind;i++)
		{
		    int t= ind * (ind+1)/2;
            int val=pre_val+(t*TSIZE)+(k*(ind+1))+(i); //randoms needed per shift + per column + per row
            byte tmp=get_robprg3(n,val);
			set_robprgcount3();
			Tp[k]=Tp[k] ^ tmp;
     	}

        if(ind>0)
        {
            for(int i=0;i<ind;i++)
            {
                int t= (ind-1) * ind/2;
                int previous_val=pre_val+(t*TSIZE)+((k^a)*(ind))+(i);
                byte tmp=get_robprg3(n,previous_val);
                Tp[k]=Tp[k] ^ tmp;
            }
        }

    }
}


void shift_tab(byte a,byte *Tp,int count)//shift_tab(x_shares[count][i],Tp,count);
{
	unsigned int j,temp,temp1;
   
	for(j=0;j<TSIZE;j++)
	{
		temp=count*TSIZE;
		temp1=j^a;
    Tp[j]=T[temp+temp1];
	}
}


void htable_r(int n,int count,int choice)//htable_r(n,i);//1 for normal 0 for increasing
{
  byte Tp[TSIZE];
  int i,k;

  for(k=0;k<TSIZE;k++)
     T[count*TSIZE+(k)]=sbox[k];

  for(i=0;i<(n-1);i++) // In pre-proessing, T will be shifted by n-1 shares.
  {
    shift_tab(x_shares[count*(N2-1)+i],Tp,count);

    if(choice==NPRG)
        loc_refresh_table_r(Tp,x_shares[count*(N2-1)+i],n,i,count);

    else if(choice==IPRG)
        loc_refresh_table_r_inc(Tp,x_shares[count*(N2-1)+i],n,i,count);


    for(k=0;k <TSIZE;k++)
      T[count*TSIZE+(k)]=Tp[k];

  }
}


//********************* Pre-processing of Table T ***************************


//*********online functions*********


void locality_refresh(byte *a,int n)
{
    byte t=a[0];
	  byte b[n-1];
	  gen_rand(b,n-1);
    for(int i=1;i<n;i++)
    {

        t=t^ b[i-1] ^ a[i];
        a[i]=b[i-1];
    }
  a[0]=t;
}


//**************Normal variant htable**********

void read_htable_r(byte a,byte *b,int n,int count)
{
  int pre_val=count*(n-1)*(n-1)*TSIZE;
	int val=pre_val+((n-2)*(n-1)*TSIZE)+((n-1)*a);

	b[0]=T[count*TSIZE+(a)];

    for(int j=1;j<n;j++)
        b[j]=get_robprg3(n,val+(j-1));

    locality_refresh_r(b,n);
}


void subbyte_htable_r(byte *a,int n,int count)
{
    read_htable_r(a[n-1],a,n,count);
}

//**************Increasing shares*************

void read_htable_inc_r(byte a,byte *b,int n,int count)
{
    unsigned int pre_val=count*n*(n-1)*TSIZE/2;
    unsigned int t= ((n-2) *(n-1)*TSIZE)/2;
    unsigned int val=pre_val+t+(a*(n-1)); //randoms needed per shift + per column + per row

	b[0]=T[count*TSIZE+(a)];

   for(int j=0;j<=n-2;j++)
    b[j+1]=get_robprg3(n,val+j);

  locality_refresh(b,n);
}


void subbyte_htable_r_inc(byte *a,int n,int count)
{
    read_htable_inc_r(a[n-1],a,n,count);
}

//************End of functions for robust PRG*******************


//**********************htable using multiple PRG**************


//************Functions for Normal variant using multi PRG*******************

void loc_refresh_table_m(byte *Tp,byte a,int n,int ind,int count)
{
    unsigned int pre_val= count*TSIZE;
		unsigned int i,j,t;
	
    for(j=0;j<256;j++)
    {
		for(i=1;i<n;i++)
		{
			unsigned int val=ind*(n-1);
            byte tmp=get_mprg_lr(val+(i-1),n,pre_val+j);
            Tp[j]=Tp[j] ^ tmp;
     	}

        if(ind>0)
        {
            for(i=1;i<n;i++)
            {
                unsigned int val=(ind-1)*(n-1);
				 t=(j^a);
                byte tmp=get_mprg_lr(val+(i-1),n,pre_val+t);
                Tp[j]=Tp[j] ^ tmp;
            }
        }

    }
}



//************Functions for Increasing shares variant using multi PRG*******************

void loc_refresh_table_inc_m(byte *Tp,byte a,int n,int ind,int count)
{
	unsigned int pre_val= count*TSIZE;
    for(int k=0;k<TSIZE;k++)
    {
			for(int i=0;i<=ind;i++)
			{
		    int val=(ind*(ind+1))/2;
		    byte tmp=get_mprg_lr(val+i,n,pre_val+k);
		    Tp[k]=Tp[k] ^ tmp;
			}

        if(ind>0)
        {
            for(int i=0;i<ind;i++)
            {
                int val=((ind-1)*ind)/2;
                byte tmp=get_mprg_lr(val+i,n,pre_val+(k^a));
                Tp[k]=Tp[k] ^ tmp;
            }
        }

    }
}


//**************online functions**********

void htable_m(int n,int count,int choice)//htable_r(n,i);//Choice 1 for normal PRG 0 for Increasing shares
{
  unsigned int j,i,t,temp;
  byte Tp[TSIZE];
  //printf("count is %d\n",count);
  for(j=0;j<256;j++)
	{
		temp=count*TSIZE;
		T[temp+j]=sbox[j];
	}
     

  for(i=0;i<(N2-1);i++) // In pre-proessing, T will be shifted by n-1 shares.
  {
		
		t=count*(N2-1);
		temp=x_shares[t+i];
		
    shift_tab(temp,Tp,count);

    if(choice==NPRG)
			loc_refresh_table_m(Tp,temp,n,i,count);

    else if(choice==IPRG)
			loc_refresh_table_inc_m(Tp,temp,n,i,count);

		
		for(j=0;j<TSIZE;j++)
		{
		  temp=count*TSIZE;
		  T[temp+j]=Tp[j];
		}
  }
}


void read_htable_m(byte a,byte *b,int n,int count)
{
	unsigned int pre_val=count*TSIZE;
	unsigned int val=(n-2)*(n-1);
	byte x[2],res[2];
	b[0]=T[(count*TSIZE)+a];
    
	for(int j=1;j<n;j++)
		b[j]=get_mprg_lr_online(val+j-1,n,pre_val+a);
    
	locality_refresh(b,n);
}

void read_htable_inc_m(byte a,byte *b,int n,int count)
{
    unsigned int pre_val=count*TSIZE;
		unsigned int val=((n-2)*(n-1))/2;
		byte x[2],x1[2],res[2];
		b[0]=T[(count*TSIZE)+a];

    for(int j=1;j<n;j++)
			b[j]=get_mprg_lr_online(val+j-1,n,pre_val+a);
		
		locality_refresh(b,n);
}

void subbyte_htable_mprg(byte *a,int n,int count)
{
    read_htable_m(a[n-1],a,n,count);
}

void subbyte_htable_inc_mprg(byte *a,int n,int count)
{
    read_htable_inc_m(a[n-1],a,n,count);
}

//************End of functions for Increasing shares multi PRG*******************

void gen_t_all(int n,int choice,int type,int count)
{
	for(int i=0;i<count;i++)
		gen_t_forall(n,choice,type);

}


void gen_t_forall(int n,int choice,int type)//void gen_t_forall(void (*table_call) (int, int,int),int n,int choice)//void (*subbyte_share_call)(int, int),int n)
{
    unsigned int i,j,temp;
	  byte a[N2-1];
   	for(i=0;i<TSIZE;i++)
    {
		gen_rand(a,n-1);
        for(j=0;j<(n-1);j++)
        {
			temp=i*(N2-1);
            byte x = a[j];
            x_shares[temp+j]=x;

        }
					
		if(type==MPRG)
           htable_m(n,i,choice);
		else if(type==RPRG)
           htable_r(n,i,choice);

    }

}


//************state shares*************


void subbytestate_share1(byte stateshare[16][N2],int n,void (*subbyte_share_call)(byte *,int,int),int round)
{
  unsigned int i,j;
  unsigned int t,ind;

  for(i=0;i<16;i++)
  {
	ind=16*round+i;
	t=ind*(N2-1);
    byte temp=0;

    for(j=0;j<n-1;j++)
       temp=temp ^ stateshare[i][j] ^ x_shares[t+j];

    stateshare[i][n-1]=stateshare[i][n-1] ^ temp;
    subbyte_share_call(stateshare[i],n,ind);
	
  }

}
