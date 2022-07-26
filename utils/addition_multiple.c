#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <time.h>
#include "./addition_multiple.h"
#include "full_adder.h"

// performs addition on two ciphered data MY VERSION
void addition_multiple(LweSample *result, LweSample *offers[], int offerNbr,
                       const TFheGateBootstrappingCloudKeySet *keyset)
{
    const LweParams *in_out_params = keyset->params->in_out_params;

    LweSample *tmp = new_LweSample_array(16, in_out_params);

    full_adder(tmp, offers[0], offers[1], keyset);
    for (int index = 2; index < offerNbr; index++)
    {
        for (int j = 0; j < 16; j++)
        {
            bootsCOPY(&result[j], &tmp[j], keyset);
        }
        full_adder(tmp, result, offers[index], keyset);
    }

    for (int j = 0; j < 16; j++)
    {
        bootsCOPY(&result[j], &tmp[j], keyset);
    }
}