#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>

/***
 * can perform euclidian division of 2 digits
 * @arg result: result that will be deciphered
 * @arg a: input 1
 * @arg b: input 2
 * @arg bk: keys
 *
 * @return encrypted result
 * ***/

int *diviser(LweSample *result, LweSample *dividende, LweSample *divisor, const TFheGateBootstrappingCloudKeySet *bk);
