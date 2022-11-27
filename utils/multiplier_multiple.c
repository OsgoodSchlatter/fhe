#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <time.h>
#include "./multiplier.h"
#include "multiplier_multiple.h"

void multiplier_multiple(LweSample *result, LweSample *offers[], int offerNbr, int nb_bits,
                         const TFheGateBootstrappingCloudKeySet *keyset)
{
    const LweParams *in_out_params = keyset->params->in_out_params;

    LweSample *tmp = new_LweSample_array(16, in_out_params);

    multiplier(tmp, offers[0], offers[1], nb_bits, keyset);
    for (int index = 2; index < offerNbr; index++)
    {
        for (int j = 0; j < 16; j++)
        {
            bootsCOPY(&result[j], &tmp[j], keyset);
        }
        multiplier(tmp, result, offers[index], nb_bits, keyset);
    }

    for (int j = 0; j < 16; j++)
    {
        bootsCOPY(&result[j], &tmp[j], keyset);
    }
}