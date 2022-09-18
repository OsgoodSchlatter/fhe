#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <time.h>
#include "./full_adder_one_bit.h"

// performs addition on two ciphered data MY VERSION
void full_adder(LweSample *result, LweSample *x, LweSample *y,
                const TFheGateBootstrappingCloudKeySet *keyset)
{
    const LweParams *in_out_params = keyset->params->in_out_params;
    // carries
    LweSample *carry = new_LweSample_array(2, in_out_params);
    // bootsSymEncrypt(carry, 0, keyset); // first carry initialized to 0
    bootsCONSTANT(carry, 0, keyset);

    for (int i = 0; i < 16; i++)
    {
        full_adder_one_bit(result + i, x + i, y + i, carry, keyset);
    }
}