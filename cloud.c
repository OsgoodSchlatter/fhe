#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "./utils/multiplier.c"
#include "./utils/diviser.c"

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

    printf("......computation took: %f secs\n", (end_time - start_time) / pow(10, 6));
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
