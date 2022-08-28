#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <time.h>

#ifndef subtract
#define subtract

/***
 * completes the truth table of three inputs (a,b,c_in) for subtraction on one bit
 * helpful in multiplier
 * @arg subtract: subtract of two bits
 * @arg a: bit 1
 * @arg b: bit 2
 * @arg carry_in: carry_in bit that will be turned into carry_out. Then, it will be carry_in for next full_subtract
 *
 * @return subtract and carry_out
 * ***/
void full_subtract_one_bit(LweSample *_subtract, LweSample *a, LweSample *b, LweSample *carry_in, const TFheGateBootstrappingCloudKeySet *bk)
{
    const LweParams *in_out_params = bk->params->in_out_params;
    // carries & result & temp
    LweSample *temp = new_LweSample_array(4, in_out_params);
    LweSample *not_a = new_LweSample_array(1, in_out_params);
    bootsNOT(not_a, a, bk);
    // layer 1
    bootsXOR(temp, a, b, bk);
    // computing temp_3 = not(temp_0)
    bootsNOT(temp + 3, temp, bk);
    bootsAND(temp + 1, not_a, b, bk);
    // layer 2
    bootsXOR(_subtract, carry_in, temp, bk);
    bootsAND(temp + 2, carry_in, temp + 3, bk);
    bootsOR(carry_in, temp + 1, temp + 2, bk);
}

#endif