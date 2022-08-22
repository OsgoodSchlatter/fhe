#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <time.h>
#include "./utils/multiplier.c"

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
void full_subtract_one_bit(LweSample *subtract, LweSample *a, LweSample *b, LweSample *carry_in, const TFheGateBootstrappingCloudKeySet *bk)
{
    const LweParams *in_out_params = bk->params->in_out_params;
    // carries & result & temp
    LweSample *temp = new_LweSample_array(4, in_out_params);
    LweSample *not_a = new_LweSample_array(1, in_out_params);
    bootsNOT(not_a, a, bk);
    // layer 1
    bootsXOR(temp, a, b, bk);
    // computing temp_3 = not(temp_0)
    bootsNOT(temp + 3, temp, bk);
    bootsAND(temp + 1, not_a, b, bk);
    // layer 2
    bootsXOR(subtract, carry_in, temp, bk);
    bootsAND(temp + 2, carry_in, temp + 3, bk);
    bootsOR(carry_in, temp + 1, temp + 2, bk);
}

// performs substraction on two ciphered data MY VERSION
void full_subtract(LweSample *result, LweSample *x, LweSample *y,
                   const TFheGateBootstrappingCloudKeySet *keyset)
{
    const LweParams *in_out_params = keyset->params->in_out_params;
    // carries
    LweSample *carry = new_LweSample_array(2, in_out_params);
    // bootsSymEncrypt(carry, 0, keyset); // first carry initialized to 0
    bootsCONSTANT(carry, 0, keyset);

    for (int i = 0; i < 16; i++)
    {
        full_subtract_one_bit(result + i, x + i, y + i, carry, keyset);
    }
}

LweSample *diviser(LweSample *result, LweSample *dividende, LweSample *divisor, const TFheGateBootstrappingCloudKeySet *bk)
{
    LweSample *temp_rest = new_LweSample_array(16, bk->params->in_out_params);
    LweSample *rest = new_LweSample_array(16, bk->params->in_out_params);
    LweSample *temp_dividende = new_LweSample_array(16, bk->params->in_out_params);
    LweSample *size_temp_dividende = new_LweSample_array(16, bk->params->in_out_params);
    LweSample *temp_dividende_copy = new_LweSample_array(16, bk->params->in_out_params);
    LweSample *quotient = new_LweSample_array(16, bk->params->in_out_params);
    LweSample *zero = new_LweSample_array(1, bk->params->in_out_params);
    bootsCONSTANT(zero, 0, bk);
    LweSample *one = new_LweSample_array(1, bk->params->in_out_params);
    bootsCONSTANT(one, 1, bk);

    // // looping over 16 bits of dividende
    for (int i = 0; i < 16; i++)
    {
        // initialisation
        if (i == 0)
        {
            bootsCOPY(temp_dividende, dividende + 15, bk);
            for (int i = 1; i < 16; i++)
            {
                bootsCOPY(temp_dividende + i, zero, bk);
            }
        }

        full_subtract(temp_rest, temp_dividende, divisor, bk);

        // the quotient is being added either 0 if dividende < divisor or 1 if opposite.
        // we check the 15th bit of temp_rest: it is 0 if dividende < divisor or 1 if opposite
        bootsMUX(quotient, temp_rest + 15, zero, one, bk);

        // we offset by one the bit to change in quotient to modify its value.
        offset(quotient, quotient, 1, bk);

        // here we update temp_dividende's value
        // two cases:
        // -------------CASE A----------------
        // Either temp_dividende < divisor then temp_dividende's value should be increased by the next
        // bit of the dividende.
        // For that, we create temp_dividende_copy:
        for (int j = 0; j < i; j++)
        {
            bootsCOPY(temp_dividende_copy + j, temp_dividende + j, bk);
        }
        // temp_dividende_copy + i is wrong, it should be + size of temp_dividende
        bootsCOPY(temp_dividende_copy + i, dividende + 15 - i, bk);

        // -------------CASE B---------------
        // or temp_dividende > divisor then temp_dividende's value should be = to temp_rest
        // nothing to prepare here

        // then we must choose whether to copy temp_dividende_copy or temp_rest to temp_dividende.

        for (int j = 0; j < i; j++)
        {
            bootsMUX(temp_dividende + j, temp_rest + 15, temp_dividende_copy + j, temp_rest + j, bk);
        }
    }
    for (int i = 0; i < 16; i++)
    {
        bootsCOPY(result + i, quotient + i, bk);
    }
}

void test(LweSample *result, LweSample *a, LweSample *b, const TFheGateBootstrappingCloudKeySet *bk)
{
    LweSample *temp_dividende = new_LweSample_array(16, bk->params->in_out_params);
    LweSample *temp = new_LweSample_array(16, bk->params->in_out_params);
    LweSample *zero = new_LweSample_array(1, bk->params->in_out_params);
    bootsCONSTANT(zero, 0, bk);

    bootsCOPY(temp_dividende, a + 15, bk);
    for (int i = 1; i < 16; i++)
    {
        bootsCOPY(temp_dividende + i, zero, bk);
    }
    full_subtract(temp, temp_dividende, b, bk);

    bootsCOPY(result, temp + 15, bk);
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
    diviser(result, ciphertexts[0], ciphertexts[1], bk);

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
