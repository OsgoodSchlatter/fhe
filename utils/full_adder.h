#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <time.h>

// performs addition on two ciphered data MY VERSION
void full_adder(LweSample *result, LweSample *x, LweSample *y,
                const TFheGateBootstrappingCloudKeySet *keyset);