#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>

// returns 4, 2 or 1 respectively wether a=b, a>b or a<b
void compare_bit_own(LweSample *result, LweSample *a, LweSample *b, const TFheGateBootstrappingCloudKeySet *bk);