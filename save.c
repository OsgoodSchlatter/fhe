#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <time.h>

LweSample *copy(LweSample *result, LweSample *input, int size, const TFheGateBootstrappingCloudKeySet *bk)
{
    for (int i = 0; i < size; i++)
    {
        bootsCOPY(&result[i], &input[i], bk);
    }
}

/***
 * completes the truth table of three inputs (a,b,c_in)
 * helpful in multiplier
 * serves as a half adder if c_in=0
 * @arg sum: sum of two bits
 * @arg a: bit 1
 * @arg b: bit 2
 * @arg carry_in: carry_in bit that will be turned into carry_out. Then, it will be carry_in for next full_adder
 *
 * ***/
void full_adder_one_bit(LweSample *sum, LweSample *a, LweSample *b, LweSample *carry_in, const TFheGateBootstrappingCloudKeySet *bk)
{

    const LweParams *in_out_params = bk->params->in_out_params;
    // carries & result & temp
    LweSample *temp = new_LweSample_array(3, in_out_params);
    // layer 1
    bootsXOR(temp, a, b, bk);
    bootsAND(temp + 1, a, b, bk);
    // layer 2
    bootsXOR(sum, carry_in, temp, bk);
    bootsAND(temp + 2, carry_in, temp, bk);
    bootsOR(carry_in, temp + 1, temp + 2, bk);
}

/***
 * Enables user to choose one bit
 * @arg result: deciphered version of a
 * @arg a: ciphered input
 * @arg index: the bit user wants to check
 *
 *
 * @return 2^index or 0 depending wether the input contains this bit
 * ***/
LweSample *test(LweSample *result, LweSample *a, int index, const TFheGateBootstrappingCloudKeySet *bk)
{
    LweSample *zero = new_gate_bootstrapping_ciphertext_array(16, bk->params);
    bootsCONSTANT(zero, 0, bk);
    for (int i = 0; i < 16; i++)
    {
        if (i == index)
        {
            bootsCOPY(&result[i], &a[i], bk);
        }
        else
        {
            bootsCOPY(&result[i], &zero[i], bk);
        }
    }

    bootsCOPY(&result[index], &a[index], bk);
}

void half_adder(LweSample *sum, const LweSample *x, const LweSample *y, const LweSample *carry_out, const int32_t nb_bits,
                const TFheGateBootstrappingCloudKeySet *keyset)
{
    for (int32_t i = 0; i < nb_bits; ++i)
    {
        bootsXOR(sum + i, x + i, y + i, keyset);   // sum = xi XOR yi
        bootsAND(carry_out, x + i, y + i, keyset); // carry_out
    }
}

/***
 * Adds 2 ciphered data to a max 16 bits threshold
 * @arg sum: the sum of two inputs
 * @arg x: input 1
 * @arg y: input 2
 * @arg nb_bits: length of max(x,y)
 *
 * @return encrypted result
 * ***/
void full_adder(LweSample *sum, const LweSample *x, const LweSample *y, const int32_t nb_bits,
                const TFheGateBootstrappingCloudKeySet *keyset)
{
    const LweParams *in_out_params = keyset->params->in_out_params;
    // carries
    LweSample *carry = new_LweSample_array(2, in_out_params);
    // first carry initialized to 0
    bootsCONSTANT(carry, 0, keyset);
    LweSample *temp = new_LweSample_array(3, in_out_params);

    for (int32_t i = 0; i < nb_bits; ++i)
    {
        // sumi = xi XOR yi XOR carry(i-1)
        bootsXOR(temp, x + i, y + i, keyset); // temp = xi XOR yi
        bootsXOR(sum + i, temp, carry, keyset);

        // carry = (xi AND yi) XOR (carry(i-1) AND (xi XOR yi))
        bootsAND(temp + 1, x + i, y + i, keyset); // temp1 = xi AND yi
        bootsAND(temp + 2, carry, temp, keyset);  // temp2 = carry AND temp
        bootsXOR(carry + 1, temp + 1, temp + 2, keyset);
        bootsCOPY(carry, carry + 1, keyset);
    }
    bootsCOPY(sum, carry, keyset);

    delete_LweSample_array(3, temp);
    delete_LweSample_array(2, carry);
}

// performs substraction on two ciphered data
void full_substract(LweSample *sum, const LweSample *x, const LweSample *y, const int32_t nb_bits,
                    const TFheGateBootstrappingCloudKeySet *keyset)
{
    const LweParams *in_out_params = keyset->params->in_out_params;
    // carries
    LweSample *carry = new_LweSample_array(2, in_out_params);
    // bootsSymEncrypt(carry, 0, keyset); // first carry initialized to 0
    bootsCONSTANT(carry, 0, keyset);
    LweSample *temp = new_LweSample_array(3, in_out_params);

    LweSample *not_x = new_LweSample_array(4, in_out_params);
    LweSample *not_temp = new_LweSample_array(3, in_out_params);

    for (int32_t i = 0; i < nb_bits; ++i)
    {
        // sumi = xi XOR yi XOR carry(i-1)
        bootsXOR(temp, x + i, y + i, keyset); // temp = xi XOR yi
        bootsXOR(sum + i, temp, carry, keyset);

        // carry = (xi AND yi) XOR (carry(i-1) AND (xi XOR yi))

        bootsNOT(not_x, x + i, keyset);
        bootsNOT(not_temp, temp, keyset);

        bootsAND(temp + 1, not_x, y + i, keyset);    // temp1 = xi AND yi
        bootsAND(temp + 2, carry, not_temp, keyset); // temp2 = carry AND temp
        bootsXOR(carry + 1, temp + 1, temp + 2, keyset);
        bootsCOPY(carry, carry + 1, keyset);
    }
    bootsCOPY(sum, carry, keyset);

    delete_LweSample_array(3, temp);
    delete_LweSample_array(2, carry);
}

/***
 * Adds 2 or more ciphered offers
 * @arg offers: vector of FHE offers
 * @arg offerNbr: the number of offers
 *
 * @return encrypted result
 * ***/
LweSample *addition_multiple(LweSample *result, LweSample *offers[], int offerNbr, int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
{
    LweSample *tmp = new_gate_bootstrapping_ciphertext_array(16, bk->params);
    full_adder(tmp, offers[1], offers[0], nb_bits, bk);

    for (int index = 2; index < offerNbr; index++)
    {
        for (int j = 0; j < nb_bits; j++)
        {
            bootsCOPY(&result[j], &tmp[j], bk);
        }
        full_adder(tmp, result, offers[index], nb_bits, bk);
    }
    for (int i = 0; i < nb_bits; i++)
    {
        bootsCOPY(&result[i], &tmp[i], bk);
    }
}

/***
 * Substracts 2 or more ciphered offers
 * see [1] in function for further details
 * @arg offers: vector of FHE offers
 * @arg offerNbr: the number of offers
 *
 *
 * @return encrypted result
 * ***/
LweSample *substraction_multiple(LweSample *result, LweSample *offers[], int offerNbr, int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
{
    LweSample *tmp = new_gate_bootstrapping_ciphertext_array(16, bk->params);

    full_substract(tmp, offers[0], offers[1], 16, bk);

    for (int index = 2; index < offerNbr; index++)
    {
        for (int j = 0; j < nb_bits; j++)
        {
            bootsCOPY(&result[j], &tmp[j], bk);
        }
        full_substract(tmp, result, offers[index], 16, bk);
    }
    for (int i = 0; i < nb_bits; i++)
    {
        bootsCOPY(&result[i], &tmp[i], bk);
    }
}

void compare_bit(LweSample *result, const LweSample *a, const LweSample *b, const LweSample *lsb_carry, LweSample *tmp, const TFheGateBootstrappingCloudKeySet *bk)
{
    bootsXNOR(tmp, a, b, bk);
    bootsMUX(result, tmp, lsb_carry, a, bk);
}

void minimum(LweSample *result, const LweSample *a, const LweSample *b, const int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
{
    LweSample *tmps = new_gate_bootstrapping_ciphertext_array(2, bk->params);

    // initialize the carry to 0
    bootsCONSTANT(&tmps[0], 0, bk);
    // run the elementary comparator gate n times
    for (int i = 0; i < nb_bits; i++)
    {
        compare_bit(&tmps[0], &b[i], &a[i], &tmps[0], &tmps[1], bk);
    }
    // tmps[0] is the result of the comparaison: 0 if a is larger, 1 if b is larger
    // select the max and copy it to the result
    for (int i = 0; i < nb_bits; i++)
    {
        bootsMUX(&result[i], &tmps[0], &b[i], &a[i], bk);
    }

    delete_gate_bootstrapping_ciphertext_array(2, tmps);
}

LweSample *offset(LweSample *result, LweSample *input, int offset, const TFheGateBootstrappingCloudKeySet *bk)
{
    LweSample *offsetArray = new_gate_bootstrapping_ciphertext_array(16, bk->params);
    for (int i = 0; i < offset; i++)
    {
        bootsCONSTANT(&offsetArray[i], 0, bk);
    }

    for (int i = 0; i < 16 - offset; i++)
    {
        bootsCOPY(&offsetArray[i + offset], &input[i], bk);
    }

    for (int i = 0; i < 16; i++)
    {
        bootsCOPY(&result[i], &offsetArray[i], bk);
    }
}

/***
 * can perform multiplication to 2 digits of 4 bits max (max = 15*15 = 225)
 * @arg result: result that will be deciphered
 * @arg a: input 1
 * @arg b: input 2
 * @arg bk: keys
 *
 * @return encrypted result
 * ***/
LweSample *multiplier_4_bits(LweSample *result, LweSample *a, LweSample *b, const TFheGateBootstrappingCloudKeySet *bk)
{
    // final array containing the result (8 bits max)
    LweSample *z = new_LweSample_array(8, bk->params->in_out_params);
    // temp values [4,4]
    LweSample *temp[4];
    for (int i = 0; i < 4; i++)
    {
        temp[i] = new_LweSample_array(4, bk->params->in_out_params);
    }
    // if we want to factorise operations, we might want to create an array of 3. (layer 1, 2, 3)
    LweSample *carry = new_LweSample_array(4, bk->params->in_out_params);

    // first carry initialised to 0
    for (int i = 0; i < 4; i++)
        bootsCONSTANT(carry + i, 0, bk);

    // computing each AND gate
    for (int line = 0; line < 4; line++)
        for (int i = 0; i < 4; i++)
            bootsAND(temp[line] + i, a + i, b + line, bk);

    // copying first 4 gates so that it can be looped afterwards
    for (int i = 0; i < 4; i++)
        bootsCOPY(z + i, &temp[0][i], bk);

    // processing main calcul
    for (int i = 0; i < 3; i++)
    {
        bootsCOPY(z + i + 4, carry + i, bk);
        for (int k = 0; k < 4; k++)
            full_adder_one_bit(z + k + i + 1, &temp[i + 1][k], z + k + i + 1, carry + i + 1, bk);
    }

    // cant fit int the loop
    bootsCOPY(z + 7, carry + 3, bk);

    // copying into result
    for (int i = 0; i < 8; i++)
        bootsCOPY(&result[i], z + i, bk);
}

int main()
{

    printf("reading the key...\n");

    // reads the cloud key from file
    FILE *cloud_key = fopen("cloud.key", "rb");
    TFheGateBootstrappingCloudKeySet *bk = new_tfheGateBootstrappingCloudKeySet_fromFile(cloud_key);
    fclose(cloud_key);

    // if necessary, the params are inside the key
    const TFheGateBootstrappingParameterSet *params = bk->params;

    printf("reading the input...\n");

    // receiving inputs from alice
    int numInputs;
    printf("input = \n");
    scanf("%d", &numInputs);
    FILE *cloud_data = fopen("cloud.data", "rb");

    LweSample *ciphertexts[numInputs];
    for (int i = 0; i < numInputs; i++)
    {
        ciphertexts[i] = new_gate_bootstrapping_ciphertext_array(16, params);
        for (int j = 0; j < 16; j++)
            import_gate_bootstrapping_ciphertext_fromFile(cloud_data, &ciphertexts[i][j], params);
    }

    fclose(cloud_data);

    // do some operations on the ciphertexts
    LweSample *result = new_gate_bootstrapping_ciphertext_array(16, params);
    time_t start_time = clock();
    // addition_multiple(result, ciphertexts, numInputs, 16, bk);
    // substraction_multiple(result, ciphertexts, numInputs, 16, bk);
    // mult_2_bits(result, ciphertexts[0], ciphertexts[1], 16, bk);
    // test(result, ciphertexts[0], 5, bk);
    // offset(result, ciphertexts[0], 2, bk);
    mult_large_inputs(result, ciphertexts, 2, bk);
    time_t end_time = clock();

    printf("......computation of the 16 binary + 32 mux gates took: %ld microsecs\n", end_time - start_time);

    printf("writing the answer to file...\n");

    // export the 32 ciphertexts to a file (for the cloud)
    FILE *answer_data = fopen("answer.data", "wb");
    for (int i = 0; i < 16; i++)
        export_gate_bootstrapping_ciphertext_toFile(answer_data, &result[i], params);
    fclose(answer_data);

    // clean up all pointers
    delete_gate_bootstrapping_ciphertext_array(16, result);
    for (int i = 0; i < numInputs; i++)
    {
        delete_gate_bootstrapping_ciphertext_array(16, ciphertexts[i]);
    }

    delete_gate_bootstrapping_cloud_keyset(bk);
}
