#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <time.h>

/***
 * completes the truth table of three inputs (a,b,c_in)
 * helpful in multiplier
 * serves as a half adder if c_in=0
 * @arg out: data that will be deciphered
 * @arg a: bit 1
 * @arg b: bit 2
 * @arg carry_in: carry_in bit
 *
 * @return sum and carry_out
 * ***/
LweSample *full_adder_one_bit(LweSample *out, LweSample *a, LweSample *b, LweSample *carry_in, const TFheGateBootstrappingCloudKeySet *bk)
{

    const LweParams *in_out_params = bk->params->in_out_params;
    // carries & result & temp
    LweSample *carry_out = new_LweSample(in_out_params);
    LweSample *result = new_LweSample(in_out_params);
    LweSample *temp = new_LweSample_array(3, in_out_params);
    // layer 1
    bootsXOR(temp, a, b, bk);
    bootsAND(temp + 1, a, b, bk);
    // layer 2
    bootsXOR(result, carry_in, temp, bk);
    bootsAND(temp + 2, carry_in, temp, bk);
    bootsOR(carry_out, temp + 1, temp + 2, bk);

    return (result, carry_out);
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
    full_adder_one_bit(result, ciphertexts[0], ciphertexts[1], ciphertexts[2], bk);
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
