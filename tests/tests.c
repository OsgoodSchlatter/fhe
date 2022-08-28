#include <assert.h>
#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

char *truncString(char *str, int pos)
{
    size_t len = strlen(str);

    if (len > abs(pos))
    {
        if (pos > 0)
            str = str + pos;
        else
            str[len + pos] = 0;
    }

    return str;
}

// executable scripts should be already generated
int main()
{
    // -------------------
    // ROUND 1
    // -------------------
    char result[40];
    int int_result;
    FILE *cmd;
    // test for multiplication
    system("../alice 120 2");
    system("../cloud 2 1");
    cmd = popen("../verif", "r");
    while (fgets(result, sizeof(result), cmd) != NULL)
        printf("%s\n", result);
    pclose(cmd);
    int_result = atoi(truncString(strdup(result), 19));
    assert(240 == int_result);
    printf("-------------------------\n");
    printf("test for multiplication was succesfully passed for round 1\n");
    printf("-------------------------\n\n");

    // test for division
    system("../alice 1020 5");
    system("../cloud 2 2");
    cmd = popen("../verif", "r");
    while (fgets(result, sizeof(result), cmd) != NULL)
        printf("%s\n", result);
    pclose(cmd);
    int_result = atoi(truncString(strdup(result), 19));
    assert(204 == int_result);
    printf("-------------------------\n");
    printf("test for division was succesfully passed for round 1\n");
    printf("-------------------------\n \n");

    // test for addition
    system("../cloud 2 3");
    cmd = popen("../verif", "r");
    while (fgets(result, sizeof(result), cmd) != NULL)
        printf("%s\n", result);
    pclose(cmd);
    int_result = atoi(truncString(strdup(result), 19));
    assert(1025 == int_result);
    printf("-------------------------\n\n");
    printf("test for addition was succesfully passed for round 1\n");
    printf("-------------------------\n\n");

    // test for division
    system("../cloud 2 4");
    cmd = popen("../verif", "r");
    while (fgets(result, sizeof(result), cmd) != NULL)
        printf("%s\n", result);
    pclose(cmd);
    int_result = atoi(truncString(strdup(result), 19));
    assert(1015 == int_result);
    printf("-------------------------\n\n");
    printf("test for subtraction was succesfully passed for round 1\n");
    printf("-------------------------\n\n");

    // -------------------
    // ROUND 2
    // -------------------

    // test for multiplication
    system("../alice 121 2");
    system("../cloud 2 1");
    cmd = popen("../verif", "r");
    while (fgets(result, sizeof(result), cmd) != NULL)
        printf("%s\n", result);
    pclose(cmd);
    int_result = atoi(truncString(strdup(result), 19));
    assert(242 == int_result);
    printf("-------------------------\n");
    printf("test for multiplication was succesfully passed for round 2\n");
    printf("-------------------------\n\n");

    // test for division
    system("../alice 30646 645");
    system("../cloud 2 2");
    cmd = popen("../verif", "r");
    while (fgets(result, sizeof(result), cmd) != NULL)
        printf("%s\n", result);
    pclose(cmd);
    int_result = atoi(truncString(strdup(result), 19));
    assert(47 == int_result);
    printf("-------------------------\n\n");
    printf("test for division was succesfully passed for round 2\n");
    printf("-------------------------\n\n");

    // test for addition
    system("../cloud 2 3");
    cmd = popen("../verif", "r");
    while (fgets(result, sizeof(result), cmd) != NULL)
        printf("%s\n", result);
    pclose(cmd);
    int_result = atoi(truncString(strdup(result), 19));
    assert(31291 == int_result);
    printf("-------------------------\n\n");
    printf("test for addition was succesfully passed for round 2\n");
    printf("-------------------------\n\n");

    // test for division
    system("../cloud 2 4");
    cmd = popen("../verif", "r");
    while (fgets(result, sizeof(result), cmd) != NULL)
        printf("%s\n", result);
    pclose(cmd);
    int_result = atoi(truncString(strdup(result), 19));
    assert(30001 == int_result);
    printf("-------------------------\n\n");
    printf("test for subtraction was succesfully passed for round 2\n");
    printf("-------------------------\n\n");
}
