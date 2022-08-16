#include <cstdio>
#include <string.h>
#include "string/string.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wwritable-strings"
void new_test(char *test_name, char *expected) {
    printf("%s Test: \n", test_name);
    printf("\tExpected: %s\n", expected);
}

int main() {
    char buffer[] = {'R', 'I', 'T', ' ', 'L', 'a', 'u', 'n', 'c', 'h', ' ', 'I', 'n', 'i', 't', 'i', 'a', 't', 'i', 'v',
                     'e', '\0'};

    // Init tests
    auto full_string = String<22>(buffer);
    new_test("Full String", "RIT Launch Initiative");
    printf("\tActual: %s\n", full_string.string);

    auto much_storage_string = String<999>(buffer);
    new_test("Extra Space String", "RIT Launch Initiative");
    printf("\tActual: %s\n", much_storage_string.string);

    auto trunc_string = String<4>(buffer);
    new_test("Truncated String", "RIT Launch Initiative");
    printf("\tActual: %s\n", trunc_string.string);


    // Accessor Tests
    new_test("Get Front", "R");
    printf("\tActual: %c\n", full_string.front());

    new_test("Get Back", "e");
    printf("\tActual: %c\n", full_string.back());

    new_test("At Index (In Bounds)", "T");
    printf("\tActual: %c\n", full_string.at(3));

    new_test("At Index (Out of Bounds)", "(Invalid Character)");
    printf("\tActual: %c\n", full_string.at(21));

    // Modifier Tests
//    new_test("Remove (In Bounds)", "RIT Lunch Initiative");
//    much_storage_string.remove(5);
//    printf("\tActual: %s\n", much_storage_string.string);
//
//    new_test("Remove (Out of Bounds)", "RIT Launch Initiative");
//    printf("\tActual: %s\n", much_storage_string.string);

    new_test("Insert (In Bounds)", "RIT Launch Initiative");
    much_storage_string.insert(10, '-');
    printf("\tActual: %s\n", much_storage_string.string);

    new_test("Insert (Out of Bounds)", "RIT Launch Initiative");
    printf("\tActual: %s\n", much_storage_string.string);

    new_test("Replace Character", "RIT Launch Initiative");
    printf("\tActual: %s\n", much_storage_string.string);

    new_test("Replace Substring", "RIT Launch Initiative");
    printf("\tActual: %s\n", much_storage_string.string);

    new_test("Clear", "(blank)");
    much_storage_string.clear();
    printf("\tActual: %s(blank)\n", much_storage_string.string);







    // Modifier Tests

    return 0;
}
#pragma clang diagnostic pop