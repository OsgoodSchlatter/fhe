#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>

/***
 * can offset by *offset* bits an array of encrypted data
 * @arg result: result that will be deciphered
 * @arg input: array to offset
 * @arg offset: number of bits user wants to offset their array
 * @arg bk: keys
 *
 * @return encrypted offset array (i.e. var result)
 * ***/
LweSample *offset(LweSample *result, LweSample *input, int offset, const TFheGateBootstrappingCloudKeySet *bk);