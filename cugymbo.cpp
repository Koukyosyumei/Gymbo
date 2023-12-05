#include <stdio.h>
#include <stdlib.h>
#include "libcugymbo/gdcuda.h"

int main()
{
    printf("Hello\n");

    const int n = 10;
    int *in = new int[n];
    int *out = new int[n];
    int *answer = new int[n];

    for (int i = 0; i < n; i++) in[i] = rand() % 100;
    for (int i = 0; i < n; i++) answer[i] = in[i] * 2;

    vecDouble(in, out, n);

    int i;
    for (i = 0; i < n; i++) {
        if (answer[i] != out[i]) {
            printf("error at index = %d\n", i);
            break;
        }
    }
    printf("OK\n");

    delete[] in;
    delete[] out;
    delete[] answer;

    return 0;
}

