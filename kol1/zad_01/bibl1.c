#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bibl1.h"

/*napisz biblioteke ladowana dynamicznie przez program zawierajaca funkcje:

1) zliczajaca sume n elementow tablicy tab:
int sumuj(int *tab, int n);

2) wyznaczajaca mediane n elementow tablicy tab
double mediana(int *tab, int n);

*/

int comparator(const void *arg1, const void *arg2 )
{
    int first = *(int *)(arg1);
    int second = *(int *)(arg2);

    if(first > second)
        return -1;

    if(first < second)
        return 1;

    return 0;
}

int sumuj(int *tab, int n){
    int sum = 0;

    for(int i = 0; i < n; i++){
        sum += tab[i];
    }

    return sum;
}

double mediana(int *tab, int n){
    int *tabCopy = malloc(sizeof(int) * n);
    if (!tabCopy) {
        perror("Memory allocation failed");
        return 0.0;
    }

    memcpy(tabCopy, tab, n * sizeof(int));
    qsort(tabCopy, n, sizeof(int), comparator);

    if(n % 2 == 0){
        int mid = n / 2;
        double median = (double)(tabCopy[mid] + tabCopy[mid-1]) / (double)(2.0);

        free(tabCopy);
        return median;
    }
    else{
        double median = (double)(tabCopy[(n - 1) / 2]);

        free(tabCopy);
        return median;
    }
}