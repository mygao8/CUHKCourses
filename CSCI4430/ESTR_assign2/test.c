#include <stdio.h>
#include <stdlib.h>

int main(){
    unsigned int i = 4294967290, j = 4294967292;
    int res = i;
    printf("%u, res:%d\n", i-j, res);
}