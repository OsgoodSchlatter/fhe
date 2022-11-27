#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <time.h>

// performs subtraction on many ciphered data MY VERSION
void subtraction_multiple(LweSample *result, LweSample *offers[], int offerNbr,
                          const TFheGateBootstrappingCloudKeySet *keyset);