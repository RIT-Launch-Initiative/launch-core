#include <cstdio>
#include "string/string.h"


void print_new_test(char* test_name, char* expected) {
    printf("%s Test: \n", test_name);
    printf("\tExpected: %s\n", expected);
}

int main() {
    char buffer[] = {'R','I','T',' ','L','a','u','n','c','h',' ','I','n','i','t','i','a','t','i','v','e', '\0'};

    // Init tests
    auto full_string = String<22>(buffer);
    print_new_test("Full String", "RIT Launch Initiative");
    printf("\tActual: %s\n", full_string.string);

    auto much_storage_string = String<999>(buffer);
    print_new_test("Extra Space String", "RIT Launch Initiative");
    printf("\tActual: %s\n", much_storage_string.string);


    auto trunc_string = String<4>(buffer);
    print_new_test("Truncated String", "RIT Launch Initiative");
    printf("\tActual: %s\n", trunc_string.string);

    // Accessor Tests



    // Modifier Tests





    return 0;
}