#include <stdio.h>
#include <math.h>
#include <stdlib.h>

// compare function for sorting
int compare(const void *a, const void *b) {
    long x = *(long*)a;
    long y = *(long*)b;

    if (x > y) return 1;
    if (x < y) return -1;
    return 0;
}

long pthFactor(long n, long p) {
    long small[100000];
    long big[100000];

    int smallCount = 0;
    int bigCount = 0;

    // find square root
    long sqrtN = (long)sqrt((double)n);

    // loop from 1 to sqrt(n)
    for (long i = 1; i <= sqrtN; i++) {
        if (n % i == 0) {
            small[smallCount] = i;
            smallCount++;

            if (i != n / i) {
                big[bigCount] = n / i;
                bigCount++;
            }
        }
    }

    // sort big factors
    qsort(big, bigCount, sizeof(long), compare);

    int total = smallCount + bigCount;

    if (p > total) {
        return 0;
    }

    if (p <= smallCount) {
        return small[p - 1];
    } else {
        return big[p - smallCount - 1];
    }
}

int main() {
    printf("%ld\n", pthFactor(10, 3));   // 5
    printf("%ld\n", pthFactor(10, 5));   // 0
    printf("%ld\n", pthFactor(100, 7));  // 25
    printf("%ld\n", pthFactor(1, 1));    // 1
    return 0;
}