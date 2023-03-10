#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    // generate a keyset
    const int minimum_lambda = 110;
    TFheGateBootstrappingParameterSet *params = new_default_gate_bootstrapping_parameters(minimum_lambda);

    // generate a random key
    uint32_t seed[] = {314, 1592, 657};
    tfhe_random_generator_setSeed(seed, 3);
    TFheGateBootstrappingSecretKeySet *key = new_random_gate_bootstrapping_secret_keyset(params);

    // asking user values
    if (argc < 3)
    {
        printf("format should be ./alice <1st input> <2nd input> ... \n");
        return -1;
    }
    int numInputs = argc - 1;
    int array[numInputs];
    for (int i = 0; i < numInputs; i++)
    {
        array[i] = strtol(argv[i + 1], NULL, 10);
    }

    printf("your values are  \n");
    for (int i = 0; i < numInputs; i++)
    {
        printf("%d\n", array[i]);
    }

    // creating the array with the values now ciphered with the bootsSymEncrypt function.
    LweSample *ciphertexts[numInputs];
    for (int i = 0; i < numInputs; i++)
    {
        ciphertexts[i] = new_gate_bootstrapping_ciphertext_array(16, params);
        for (int j = 0; j < 16; j++)
        {
            bootsSymEncrypt(&ciphertexts[i][j], (array[i] >> j) & 1, key);
        }
    }

    // printf("Hi there! Today, I will ask the cloud what is the minimum between %d and %d\n", plaintext1, plaintext2);

    // export the secret key to file for later use
    FILE *secret_key = fopen("secret.key", "wb");
    export_tfheGateBootstrappingSecretKeySet_toFile(secret_key, key);
    fclose(secret_key);

    // export the cloud key to a file (for the cloud)
    FILE *cloud_key = fopen("cloud.key", "wb");
    export_tfheGateBootstrappingCloudKeySet_toFile(cloud_key, &key->cloud);
    fclose(cloud_key);

    // export the 32 ciphertexts to a file (for the cloud)
    FILE *cloud_data = fopen("cloud.data", "wb");

    for (int i = 0; i < numInputs; i++)
    {
        for (int j = 0; j < 16; j++)
            export_gate_bootstrapping_ciphertext_toFile(cloud_data, &ciphertexts[i][j], params);
    }
    fclose(cloud_data);

    // clean up all pointers
    for (int i = 0; i < numInputs; i++)
    {
        delete_gate_bootstrapping_ciphertext_array(16, ciphertexts[i]);
    }

    delete_gate_bootstrapping_secret_keyset(key);
    delete_gate_bootstrapping_parameters(params);
}
