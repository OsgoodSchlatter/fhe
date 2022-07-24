#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <time.h>

// performs addition on two ciphered data
void full_adder(LweSample *sum, const LweSample *x, const LweSample *y, const int32_t nb_bits,
                const TFheGateBootstrappingCloudKeySet *keyset)
{
    const LweParams *in_out_params = keyset->params->in_out_params;
    // carries
    LweSample *carry = new_LweSample_array(2, in_out_params);
    // bootsSymEncrypt(carry, 0, keyset); // first carry initialized to 0
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

    full_adder(tmp, offers[0], offers[1], 16, bk);

    for (int index = 2; index < offerNbr; index++)
    {
        for (int j = 0; j < nb_bits; j++)
        {
            bootsCOPY(&result[j], &tmp[j], bk);
        }
        full_adder(tmp, result, offers[index], 16, bk);
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
        compare_bit(&tmps[0], &a[i], &b[i], &tmps[0], &tmps[1], bk);
    }
    // tmps[0] is the result of the comparaison: 0 if a is larger, 1 if b is larger
    // select the max and copy it to the result
    for (int i = 0; i < nb_bits; i++)
    {
        bootsMUX(&result[i], &tmps[0], &b[i], &a[i], bk);
    }

    delete_gate_bootstrapping_ciphertext_array(2, tmps);
}

// a1,a0 * b1,b0
LweSample *mult_2_bits(LweSample *result, LweSample *a, LweSample *b, int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
{
    LweSample *out0 = new_gate_bootstrapping_ciphertext_array(4, bk->params);

    LweSample *temp0 = new_gate_bootstrapping_ciphertext_array(2, bk->params);
    LweSample *temp1 = new_gate_bootstrapping_ciphertext_array(2, bk->params);
    LweSample *temp2 = new_gate_bootstrapping_ciphertext_array(2, bk->params);
    LweSample *temp3 = new_gate_bootstrapping_ciphertext_array(2, bk->params);

    // out0
    bootsAND(&out0[0], &a[0], &b[0], bk);

    // Pre out1
    bootsAND(&temp0[0], &a[0], &b[1], bk);
    bootsAND(&temp1[0], &a[1], &b[0], bk);
    // out1
    bootsXOR(&out0[1], &temp0[0], &temp1[0], bk);

    // Pre out2 & out3
    bootsAND(&temp2[0], &temp0[0], &temp1[0], bk);

    bootsAND(&temp3[0], &a[1], &b[1], bk);
    // out2
    bootsXOR(&out0[2], &temp2[0], &temp3[0], bk);
    // out3
    bootsAND(&out0[3], &temp3[0], &temp2[0], bk);

    for (int i = 0; i < 4; i++)
    {
        bootsCOPY(&result[i], &out0[i], bk);
    }
}

// a1,a0 *b0

LweSample *mult(LweSample *result, LweSample *a, LweSample *b, const TFheGateBootstrappingCloudKeySet *bk)
{
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
    test(result, ciphertexts[0], 5, bk);
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
