#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <time.h>
#include "full_subtract.h"

void max(LweSample *result, LweSample *x, LweSample *y, const TFheGateBootstrappingCloudKeySet *keyset)
{
    int nb_bits = 16;
    LweSample *tmp = new_LweSample_array(16, keyset->bk->in_out_params);
    full_subtract(tmp, x, y, keyset);
    for (int i = 0; i < nb_bits; i++)
        bootsMUX(result + i, tmp + nb_bits - 1, y + i, x + i, keyset);
}

// performs  max
void max_multiple(LweSample *result, LweSample *offers[], int offerNbr,
                  const TFheGateBootstrappingCloudKeySet *keyset)
{
    const LweParams *in_out_params = keyset->params->in_out_params;

    LweSample *tmp = new_LweSample_array(16, in_out_params);

    max(tmp, offers[0], offers[1], keyset);
    for (int index = 2; index < offerNbr; index++)
    {
        for (int j = 0; j < 16; j++)
        {
            bootsCOPY(&result[j], &tmp[j], keyset);
        }
        max(tmp, result, offers[index], keyset);
    }

    for (int j = 0; j < 16; j++)
    {
        bootsCOPY(&result[j], &tmp[j], keyset);
    }
}