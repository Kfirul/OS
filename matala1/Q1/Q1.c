
#include <stdio.h>
#include <stdlib.h>

int infinityRec(int num){
    return infinityRec(num * 2);
}

int illegalAccess(){
   int* arr = malloc(1);
    arr[3423423] = 3;
    free(arr);
    return 0;
}


int main(){
    //infinityRec(40); //switch between the comments
    illegalAccess();
    return 0;
}