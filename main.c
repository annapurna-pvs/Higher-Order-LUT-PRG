#include <stdio.h>

#include "AES/aes.h"

#include "PRESENT/present.h"
#include "AES/aes_htable_prg.h"
#include "Util/driver_functions.h"
#include "Util/prg3.h"





//*******************main*****************************************

int main()
{
    /**********Input parameters for Higher-order LUT-based block cipher implementation********/

    int nt = 10; //Number of times to repeat experiments
    int shares = shares_N; // #Input shares. Set the parameter in common.h.
    int cipher =AES_THIRD; //Cipher can be AES or PRESENT or BITSLICE or CRV_present or AES_THIRD or PRESENT_THIRD
    //for AES_THIRD or PRESENT_THIRD or BITSLICE no need to set scheme or type_PRG
    int scheme = VARIANT; //Set the parameter in common.h file. Type of LUT construction. normal--> NPRG  Increasing shares--> IPRG
    int type_PRG = RPRG; //Type of PRG to generate randoms, either robust-->RPRG or multiple-->MPRG

    double time[2]={0,0};// To hold offline and online execution clock cycle count
     double time1[11]={0,0,0,0,0,0,0,0,0,0,0};
    int i,k;

    printf("**********************************************\n");
    printf("Input choices\n");
    printf("Cipher: %d (1:AES 2:PRESENT 3:Bitslice 4:PRESENT_CRV 5:AES_THIRD 6:PRESENT_THIRD)\n",cipher);
    printf("#shares: %d, Variant:%d  (1:Normal 0:Increasing shares) and PRG type: %d (2:robust 3:multiple PRG)\n",shares,scheme,type_PRG);
    printf("**********************************************\n");

    if(cipher==AES||cipher==AES_THIRD)
    {
        //****************Test vectors********************
		byte keyex[16]={0x2b,0x7e,0x15,0x16,0x28,0xae,0xd2,0xa6,0xab,0xf7,0x15,0x88,0x09,0xcf,0x4f,0x3c};
		byte inex[16]={0x32,0x43,0xf6,0xa8,0x88,0x5a,0x30,0x8d,0x31,0x31,0x98,0xa2,0xe0,0x37,0x07,0x34};
		//Expected result:{0x39,0x25,0x84,0x1d,0x02,0xdc,0x09,0xfb,0xdc,0x11,0x85,0x97,0x19,0x6a,0x0b,0x32};

		byte in1[16],in2[16],out1[16],out2[16];
		// In1 and In2 (out and key also) to represent the input to unshared and shared AES block cipher.
		byte key1[16],key2[16];

		for(i=0;i<16;i++)
        {
            key1[i]=keyex[i];
            key2[i]=keyex[i];
        }


		for(i=0;i<16;i++)
        {
            in1[i]=inex[i];
            in2[i]=inex[i];

        }


		for(k=0;k<16;k++)
        {
            out1[k]=0x0;
        }


        //*******Un masked AES***
        run_aes(in1,out1,key1,1);
       
        if(cipher==AES_THIRD)
        {
            
            run_aes_shares_third(in2,out2,key2,shares,scheme,nt,time1);

        }

        //*********Masked AES with randomness generated from PRG*******
        if(cipher==AES)
        run_aes_shares_prg(in2,out2,key2,shares,scheme,type_PRG,time,nt);
       
        if(compare_output(out1,out2,16))
        {
            printf("Successful execution of LUT-based AES\n");

            if(type_PRG==RPRG)
                printf("# Pseudo random: %u\n",get_robprgcount3());

            if(type_PRG==MPRG)
            {
                int ni=0;
                if(scheme==NPRG)//Choice 1 for normal PRG 0 for Increasing shares
                    ni=(shares-1)*(shares-1);
                else
                    ni=shares*(shares-1)/2;

                printf("# Pseudo random: %u\n",get_mprg_lr_count(ni));
            }

            #if TRNG==0
            {
                if(cipher==AES)
                printf("#Milli seconds: Off-line: %f and Online: %f\n ",time[0],time[1]);
                else if(cipher==AES_THIRD)
                printf("#Milli seconds: Off-line: %f and Online: %f\n ",time1[0],time1[1]);
            }
            #else
            printf("#Clock_cycles: Off-line: %f and Online: %f\n ",time[0],time[1]);
            #endif
        }

        else
            printf("Unsuccessful execution :(, pls check...");

   }

    if(cipher==PRESENT||cipher==PRESENT_THIRD)
    {
            /*********Test Vectors*********/
        byte keyex[] ={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
        byte inex[8]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};//{0x32,0x43,0xf6,0xa8,0x88,0x5a,0x30,0x8d};


        byte in1[8],in2[8],out1[8],out2[8];
		byte key1[10],key2[10];

		for(i=0;i<10;i++)
        {
            key1[i]=keyex[i];
            key2[i]=keyex[i];
        }


		for(i=0;i<8;i++)
        {
            in1[i]=inex[i];//rand()%256;
            in2[i]=inex[i];

        }


		for(k=0;k<8;k++)
        {
            out1[k]=0x0;
            out2[k]=0x0;
        }

        present(in1,out1,key1);
        if(cipher==PRESENT)
        run_present_shares_prg(in2,out2,key2,shares,time,nt);
        if(cipher==PRESENT_THIRD)
        run_present_shares_third(in2,out2,key2,shares,time1,nt,scheme);

        if(compare_output(out1,out2,8))
        {
            printf("Successful execution of LUT-based PRESENT\n");

            #if TRNG==0
            {
                if(cipher==PRESENT_THIRD)
                 printf("#Milli seconds: Off-line: %f and Online: %f\n ",time1[0],time1[1]);
                if(cipher==PRESENT)
                printf("#Milli seconds: Off-line: %f and Online: %f\n ",time[0],time[1]);
            }
           
            #else
            printf("#Clock_cycles: Off-line: %f and Online: %f\n ",time[0],time[1]);
            #endif

        }

        else
            printf("Unsuccessful execution :(, pls check...");



    }


    if(cipher==BITSLICE)
    {
        //****************Test vectors********************
		byte keyex[16]={0x2b,0x7e,0x15,0x16,0x28,0xae,0xd2,0xa6,0xab,0xf7,0x15,0x88,0x09,0xcf,0x4f,0x3c};
		byte inex[16]={0x32,0x43,0xf6,0xa8,0x88,0x5a,0x30,0x8d,0x31,0x31,0x98,0xa2,0xe0,0x37,0x07,0x34};

		byte in1[16],in2[16],out1[16],out2[16];
		byte key1[16],key2[16];

		double time_bs=0;

		for(i=0;i<16;i++)
        {
            key1[i]=keyex[i];
            key2[i]=keyex[i];
            in1[i]=inex[i];
            in2[i]=inex[i];

        }

		for(k=0;k<16;k++)
        {
            out1[k]=0x0;
            out2[k]=0x0;
        }

        //*******Un masked AES***
        //run_aes(in1,out1,key1,1);

        //*********8-bit bitsliced AES*******
       //time_bs=run_aes_share_bitslice8(in2,out2,key2,shares,nt);
       double time_b[1]={0};
       run_bitslice(in1,out1,key1,nt);
		run_bitslice_shares(in2,out2,key2,nt,time_b);
      
        if(compare_output(out1,out2,16))
        {
            printf("Successful execution of bitsliced AES\n");

            #if TRNG==0
            printf("#Milli seconds: Online: %f ",time_b[0]); //printf("#Milli seconds: Online: %f ",time_bs);
            #else
            printf("#Clock_cycles: Online: %f ",time_bs);
            #endif

        }

        else
            printf("Unsuccessful execution :(, pls check...");


    }

    if(cipher==CRV_present)
    {
            /*********Test Vectors*********/
        byte keyex[] ={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
        byte inex[8]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};//{0x32,0x43,0xf6,0xa8,0x88,0x5a,0x30,0x8d};


        byte in1[8],in2[8],out1[8],out2[8];
		byte key1[10],key2[10];

		double time_crv=0.0;

		for(i=0;i<10;i++)
        {
            key1[i]=keyex[i];
            key2[i]=keyex[i];
        }


		for(i=0;i<8;i++)
        {
            in1[i]=inex[i];//rand()%256;
            in2[i]=inex[i];

        }


		for(k=0;k<8;k++)
        {
            out1[k]=0x0;
            out2[k]=0x0;
        }

        present(in1,out1,key1);
        time_crv = run_present_shares_crv(in2,out2,key2,shares,nt);
        //testCRV_present_share();

        if(compare_output(out1,out2,8))
        {
            printf("Successful execution of LUT-based PRESENT using CRV\n");

            #if TRNG==0
            printf("#Milli seconds: Online: %f ",time_crv);
            #else
            printf("#Clock_cycles: Online: %f ",time_crv);
            #endif

        }

        else
            printf("Unsuccessful execution :(, pls check...");


    }
	return 0;
}

