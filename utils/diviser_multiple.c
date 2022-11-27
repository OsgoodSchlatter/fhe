#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <time.h>
#include "./diviser.h"
#include "./diviser_multiple.h"

void diviser_multiple(LweSample *result, LweSample *offers[], int offerNbr,
                      const TFheGateBootstrappingCloudKeySet *keyset)
{
    const LweParams *in_out_params = keyset->params->in_out_params;

    LweSample *tmp = new_LweSample_array(16, in_out_params);

    diviser(tmp, offers[0], offers[1], keyset);
    for (int index = 2; index < offerNbr; index++)
    {
        // printf("echo %d\n", index);
        for (int j = 0; j < 16; j++)
        {
            bootsCOPY(&result[j], &tmp[j], keyset);
        }
        diviser(tmp, result, offers[index], keyset);
    }

    for (int j = 0; j < 16; j++)
    {
        bootsCOPY(&result[j], &tmp[j], keyset);
    }
}