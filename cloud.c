#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <time.h>
#include "./utils/full_adder_one_bit.c"
#include "./utils/multiplier.c"

void compare_bit(LweSample *result, const LweSample *a, const LweSample *b, const LweSample *lsb_carry, LweSample *tmp, const TFheGateBootstrappingCloudKeySet *bk)
{
    bootsXNOR(tmp, a, b, bk);
    bootsMUX(result, tmp, lsb_carry, a, bk);
}

// a1,a0 *b0
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

LweSample *diviser(LweSample *result, LweSample *dividende, LweSample *divisor, const TFheGateBootstrappingCloudKeySet *bk)
{
    LweSample *temp_result = new_gate_bootstrapping_ciphertext_array(16, bk);
    LweSample *temp_dividende = new_gate_bootstrapping_ciphertext_array(16, bk);
    LweSample *quotient = new_gate_bootstrapping_ciphertext_array(16, bk);
    LweSample *zero = new_gate_bootstrapping_ciphertext_array(1, bk);
    bootsCOPY(zero, 0, bk);
    LweSample *one = new_gate_bootstrapping_ciphertext_array(1, bk);
    bootsCOPY(one, 1, bk);

    // looping over 16 bits of dividende
    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < i; j++)
        {
            bootsCOPY(temp_dividende + j, dividende + 16 - i - j, bk);
        }

        full_substract(temp_result, temp_dividende, divisor, 16, bk);

        bootsMUX(quotient, temp_result + 15, zero, one, bk);
    }
}

// returns 4, 2 or 1 respectively wether a=b, a>b or a<b
void compare_bit_own(LweSample *result, LweSample *a, LweSample *b, const TFheGateBootstrappingCloudKeySet *bk)
{
    LweSample *inf_or_eq_or_sup = new_LweSample_array(3, bk->params->in_out_params);
    LweSample *temp = new_LweSample_array(2, bk->params->in_out_params);
    LweSample *not = new_LweSample_array(2, bk->params->in_out_params);
    // not a
    bootsNOT(not, a, bk);
    // not b
    bootsNOT(not +1, b, bk);
    bootsAND(temp, not, b, bk);
    bootsAND(temp + 1, a, not +1, bk);
    // a < b
    bootsCOPY(result, temp, bk);
    // a > b
    bootsCOPY(result + 1, temp + 1, bk);
    // a = b
    bootsNOR(result + 2, temp, temp + 1, bk);
}

void test(LweSample *result, LweSample *a, LweSample *b, const TFheGateBootstrappingCloudKeySet *bk)
{
    LweSample *temp = new_LweSample_array(16, bk->params->in_out_params);

    full_substract(temp, a, b, 16, bk);
    // for (int i = 0; i < 16; i++)
    // {
    //     bootsCOPY(result + i, temp + i, bk);
    // }
    bootsCOPY(result, temp + 1, bk);
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

    // ----------------------------------------- //

    // compare_bit_own(result, ciphertexts[0], ciphertexts[1], bk);
    test(result, ciphertexts[0], ciphertexts[1], bk);

    // ----------------------------------------- //

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
        delete_gate_bootstrapping_ciphertext_array(16, ciphertexts[i]);

    delete_gate_bootstrapping_cloud_keyset(bk);
}
