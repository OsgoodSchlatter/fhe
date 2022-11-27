#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <time.h>

/***
 * can perform multiplication to 2 digits for a result of 15 bits max
 * @arg result: result that will be deciphered
 * @arg a: input 1
 * @arg b: input 2
 * @arg bk: keys
 *
 * @return encrypted result
 * ***/
LweSample *multiplier(LweSample *result, LweSample *a, LweSample *b, int nb_bits, const TFheGateBootstrappingCloudKeySet *bk);