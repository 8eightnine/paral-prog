#include "pthread.h"
#include <iostream>
#include <time.h>
#pragma comment(lib, "pthreadVCE2.lib")

void* CentralRect(void* all_param);
double f(double x);

struct Param
{
    double a;
    double b;
    int n;
    double sum;
};


int main(void) {
    double a = 10;
    double b = 100000000000000;
    int n = 100000000;
    const int n_threads = 1;

    std::cout << "Threads count:" << n_threads << std::endl;
    struct Param arr_parall[n_threads];
    pthread_t p[n_threads];

    double start = clock();
    double b_h = (b - a) / n_threads;

    for (int i = 0; i < n_threads; i++) {
        arr_parall[i].a = a + b_h * i;
        arr_parall[i].b = a + b_h * (i + 1);
        arr_parall[i].n = n / n_threads;
        arr_parall[i].sum = 0;
        std::cout << "a = " << arr_parall[i].a << " b = " << arr_parall[i].b << " n = " << arr_parall[i].n << std::endl;
        pthread_create(&p[i], NULL, CentralRect, &arr_parall[i]);
    }

    double sum = 0;

    for (int i = 0; i < n_threads; i++) {
        pthread_join(p[i], 0);
        sum += arr_parall[i].sum;
    }

    double end = clock();

    std::cout << sum << std::endl;
    
    double seconds =  (double)(end - start) / CLOCKS_PER_SEC;

    std::cout << "Exec time: " << seconds << std::endl;

    return 0;
}

void* CentralRect(void* arr_param) {
    Param* param = (Param*)arr_param;
    double a = param->a;
    double b = param->b;
    int n = param->n;

    double h = (b - a) / n;
    double sum = 0;

    double x0 = a;
    double x1 = a + h;

    for (unsigned i = 0; i <= n - 1; i++) {
        sum += f(x0) + 4 * f(x0 + h / 2) + f(x1);

        x0 += h;
        x1 += h;
    }
    param->sum = (h / 6) * sum;
    
    return 0;
}

double f(double x) {
    return 2 * x;
}