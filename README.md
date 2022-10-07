# Higher-Order-LUT-PRG and Third-Order Lookup Table base Masking
Higher-Order Lookup Table Masking in Essentially Constant Memory And A Faster Third-Order Masking of Lookup Tables

The provided c code is to run the higher-order look-up table (LUT) scheme using PRG for various security orders. 
Our code supports the masked execution of AES and PRESENT block-ciphers using higher-order LUT where the required randomness for the implementation is generated using a robust PRG or from multiple PRGs. It also provides the masked bitsliced implementation of 8-bit AES and circuit-based implementation of PRESENT using the CRV scheme. 
The proposed lookup table countermeasure is shown to be t-SNI secure against a t-th order DPA attack.

The target device for the code is FRDM-K64f from NXP and the microcontroller on the target architecture is MK64FN1M0VLL12.  The code makes use of the RNGA module built-in to the microcontroller for generating the input random seed/initial sharing of key/plaintext.

To build the code, 
1. run $make clean 
2. $make (the executable will be created inside the Build folder) 
3. Run the executable using $./Build/higherPRG.exe

The code can either run on the target micro-controller or on a desktop. Set the value of TRNG(in Utils/common.h) to "zero (0)" to run on a desktop (where the random seed is obtained using AES-CTR PRG) or to "one(1)" to use device built-in RNGA. This code will include the appropriate header files depending on the choice of TRNG parameter. 

The following input choices will decide the outcome.
*************************************************************
Set the following inputs in Utils/common.h

1. Set TRNG to 0 or 1. 
2. Set #shares using the constant shares_N.
3. Set VARIANT as either Normal (NPRG)/ Increasing (IPRG) shares.

And the below input parameters inside main:

4. CIPHER decides the block cipher and the scheme to execute. It can be AES or PRESENT or BITSLICE or CRV_present.
5. type_PRG indicates the type of PRG to generate randoms, either robust-->RPRG or multiple PRG-->MPRG.

*************************************************************

The following is the output from the code
*************************************************************

1. Summary of input choices (cipher, #shares, variant, type of PRG)
2. Indicative flag of execution status: success/failure
3. On successful execution, count of pseudorandom values that are generated using robust/multiple PRG
4. The offline and online execution times indicating the pre-processing time and online time, respectively.
*************************************************************

Please note for circuit-based schemes (bitslice and CRV), the code outputs online time only, as there is no offline time. If TRNG=0, our code outputs time in milliseconds whereas when TRNG=1, it outputs clock_cycle_count. 

It is also possible to customise the code for a target micro-controller. List of changes to the current code to customise to a target device.

*************************************************************
1. Update the header file for the target device (Replace #include "MK64F12.h" with the header file of target device).
2. If required, modify the device-specific header files and RNGA initialisation in "Utils/common.c"  to suit the target device. 
*************************************************************

The code for header files aes.h, aes_share.h, aes_rp.h, share.h and souce code files aes.c, aes_share.c, aes_rp.c and share.c is taken from the public github repository https://github.com/coron/htable. Few methods from these files are customized according to the target architecture requirements.

The unmasked PRESENT implementation is taken from http://www.lightweightcrypto.org/implementations.php. 

The sample build file is taken from https://riptutorial.com/makefile/example/21376/building-from-different-source-folders-to-different-target-folders 

Note:
The key scheduling is not secured against DPA attacks and can be imagined as a black-box (means can not be probed by a DPA attacker).





