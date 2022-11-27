#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <time.h>

// performs substraction on two ciphered data MY VERSION
void full_subtract(LweSample *result, LweSample *x, LweSample *y,
                   const TFheGateBootstrappingCloudKeySet *keyset);