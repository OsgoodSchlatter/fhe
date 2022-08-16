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

LweSample *diviser(LweSample *result, LweSample *dividende, LweSample *divisor, const TFheGateBootstrappingCloudKeySet *bk)
{
    LweSample *temp_rest = new_LweSample_array(16, bk->params->in_out_params);
    LweSample *rest = new_LweSample_array(16, bk->params->in_out_params);
    LweSample *temp_dividende = new_LweSample_array(16, bk->params->in_out_params);
    LweSample *temp_dividende_copy = new_LweSample_array(16, bk->params->in_out_params);
    LweSample *quotient = new_LweSample_array(16, bk->params->in_out_params);
    LweSample *zero = new_LweSample_array(1, bk->params->in_out_params);
    bootsCONSTANT(zero, 0, bk);
    LweSample *one = new_LweSample_array(1, bk->params->in_out_params);
    bootsCONSTANT(one, 1, bk);

    // looping over 16 bits of dividende
    for (int i = 0; i < 16; i++)
    {
        // initialisation
        if (i == 0)
        {
            bootsCOPY(temp_dividende, dividende + 15, bk);
        }

        full_substract(temp_rest, temp_dividende, divisor, 16, bk);

        // the quotient is being added either 0 if dividende < divisor or 1 if opposite.
        // we check the 15th bit of temp_rest: it is 0 if dividende < divisor or 1 if opposite
        bootsMUX(quotient, temp_rest + 15, zero, one, bk);

        // we offset by one the bit to change in quotient to modify its value.
        offset(quotient, quotient, 1, bk);

        // here we update temp_dividende's value
        // two cases:
        // 1 - either dividende < divisor then temp_dividende's value should be increased by one
        // bit of the dividende
        for (int j = 0; j < i; j++)
        {
            bootsCOPY(temp_dividende_copy + j, temp_dividende + j, bk);
        }
        bootsCOPY(temp_dividende_copy + i, dividende + 15 - i, bk);

        // 2 - or dividende > divisor then temp_dividende's value should be = to temp_rest
        // nothing to prepare here

        // then we must choose whether to copy temp_dividende_copy or temp_rest to temp_dividende.

        for (int j = 0; j < i; j++)
        {
            bootsMUX(temp_dividende + j, temp_rest + 15, temp_rest + j, temp_dividende_copy + j, bk);
        }
    }
    for (int i = 0; i < 16; i++)
    {
        bootsCOPY(result + i, quotient + i, bk);
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
    LweSample *temp_dividende = new_LweSample_array(16, bk->params->in_out_params);
    for (int i = 0; i < 16; i++)
    {
        bootsCONSTANT(temp_dividende + i, i, bk);
    }

    bootsMUX(result, &a[0], &b[3], &temp_dividende[2], bk);
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
