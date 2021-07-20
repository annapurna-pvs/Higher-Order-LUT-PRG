#include <stdio.h>
#include "present.h"
#include "rand_k64.h"
#include "share.h"
#include "aes_htable_prg.h"
#include "present_htable_PRG.h"
#include "prg3.h"

void addroundkey_present_share(byte *state[8],byte *key[10],int n)
{
    int i,j;
   for(i=0;i<n;i++)
    for(j=0;j<8;j++)
        state[j][i] = state[j][i] ^ key[j+2][i];
}


void keyschedule_present_share(byte *key,int round)
{
    byte save1,save2;
    int i;

   		save1  = key[0];
		save2  = key[1];

		for(i=0;i<8;i++)
            key[i] = key[i+2];

		key[8] = save1;
		key[9] = save2;
		save1 = key[0] & 7;								//61-bit left shift

        for(i=0;i<9;i++)
			key[i] = key[i] >> 3 | key[i+1] << 5;

		key[9] = key[9] >> 3 | save1 << 5;
		key[9] = sbox_p[key[9]>>4]<<4 | (key[9] & 0xF);	//S-Box application

		if((round+1) % 2 == 1)							//round counter addition
			key[1] ^= 128;
		key[2] = ((((round+1)>>1) ^ (key[2] & 15)) | (key[2] & 240));

}

void permute_present_share(byte *state[8],int n)
{
    int i,j;
    byte temp[8];

    for(j=0;j<n;j++)
    {

    for(i=0;i<8;i++)
    {
			temp[i] = 0;
	}

	for(i=0;i<64;i++)
	{
		byte position = (16*i) % 63;						//Artithmetic calculation of the pLayer
		if(i == 63)									//exception for bit 63
			position = 63;
		byte element_source		= i / 8;
		byte bit_source 			= i % 8;
		byte element_destination	= position / 8;
		byte bit_destination 	= position % 8;
		temp[element_destination] |= ((state[element_source][j]>>bit_source) & 0x1) << bit_destination;
    }

	for(i=0;i<=7;i++)
			state[i][j] = temp[i];

    }
}


int main()
{
	byte key[] ={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
	byte state[8]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};//{0x32,0x43,0xf6,0xa8,0x88,0x5a,0x30,0x8d};//{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	byte *stateshare[8],*keyshare[10],state1[8];
    byte i=0;
    int round,n=N;

    int ni=n*(n-1)/2;

    printf("initialisation of multiple PRGs\n");
    init_mprg2(n,ni);


    printf("pre-processing of tables\n");
    gen_t_forall_present(n);
    printf("Done with pre-processing of tables\n");

    for(i=0;i<8;i++)
    {
        stateshare[i]=(byte*) malloc(n*sizeof(byte));
        share_rnga(state[i],stateshare[i],n);
    }

    for(i=0;i<10;i++)
    {
        keyshare[i]=(byte*) malloc(n*sizeof(byte));
        share_rnga(key[i],keyshare[i],n);
    }

    printf("Online phase\n");

    for(round=0;round<31;round++)
    {
        addroundkey_present(state,key);
        addroundkey_present_share(stateshare,keyshare,n);

        sbox_present(state);
        subbytestate_share_prg_present(stateshare,n,subbyte_htable_inc_mprg_present,round);

        permute_present(state);
        permute_present_share(stateshare,n);

        keyschedule_present(key,round);

        for(i=0;i<10;i++)
        {
            share_rnga(key[i],keyshare[i],n);

        }

    }
    addroundkey_present(state,key);
    addroundkey_present_share(stateshare,keyshare,n);

    for(i=0;i<8;i++)
        state1[i]=decode(stateshare[i],n);

	for(i=0;i<8;i++)
    {
        if(state[i]!=state1[i])
        {
            printf("Output is not as expected!! pls check....");
            return 0;
        }

    }

    printf("Obtained matched expected :-\n");

    free_mprg2(n,ni);
	return 0;
}


