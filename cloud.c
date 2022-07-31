#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <time.h>

/***
 * completes the truth table of three inputs (a,b,c_in)
 * helpful in multiplier
 * serves as a half adder if c_in=0
 * @arg sum: sum of two bits
 * @arg a: bit 1
 * @arg b: bit 2
 * @arg carry_in: carry_in bit that will be turned into carry_out. Then, it will be carry_in for next full_adder
 *
 * @return sum and carry_out
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

LweSample *multiplier_4_bits(LweSample *result, LweSample *a, LweSample *b, const TFheGateBootstrappingCloudKeySet *bk)
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

    // cant fit int the loop
    bootsCOPY(z + 15, carry + 7, bk);

    // copying into result
    for (int i = 0; i < 16; i++)
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

    // ----------------------------------------- //
    multiplier_4_bits(result, ciphertexts[0], ciphertexts[1], bk);
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
