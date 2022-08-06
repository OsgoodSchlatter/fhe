#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>

void compare_bit_own(LweSample *result, LweSample *a, LweSample *b, const TFheGateBootstrappingCloudKeySet *bk)
{
    LweSample *inf_or_eq_or_sup = new_LweSample_array(3, bk->params->in_out_params);
    LweSample *temp = new_LweSample_array(2, bk->params->in_out_params);
    LweSample *not = new_LweSample_array(2, bk->params->in_out_params);
    // not a
    bootsNOT(not, a, bk);
    // not b
    bootsNOT(not +1, b, bk);
    bootsAND(temp, not, b, bk);
    bootsAND(temp + 1, a, not +1, bk);
    // a < b
    bootsCOPY(result, temp, bk);
    // a > b
    bootsCOPY(result + 1, temp + 1, bk);
    // a = b
    bootsNOR(result + 2, temp, temp + 1, bk);
}