#include <stdio.h>

#include "../Util/prg3.h"
#include "../Util/share.h"

#include "present.h"
#include "present_htable_PRG.h"

#define P_TSIZE 16 //16 for PRESENT


byte p_x_shares[496*(shares_N-1)]; //Shares of x used as part of pre-processing.  496 *(N2-1) fpr PRESENT
byte PT[496*P_TSIZE]; //Size of pre-computed tables for 10*16=160 S-box calls for AES-128, 496*P_TSIZE for PRESENT.


//************Functions for Normal variant using robust PRG *******************


//*************off-line functions******


//********************* Pre-processing of Table T ***************************


void shift_tab_present(byte a,byte *Tp,int count)//shift_tab(x_shares[count][i],Tp,count);
{
	unsigned int j,temp,temp1;

	for(j=0;j<P_TSIZE;j++)
	{
		temp=count*P_TSIZE;
		temp1=j^a;
        Tp[j]=PT[temp+temp1];
	}
}


//************Functions for Increasing shares variant using multi PRG*******************

void loc_refresh_table_inc_m_present(byte *Tp,byte a,int n,int ind,int count)
{
	unsigned int pre_val= count*P_TSIZE;
    for(int k=0;k<P_TSIZE;k++)
    {
			for(int i=0;i<=ind;i++)
			{
                int val=(ind*(ind+1))/2;
                byte tmp=get_mprg_lr_present(val+i,n,pre_val+k)%16;
                Tp[k]=Tp[k] ^ tmp;
			}

            if(ind>0)
            {
                for(int i=0;i<ind;i++)
                {
                    int val=((ind-1)*ind)/2;
                    byte tmp=get_mprg_lr_present(val+i,n,pre_val+(k^a))%16;
                    Tp[k]=Tp[k] ^ tmp;
                }
            }

    }
}

void htable_m_present(int n,int count)
{
    unsigned int t,temp;
    byte Tp[P_TSIZE];

    for(int j=0;j<P_TSIZE;j++)
	{
		temp=count*P_TSIZE;
		PT[temp+j]=sbox_p[j];
	}

	byte k=n-1;

    for(int i=0;i<k;i++) // In pre-proessing, T will be shifted by n-1 shares.
    {

        t=count*k;
        temp=p_x_shares[t+i];

        shift_tab_present(temp,Tp,count);
        loc_refresh_table_inc_m_present(Tp,temp,n,i,count);

        for(int j=0;j<P_TSIZE;j++)
		{
            temp=count*P_TSIZE;
            PT[temp+j]=Tp[j];
		}
  }

}


void gen_t_forall_present(int n)
{
    unsigned int i,j,temp;
    byte a[n-1];

   	for(i=0;i<496;i++)
    {
		gen_rand(a,n-1);
		temp=i*(n-1);

		for(j=0;j<n-1;j++)
        {
            p_x_shares[temp+j]=(a[j]%16);
        }
        htable_m_present(n,i);


    }

}


//**************online functions**********


void read_htable_inc_m_present(byte a,byte *b,int n,int count)
{
    unsigned int pre_val=count*P_TSIZE;
    unsigned int val=((n-2)*(n-1))/2;
	b[0]=PT[(count*P_TSIZE)+a];

    for(int j=1;j<n;j++)
			b[j]=get_mprg_lr_present(val+j-1,n,pre_val+a)%16;

}


void subbyte_htable_inc_mprg_present(byte *a,int n,int count)
{
    read_htable_inc_m_present(a[n-1],a,n,count);
}



void subbytestate_share_prg_present(byte stateshare[8][shares_N],int n,void (*subbyte_share_call)(byte *,int,int),int round)
{
  unsigned int i,j;
  unsigned int t,ind[2];
  byte a[2][shares_N];

  /*for(j=0;j<2;j++)
        a[j]=(byte*) malloc(n*sizeof(byte));*/

  for(i=0;i<8;i++)
  {

	ind[0]=16*round+i;
	ind[1]=16*round+i+1;
	t=ind[0]*(n-1);
    byte temp[2]={0,0};

    for(j=0;j<n;j++)
    {
        a[0][j]=stateshare[i][j]>>4;
        a[1][j]=stateshare[i][j] & 0xF;
    }

    for(j=0;j<n-1;j++)
    {
        temp[0]=temp[0]^a[0][j]^p_x_shares[t+j];
        temp[1]=temp[1]^a[1][j]^p_x_shares[t+(n-1)+j];
    }

    a[0][n-1]=a[0][n-1]^temp[0];
    a[1][n-1]=a[1][n-1]^temp[1];


    subbyte_share_call(a[0],n,ind[0]);
    subbyte_share_call(a[1],n,ind[1]);

    for(j=0;j<n;j++)
    {
        stateshare[i][j]=a[0][j]<<4 | a[1][j];
    }

    locality_refresh(stateshare[i],n);

  }

}





//************End of functions for Increasing shares multi PRG*******************

