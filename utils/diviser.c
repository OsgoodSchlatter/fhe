#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include "./offset.h"
#include "./full_subtract.h"
#include "./diviser.h"

/***
 * can perform euclidian division of 2 digits
 * @arg result: result that will be deciphered
 * @arg a: input 1
 * @arg b: input 2
 * @arg bk: keys
 *
 * @return encrypted result
 * ***/

int *diviser(LweSample *result, LweSample *dividende, LweSample *divisor, const TFheGateBootstrappingCloudKeySet *bk)
{
    LweSample *temp_rest = new_LweSample_array(16, bk->params->in_out_params);
    LweSample *temp_dividende = new_LweSample_array(16, bk->params->in_out_params);
    LweSample *temp_dividende_copy = new_LweSample_array(16, bk->params->in_out_params);
    LweSample *quotient = new_LweSample_array(16, bk->params->in_out_params);
    LweSample *zero = new_LweSample_array(1, bk->params->in_out_params);
    bootsCONSTANT(zero, 0, bk);
    LweSample *one = new_LweSample_array(1, bk->params->in_out_params);
    bootsCONSTANT(one, 1, bk);
    int iter = 16; // should be 16 when normal
    //  looping over 16 bits of dividende
    for (int i = 0; i < iter; i++)
    {
        // initialisation
        if (i == 0)
        {
            bootsCOPY(temp_dividende, dividende + iter - 1, bk);
            for (int i = 1; i < iter; i++)
            {
                bootsCOPY(temp_dividende + i, zero, bk);
            }
        }

        // here we substract our running dividende by the divisor, at each loop
        full_subtract(temp_rest, temp_dividende, divisor, bk);

        // the quotient is being added either 0 if dividende < divisor or 1 if opposite.
        // we check the 15th bit of temp_rest: it is 0 if dividende < divisor or 1 if opposite
        bootsMUX(quotient, temp_rest + iter - 1, zero, one, bk);

        // we offset by one the bit to change in quotient to modify its value.
        offset(quotient, quotient, 1, bk);

        // here we update temp_dividende's value
        // two cases:
        // -------------CASE A----------------
        // Either temp_dividende < divisor then temp_dividende's value should be increased by the next
        // bit of the dividende.
        // For that, we create temp_dividende_copy:
        for (int j = 0; j < i; j++)
        {
            bootsCOPY(temp_dividende_copy + j, temp_dividende + j, bk);
        }
        // we offset by one bit temp_dividende_copy to cater dividende next bit
        offset(temp_dividende_copy, temp_dividende_copy, 1, bk);
        bootsCOPY(temp_dividende_copy, dividende + iter - 1 - i, bk);

        // -------------CASE B---------------
        // here, temp_dividende > divisor then temp_dividende's value should be = to temp_rest
        // we offset by one bit temp_rest to cater dividende next bit, just like before
        offset(temp_rest, temp_rest, 1, bk);
        bootsCOPY(temp_rest, dividende + iter - 1 - i, bk);

        // then we must choose whether to copy temp_dividende_copy or temp_rest to temp_dividende.
        // we still choose based on the value of the 15th bit of temp_rest
        for (int j = 0; j < i; j++)
        {
            bootsMUX(temp_dividende + j, temp_rest + iter - 1, temp_dividende_copy + j, temp_rest + j, bk);
        }
    }

    // we subtract one last time and change the quotient accordingly.
    full_subtract(temp_rest, temp_dividende, divisor, bk);
    bootsMUX(quotient, temp_rest + iter - 1, zero, one, bk);

    // finally, we copy the quotient into the result, to be decrypted
    for (int i = 0; i < iter; i++)
    {
        bootsCOPY(result + i, quotient + i, bk);
    }
}