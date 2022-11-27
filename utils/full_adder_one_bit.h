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
void full_adder_one_bit(LweSample *sum, LweSample *a, LweSample *b, LweSample *carry_in, const TFheGateBootstrappingCloudKeySet *bk);