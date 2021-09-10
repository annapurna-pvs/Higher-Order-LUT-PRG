#include <stdio.h>
#include <time.h>

#include "common.h"
#include "../AES/aes.h"


#if TRNG == 1
#include "board.h"
#include "MK64F12.h"
#include "fsl_rnga.h"
#include "fsl_debug_console.h"

RNG_Type *const base =(RNG_Type *) ((char *)0+ 0x40029000u); //RNG_base register initialisation*/

#else

static byte seed_AES[16]={0x42,0x78,0xb8,0x40,0xfb,0x44,0xaa,0xa7,0x57,0xc1,0xbf,0x04,0xac,0xbe,0x1a,0x3e};
static byte counter_AES[16]={0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
static byte buf_AES[16];
static byte rem=0,c_out=0,ind=0;

#endif // TRNG

/************* TRNG random values count***********/

static unsigned int randcount=0;


void init_randcount()
{
  randcount=0;
}

unsigned int get_randcount()
{
  return randcount;
}

void set_randcount(unsigned int randc)
{
  randcount=randc;
}


/***********Code for TRNG/ AES-CTR PRG**********/



void rand_in(){

	#if TRNG == 1
		//RNGA initialisation code
		RNGA_Init(base);
		rnga_mode_t mode;
		//status_t flag;
		mode=0U;
		RNGA_SetMode(base,mode);

	#endif // TRNG

}

void gen_rand(byte *arr,int size){ //Populate arr with required number of random bytes using RNGA


    int j;
		for(j=0;j<size;j++)
    {
			arr[j]=0;
		}

		#if TRNG == 1

				status_t flag=RNGA_GetRandomData(base,arr,size);

		#else

				int req=size;
				int i=0;
				int temp=rem;

				while(rem && req)
				{
						arr[i]=buf_AES[15-i];
						i++;
						req--;
						rem--;
				}

				while(req>0)
				{
						aes(counter_AES,buf_AES,seed_AES);
						rem=16;

						c_out++;
						counter_AES[ind]=c_out;

						if(c_out==255 && ind<15)
						{
								ind++;
								c_out=0;
						}

						if(ind>15)
								printf("Counter value will repeat..Re-initialise PRG seed and reset the counter");


						for(i=0;i<req&&i<rem;i++,req--,rem--)
								arr[temp+i]=buf_AES[i];

				}


		#endif // TRNG


}

void rand_dein()
{
        #if TRNG > 0
			RNGA_Deinit(base);
         #endif // TRNG
}


void reset_systick()
{
    #if TRNG==1

    SysTick->CTRL = 0; // Disables SysTick

    SysTick->LOAD = 0xFFFFFFFF; // Sets the Reload value to maximum

    SysTick->VAL = 0; // Clears the current value to 0

    SysTick->CTRL = 0x5; // Enables the SysTick, uses the processor clock

    while(SysTick->VAL != 0); // Waits until the SysTick is reloaded

    #endif // TRNG
}


int compare_output(byte *out1,byte *out2,byte size)
{
    for(int i=0;i<size;i++)
        if(out1[i]!=out2[i])
            return 0;
    return 1;
}




double cal_time(clock_t stop, clock_t start)
{
    double time =((double) (stop - start))*UNIT/CLOCKS_PER_SEC;// * 1000000000 + stop.tv_usec - start.tv_usec);
    //time=time*UNIT;
    return time;
}



