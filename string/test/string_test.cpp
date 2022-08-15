#include <cstdio>
#include "string/string.h"

int main() {
    char buffer[] = {'R','I','T',' ','L','a','u','n','c','h',' ','I','n','i','t','i','a','t','i','v','e', '\0'};

    printf("Full String Test: \n");
    auto full_string = String(buffer, sizeof(buffer));
    printf("\tExpected: RIT Launch Initiative\n");
    printf("\tActual: %s\n", full_string.string);

    printf("Extra Space String Test: \n");
    auto much_storage_string = String(buffer, 999);
    printf("\tExpected: RIT Launch Initiative\n");
    printf("\tActual: %s\n", much_storage_string.string);


    printf("Truncated String Test: \n");
    auto trunc_string = String(buffer, 4);
    printf("\tExpected: RIT\n");
    printf("\tActual: %s\n", trunc_string.string);



    return 0;
}