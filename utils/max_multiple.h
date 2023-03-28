#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <string.h>
void max_multiple(LweSample *result, LweSample *offers[], int offerNbr,
                     const TFheGateBootstrappingCloudKeySet *keyset);

void max(LweSample *result, LweSample *x, LweSample *y, const TFheGateBootstrappingCloudKeySet *keyset);