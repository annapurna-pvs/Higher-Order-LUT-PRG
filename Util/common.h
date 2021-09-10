typedef unsigned char byte;

/*********This file has global constants and common functions********/

#include <time.h>

#define TRNG 0 // Set value to 1 to use device-specific TRNG. Set to 0 to use AES-based PRG.

#define NPRG 1 // Normal variant
#define IPRG 0 // Increasing shares variant

#define RPRG 2 // Robust PRG
#define MPRG 3 // Multiple PRG

/***********Input parameters to define********/

#define shares_N 5 // #shares.
#define VARIANT IPRG //// Either Normal (NPRG)/ Increasing (IPRG) variants

/***********end of input parameters**********/

#define UNIT 1000000 //Time unit (Milli seconds)

extern int cipher; // Cipher can be either AES/ PRESENT/ BITSLICE
extern int type_PRG; // Either Robust/ multiple PRG

void rand_in(void);
void rand_dein(void);
void gen_rand(byte *a,int n);
unsigned int gen_rand32(void);

void init_randcount(void);
unsigned int get_randcount(void);
void set_randcount(unsigned int randc);

double cal_time(clock_t stop, clock_t start);
void reset_systick(void);

int compare_output(byte *out1,byte *out2,byte size);
