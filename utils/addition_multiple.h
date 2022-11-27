#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <time.h>
#include "full_adder.h"

// performs addition on two ciphered data MY VERSION
void addition_multiple(LweSample *result, LweSample *offers[], int offerNbr,
                       const TFheGateBootstrappingCloudKeySet *keyset);