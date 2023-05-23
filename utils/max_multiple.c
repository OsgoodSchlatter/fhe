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

// performs  max
void max_mult_inputs_sort(LweSample *result[], LweSample *offers[], int offerNbr,
                          const TFheGateBootstrappingCloudKeySet *keyset)
{
    const LweParams *in_out_params = keyset->params->in_out_params;

    LweSample *zero = new_LweSample_array(1, keyset->params->in_out_params);
    bootsCONSTANT(zero, 0, keyset);
    LweSample *one = new_LweSample_array(1, keyset->params->in_out_params);
    bootsCONSTANT(one, 1, keyset);

    LweSample *tmp_max = new_LweSample_array(16, in_out_params);

    LweSample *tmp_sub = new_LweSample_array(16, in_out_params);

    // we have the array in which we know the positions of the data to copy
    LweSample *number_to_copy[offerNbr];

    LweSample *offers_cpy[offerNbr];

    for (int offer = 0; offer < offerNbr; offer++)
    {
        max_mult_inputs(tmp_max, offers, offerNbr, keyset);

        for (int offer_tmp = 0; offer_tmp < offerNbr; offer_tmp++)
        {
            offers_cpy[offer_tmp] = new_LweSample_array(16, in_out_params);
            for (int i = 0; i < 16; i++)
                bootsCOPY(offers_cpy[offer_tmp] + i, offers[offer_tmp] + i, keyset);

            full_subtract(tmp_sub, offers_cpy[offer_tmp], tmp_max, keyset);
            number_to_copy[offer_tmp] = new_LweSample_array(1, in_out_params);
            // if tmp sub + 15 = 1  it means that we're not on the good number so we add one to number_to_copy
            // if tmp sub + 15 = 0 we are on the good number, so we add zero
            bootsMUX(number_to_copy[offer_tmp], tmp_sub + 15, one, zero, keyset);
        }
        for (int offer_tmp = 0; offer_tmp < offerNbr; offer_tmp++)
        {
            for (int i = 0; i < 16; i++)
            {
                // here, if number to copy = 1, it means we are not on the random that is the max, so we just recopy that number in the list for the next turn.
                // if number to copy = 0, it means we are on the max, and therefor should not copy it back. So we add zeroes
                bootsMUX(offers[offer_tmp] + i, number_to_copy[offer_tmp], offers_cpy[offer_tmp] + i, zero, keyset);
            }
        }

        for (int i = 0; i < 16; i++)
        {
            bootsCOPY(&result[offer][i], &tmp_max[i], keyset);
        }
    }
}