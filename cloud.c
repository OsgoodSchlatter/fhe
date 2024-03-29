#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "utils/multiplier_multiple.h"
#include "utils/subtraction_multiple.h"
#include "utils/addition_multiple.h"
#include "utils/diviser_multiple.h"
#include "utils/max_multiple.h"

int main(int agrc, char **argv)
{
    // printf("input should be ./cloud <number of inputs in alice.c> <choice of calculation> <optional: nb of bits>\n");
    // printf("reading the key...\n");

    // reads the cloud key from file
    FILE *cloud_key = fopen("cloud.key", "rb");
    TFheGateBootstrappingCloudKeySet *bk = new_tfheGateBootstrappingCloudKeySet_fromFile(cloud_key);
    fclose(cloud_key);

    // if necessary, the params are inside the key
    const TFheGateBootstrappingParameterSet *params = bk->params;

    // printf("reading the input...\n");

    // receiving inputs from alice
    int numInputs = strtol(argv[1], NULL, 10);
    int choice = strtol(argv[2], NULL, 10);
    // by default, nb_bits is 16.
    int nb_bits = 16;
    if (argv[3])
    {
        nb_bits = strtol(argv[3], NULL, 10);
    }
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

    switch (choice)
    {
    case 1:
        multiplier_multiple(result, ciphertexts, numInputs, nb_bits, bk);
        break;

    case 2:
        diviser_multiple(result, ciphertexts, numInputs, bk);
        break;

    case 3:
        addition_multiple(result, ciphertexts, numInputs, bk);
        break;

    case 4:
        subtraction_multiple(result, ciphertexts, numInputs, bk);
        break;

    case 5:
        max_multiple(result, ciphertexts, numInputs, bk);
        break;

    case 6:
        for (int i = 0; i < 16; i++)
            bootsCOPY(result + i, ciphertexts[1] + i, bk);

    default:
        break;
    }

    // ------------------------------------------ //

    time_t end_time = clock();

    // printf("......computation took: %f secs\n", (end_time - start_time) / pow(10, 6));
    // printf("writing the answer to file...\n");
    printf("%f\n", (end_time - start_time) / pow(10, 6));

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
