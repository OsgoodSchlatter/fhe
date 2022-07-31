#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <time.h>

/***
 * completes the truth table of three inputs (a,b,c_in)
 * helpful in multiplier
 * serves as a half adder if c_in=0
 * @arg sum: sum of two bits
 * @arg a: bit 1
 * @arg b: bit 2
 * @arg carry_in: carry_in bit that will be turned into carry_out. Then, it will be carry_in for next full_adder
 *
 * @return sum and carry_out
 * ***/
void full_adder_one_bit(LweSample *sum, LweSample *a, LweSample *b, LweSample *carry_in, const TFheGateBootstrappingCloudKeySet *bk)
{

    const LweParams *in_out_params = bk->params->in_out_params;
    // carries & result & temp
    LweSample *temp = new_LweSample_array(3, in_out_params);
    // layer 1
    bootsXOR(temp, a, b, bk);
    bootsAND(temp + 1, a, b, bk);
    // layer 2
    bootsXOR(sum, carry_in, temp, bk);
    bootsAND(temp + 2, carry_in, temp, bk);
    bootsOR(carry_in, temp + 1, temp + 2, bk);
}