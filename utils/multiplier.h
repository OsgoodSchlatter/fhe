#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <time.h>
#include "./full_adder_one_bit.h"

/***
 * can perform multiplication to 2 digits for a result of 15 bits max
 * @arg result: result that will be deciphered
 * @arg a: input 1
 * @arg b: input 2
 * @arg bk: keys
 *
 * @return encrypted result
 * ***/
LweSample *multiplier(LweSample *result, LweSample *a, LweSample *b, int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
{
    // final array containing the result (16 bits max, see result)
    LweSample *z = new_LweSample_array(2 * nb_bits, bk->params->in_out_params);
    // temp values [m,m], m = size of max(a,b)
    LweSample *temp[nb_bits];
    for (int i = 0; i < nb_bits; i++)
    {
        temp[i] = new_LweSample_array(nb_bits, bk->params->in_out_params);
    }
    // if we want to factorise operations, we might want to create an array of 3. (layer 1, 2, 3)
    LweSample *carry = new_LweSample_array(nb_bits, bk->params->in_out_params);

    // first carry initialised to 0
    for (int i = 0; i < nb_bits; i++)
        bootsCONSTANT(carry + i, 0, bk);

    // computing each AND gate
    for (int line = 0; line < nb_bits; line++)
        for (int row = 0; row < nb_bits; row++)
            bootsAND(temp[line] + row, a + row, b + line, bk);

    // copying first 8 AND gates (1st line) so that it can fit in the loop afterwards
    for (int i = 0; i < nb_bits; i++)
        bootsCOPY(z + i, &temp[0][i], bk);

    // processing main calcul
    for (int i = 1; i < nb_bits; i++)
    {
        bootsCOPY(z + i + nb_bits - 1, carry + i, bk);
        for (int k = 0; k < nb_bits; k++)
            full_adder_one_bit(z + k + i, &temp[i][k], z + k + i, carry + i, bk);
    }

    // copying into result
    for (int i = 0; i < nb_bits; i++)
        bootsCOPY(&result[i], z + i, bk);
}
