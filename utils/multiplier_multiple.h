#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <time.h>

void multiplier_multiple(LweSample *result, LweSample *offers[], int offerNbr, int nb_bits,
                         const TFheGateBootstrappingCloudKeySet *keyset);
