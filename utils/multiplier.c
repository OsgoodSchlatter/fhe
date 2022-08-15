#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <time.h>

/***
 * can perform multiplication to 2 digits for a result of 15 bits max
 * @arg result: result that will be deciphered
 * @arg a: input 1
 * @arg b: input 2
 * @arg bk: keys
 *
 * @return encrypted result
 * ***/
LweSample *multiplier(LweSample *result, LweSample *a, LweSample *b, const TFheGateBootstrappingCloudKeySet *bk)
{
    // final array containing the result (8 bits max)
    LweSample *z = new_LweSample_array(16, bk->params->in_out_params);
    // temp values [m,m], m = size of max(a,b)
    LweSample *temp[8];
    for (int i = 0; i < 8; i++)
    {
        temp[i] = new_LweSample_array(8, bk->params->in_out_params);
    }
    // if we want to factorise operations, we might want to create an array of 3. (layer 1, 2, 3)
    LweSample *carry = new_LweSample_array(8, bk->params->in_out_params);

    // first carry initialised to 0
    for (int i = 0; i < 8; i++)
        bootsCONSTANT(carry + i, 0, bk);

    // computing each AND gate
    for (int line = 0; line < 8; line++)
        for (int i = 0; i < 8; i++)
            bootsAND(temp[line] + i, a + i, b + line, bk);

    // copying first 4 gates so that it can be looped afterwards
    for (int i = 0; i < 8; i++)
        bootsCOPY(z + i, &temp[0][i], bk);

    // processing main calcul
    for (int i = 0; i < 7; i++)
    {
        bootsCOPY(z + i + 8, carry + i, bk);
        for (int k = 0; k < 8; k++)
            full_adder_one_bit(z + k + i + 1, &temp[i + 1][k], z + k + i + 1, carry + i + 1, bk);
    }

    // cant fit into the loop
    bootsCOPY(z + 15, carry + 7, bk);

    // copying into result
    for (int i = 0; i < 16; i++)
        bootsCOPY(&result[i], z + i, bk);
}
