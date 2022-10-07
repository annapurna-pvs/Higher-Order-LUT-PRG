#include <stdio.h>
#include <time.h>


#if TRNG == 1
#include "MK64F12.h"
#else
#include <time.h>
#endif

#include "../Util/common.h"
#include "../Util/share.h"
#include "../Util/prg3.h"
#include "../Util/driver_functions.h"

#include "aes.h"
#include "aes_htable_prg.h"

byte wshare[176][shares_N];


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

void addroundkey_share(byte stateshare[16][shares_N],int round,int n)
{
      int i,j;
      for(i=0;i<16;i++)
        for(j=0;j<n;j++)
          stateshare[i][j]^=wshare[16*round+i][j];
}



void shiftrows_share(byte stateshare[16][shares_N],int n)
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



void mixcolumns_share(byte stateshare[16][shares_N],int n)
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

      byte stateshare[16][shares_N];

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

void run_aes_share_rprg_table(byte in[16],byte out[16],byte key[16],int n,void (*subbyte_share_call)(byte *,int,int),int nt)
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

void run_aes_share_mprg_table(byte in[16],byte out[16],byte key[16],int n,void (*subbyte_share_call)(byte *,int,int),int nt)
{
      keyexpansion_share(key,n);

      for(int i=0;i<nt;i++)
        aes_share_subkeys(in,out,n,subbyte_share_call);

}

int rprg_AES(int n) //Value of r for robust PRG
{
        int loc=1;
        loc=2*(n-1);
        return (n-1)*loc;
}

void run_aes_shares_prg(byte *in,byte *out,byte *key,int n,int choice,int type,double *time,int nt)
{
    int ni;

	#if TRNG==0
    struct timespec begin, end;
	#endif

	unsigned int begin1,end1;
    long sec,nsec;
    double temp=0.0;



    //TRNG initialisation
		rand_in();

    //**********Higher-order AES with pre-processing using robustPRG T contains pre-processed tables

    if(type==RPRG)
    {
        int rprg=rprg_AES(n);
        printf("Inside main ...higher-order PRG with robust prg %d!!!\n",rprg);
        init_robprg3(rprg,n);

        printf("Pre-computation of 160 tables for AES-128...\n");

        #if TRNG==0
        clock_gettime(CLOCK_REALTIME, &begin);
        #endif // TRNG

        #if TRNG==1
        reset_systick();
        begin1 = SysTick->VAL; // Obtains the start time
        #endif // TRNG

        gen_t_forall(n,choice,type); //Pre-processing table T1 for all rounds

        #if TRNG==0
        clock_gettime(CLOCK_REALTIME, &end);
        sec = end.tv_sec - begin.tv_sec;
        nsec = end.tv_nsec - begin.tv_nsec;
        temp = sec + nsec*1e-9;

        time[0] = temp*UNIT;
        #endif // TRNG

        #if TRNG==1
        end1 = SysTick->VAL; // Obtains the stop time
        time[0] = (double) (begin1-end1); // Calculates the time taken
        #endif // TRNG


        printf("Online pahse\n");


        #if TRNG==0
        clock_gettime(CLOCK_REALTIME, &begin);
        #endif // TRNG

        #if TRNG==1
        reset_systick();
        begin1 = SysTick->VAL; // Obtains the start time
        #endif // TRNG

        if(choice==NPRG)
        {
            /*robust PRG with normal variant*/
            run_aes_share_rprg_table(in,out,key,n,&subbyte_htable_r,nt);

        }

        if(choice==IPRG)
        {
            /*robust PRG with increasing shares variant*/
            run_aes_share_rprg_table(in,out,key,n,&subbyte_htable_r_inc,nt);

        }


        #if TRNG==0
        clock_gettime(CLOCK_REALTIME, &end);
        sec = end.tv_sec - begin.tv_sec;
        nsec = end.tv_nsec - begin.tv_nsec;
        temp = sec + nsec*1e-9;

        time[1] = temp*UNIT/nt;
        #endif // TRNG

        #if TRNG==1
        end1 = SysTick->VAL; // Obtains the stop time
        time[1] = ((double)(begin1-end1))/nt; // Calculates the time taken
        #endif // TRNG

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

            #if TRNG==0
            clock_gettime(CLOCK_REALTIME, &begin);

            #endif // TRNG

            #if TRNG==1
            reset_systick();
            begin1 = SysTick->VAL; // Obtains the start time
            #endif // TRNG

            gen_t_forall(n,choice,type); //Pre-processing table T1 for all rounds

            #if TRNG==0

            clock_gettime(CLOCK_REALTIME, &end);
            sec = end.tv_sec - begin.tv_sec;
            nsec = end.tv_nsec - begin.tv_nsec;
            temp = sec + nsec*1e-9;

            time[0] = temp*UNIT;//cal_time(stop,start);
            #endif // TRNG

            #if TRNG==1
            end1 = SysTick->VAL; // Obtains the stop time
            time[0] = (double) (begin1-end1); // Calculates the time taken
            #endif // TRNG

            printf("Online Phase\n\n\n");

            #if TRNG==0
            clock_gettime(CLOCK_REALTIME, &begin);
            #endif // TRNG

            #if TRNG==1
            reset_systick();
            begin1 = SysTick->VAL; // Obtains the start time
            #endif // TRNG


            if(choice==NPRG)
            {
                /*multiple PRG with normal variant*/
                run_aes_share_mprg_table(in,out,key,n,&subbyte_htable_mprg,nt);
            }

            if(choice==IPRG)
            {
                /*multi PRG with increasing shares variant*/
                run_aes_share_mprg_table(in,out,key,n,&subbyte_htable_inc_mprg,nt);
            }

            #if TRNG==0

            clock_gettime(CLOCK_REALTIME, &end);
            sec = end.tv_sec - begin.tv_sec;
            nsec = end.tv_nsec - begin.tv_nsec;
            temp = sec + nsec*1e-9;

            time[1] = temp*UNIT/nt;//cal_time(stop,start);

            #endif // TRNG

            #if TRNG==1
            end1 = SysTick->VAL; // Obtains the stop time
            time[1] = (double) (begin1-end1); // Calculates the time taken
            #endif // TRNG

            //printf("Total random bytes used are: %d",get_robprgcount3());
            free_mprg2(n,ni);
        }
    //random generator de-initialisation
    rand_dein();

}

/*********************specific to third order***********************/
void aes_share_subkeys_third(byte in[16], byte out[16], int n, void(*subbyte_share_call)(byte *, int, int, int), int choice)
{
	int i, tmp = 0;
	int round = 0;  
	byte stateshare[16][shares_N];
	for (i = 0; i < 16; i++)
	{
		share_rnga(in[i], stateshare[i], n);
	}
	addroundkey_share(stateshare, 0, n); 
	for (round = 1; round < 10; round++)
	{
		subbytestate_share_third(stateshare, n, subbyte_share_call, round - 1, choice);
		
		shiftrows_share(stateshare, n);
		mixcolumns_share(stateshare, n);
		addroundkey_share(stateshare, round, n);
	}

	subbytestate_share_third(stateshare, n, subbyte_share_call, round - 1, choice);
	shiftrows_share(stateshare, n);
	addroundkey_share(stateshare, 10, n);

	for (i = 0; i < 16; i++)
	{
		out[i] = decode(stateshare[i], n);
		//free(stateshare[i]);
	}
}

void run_aes_third(byte in[16], byte out[16], byte key[16], int n, void(*subbyte_share_call)(byte *, int, int, int), int nt, int choice, double time[11])
{
	int i;
	keyexpansion_share(key, n);
	unsigned int begin1=0,end1=0;
 
	for (i = 0; i < nt; i++)
	{

		#if TRNG==1
            reset_systick();
            begin1 = SysTick->VAL; // Obtains the start time
            #endif // TRNG
		aes_share_subkeys_third(in, out, n, subbyte_share_call, choice);

		
		#if TRNG==1
            end1 = SysTick->VAL; // Obtains the stop time
            time[i+1] = ((double) (begin1-end1)); // Calculates the time taken
            #endif // TRNG
	}

}
void run_aes_shares_third(byte *in, byte *out, byte *key, int n, int type, int nt, double time1[11])
{
	
	unsigned int begin1, end1, begin2, end2;
	long sec, nsec;
	double temp = 0.0;
		#if TRNG==0
    struct timespec begin, end;
	#endif

        #if TRNG==0
        clock_gettime(CLOCK_REALTIME, &begin);
        #endif // TRNG
    		
        #if TRNG==1
            reset_systick();
            begin1 = SysTick->VAL; // Obtains the start time
            #endif // TRNG
        gen_t_forall_third(n, type);
       
        #if TRNG==1
            end1 = SysTick->VAL; // Obtains the stop time
            time[0] = (double) (begin1-end1); // Calculates the time taken
            #endif // TRNG
        
        #if TRNG==0
        clock_gettime(CLOCK_REALTIME, &end);
        sec = end.tv_sec - begin.tv_sec;
        nsec = end.tv_nsec - begin.tv_nsec;
        temp = sec + nsec*1e-9;

        time1[0] = temp*UNIT;
        #endif // TRNG
		//printf("\n \n Online Phase\n\n\n");
        #if TRNG==0
        clock_gettime(CLOCK_REALTIME, &begin);
        #endif // TRNG
        run_aes_third(in, out, key, n, &subbyte_htable_third, nt, type, time1);	
		
        #if TRNG==0
        clock_gettime(CLOCK_REALTIME, &end);
        sec = end.tv_sec - begin.tv_sec;
        nsec = end.tv_nsec - begin.tv_nsec;
        temp = sec + nsec*1e-9;

        time1[1] = temp*UNIT/nt;
        #endif // TRNG
	
}

