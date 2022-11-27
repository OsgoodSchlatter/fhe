#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <time.h>

void diviser_multiple(LweSample *result, LweSample *offers[], int offerNbr,
                      const TFheGateBootstrappingCloudKeySet *keyset);