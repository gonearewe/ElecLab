#include <complex.h>
#include <math.h>
#include "fft.h"

Complex y[1024];

void butterfly_transform(Complex y[], int len)
{
    int k;
    for (int i = 1, j = len / 2; i < len - 1; i++)
    {
        if (i < j)
        {
            Complex tmp = y[i];
            y[i] = y[j];
            y[j] = tmp;
        }
        // 交换互为小标反转的元素，i<j 保证交换一次
        // i 做正常的 + 1，j 做反转类型的 + 1，始终保持 i 和 j 是反转的
        k = len / 2;
        while (j >= k)
        {
            j = j - k;
            k = k / 2;
        }
        if (j < k)
            j += k;
    }
}

void fft(float x[], int len)
{
    for (int i = 0; i < len; i++)
    {
        y[i] = x[i];
    }

    butterfly_transform(y, len);
    for (int h = 2; h <= len; h <<= 1)
    {
        Complex wn = cosf(-2 * PI / h) + sinf(-2 * PI / h) * 1.0 * _Complex_I;
        for (int j = 0; j < len; j += h)
        {
            Complex w = 1.0f;
            for (int k = j; k < j + h / 2; k++)
            {
                Complex u = y[k];
                Complex t = w * y[k + h / 2];
                y[k] = u + t;
                y[k + h / 2] = u - t;
                w = w * wn;
            }
        }
    }

    for (int i = 0; i < len; i++)
    {
        x[i] = cabsf(y[i]);
    }
}
