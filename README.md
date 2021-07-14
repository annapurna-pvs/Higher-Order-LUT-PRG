# Higher-Order-LUT-PRG

Higher-Order Lookup Table Masking in Essentially Constant Memory

The provided c code is to run the higher-order look-up table scheme using PRG for various security orders. 
The proposed lookup table countermeasure is shown to be t-SNI secure against a t-th order DPA attack.

The target device for the code is FRDM-K64f from NXP and microcontroller on the target architecture is MK64FN1M0VLL12.
The code makes use of RNGA module built-in to the microcontroller for random number generation.

The code for header files aes.h, aes_share.h, aes_rp.h, share.h and souce code files aes.c, aes_share.c, aes_rp.c and share.c are taken from the public github repository https://github.com/coron/htable. Few methods from these files are customized according to the target architecture requirements.

Notes:

The key scheduling is not secured against DPA attacks and can be imagined as a black-box (means can not be probed by a DPA attacker).
The RNGA module is microcontroller specific and need the device specific files for compiling the code.






