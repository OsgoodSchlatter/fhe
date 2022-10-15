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
LweSample *offset(LweSample *result, LweSample *input, int offset, const TFheGateBootstrappingCloudKeySet *bk)
{
    LweSample *offsetArray = new_gate_bootstrapping_ciphertext_array(16, bk->params);
    for (int i = 0; i < offset; i++)
    {
        bootsCONSTANT(&offsetArray[i], 0, bk);
    }

    for (int i = 0; i < 16 - offset; i++)
    {
        bootsCOPY(&offsetArray[i + offset], &input[i], bk);
    }

    for (int i = 0; i < 16; i++)
    {
        bootsCOPY(&result[i], &offsetArray[i], bk);
    }
}