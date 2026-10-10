#include <stdio.h>

int main() {
    int num = 6;
    int fact = 1;
    int temp = num;

    while (temp > 1) {
        fact = fact * temp;
        temp--;
    }

    printf("Factorial of %d = %d\n", num, fact);
    return 0;
}
