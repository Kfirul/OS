#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int pitagoras(int a, int b, int c) {
    if (a <= 0 || b <= 0 || c <= 0) {
        return 0;
    }
    
    if (pow(a, 2) + pow(b, 2) == pow(c, 2))
        return 1;

    return 0;
}

int main() {
    int a, b, c;

    printf("Enter the lengths of 3 edges:\n");
    printf("a = ");
    scanf("%d", &a);
    printf("b = ");
    scanf("%d", &b);
    printf("c = ");
    scanf("%d", &c);


    if (pitagoras(a, b, c)) {
        double alpha = asin((double)a / c); 
        double beta = asin((double)b / c);
        double gamma = M_PI / 2.0; // 90 degrees in radians

        printf("The angles are: %.5f radians, %.5f radians, %.5f radians\n", alpha, beta, gamma);
    } else {
        printf("Error: Not a Pythagorean triple\n");
        exit(-1);
    }

    return 0;
}
