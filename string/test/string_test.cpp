#include <cstdio>
#include <string.h>
#include "string/string.h"
#include <utility>

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
    String full_string = alloc::String<22>();
    full_string = buffer;
    new_test("Full String", "RIT Launch Initiative");
    printf("\tActual: %s\n", full_string.string);

    String much_storage_string = alloc::String<1000>();
    much_storage_string = buffer;
    new_test("Extra Space String", "RIT Launch Initiative");
    printf("\tActual: %s\n", much_storage_string.string);

    String trunc_string = alloc::String<11>();
    trunc_string = buffer;
    new_test("Truncated String", "RIT Launch Initiative");
    printf("\tActual: %s\n", trunc_string.string);

    // Accessor Tests
    new_test("Get Front", "R");
    printf("\tActual: %c\n", full_string.front());

    new_test("Get Back", "e");
    printf("\tActual: %c\n", full_string.back());

    new_test("At Index (In Bounds)", "T");
    printf("\tActual: %c\n", full_string.at(2));

    new_test("At Index (Out of Bounds)", "(Invalid Character)");
    printf("\tActual: %c\n", full_string.at(21));

    new_test("Get Substring", "RIT");
    char result[4];
    full_string.substr(0, 3, result);
    printf("\tActual: %s\n", result);

    // Modifier Tests
    new_test("Remove (In Bounds)", "RIT Lunch Initiative");
    much_storage_string.remove(5);
    printf("\tActual: %s\n", much_storage_string.string);

    new_test("Remove (Out of Bounds)", "RIT Lunch Initiative");
    much_storage_string.remove(10000);
    printf("\tActual: %s\n", much_storage_string.string);

    new_test("Insert (In Bounds)", "RIT Launch Initiative");
    much_storage_string.insert(5, 'a');
    printf("\tActual: %s\n", much_storage_string.string);

    new_test("Insert (Out of Bounds)", "RIT Launch Initiative");
    much_storage_string.insert(555, '-');
    printf("\tActual: %s\n", much_storage_string.string);

    new_test("Replace Character", "RIT Launch");
    much_storage_string.replace(10, '\0');
    printf("\tActual: %s\n", much_storage_string.string);

    new_test("Recount", "10");
    printf("\tActual: %d\n", much_storage_string.recount());

    new_test("Replace Substring", "RPI Launch");
    char inferior_rocket_team[] = {'R', 'P', 'I'};
    much_storage_string.replace(0, 3, &inferior_rocket_team[0]);
    printf("\tActual: %s\n", much_storage_string.string);

    new_test("Replace Substring (and append)", "RPI Initiative");
    char initiative[11] = {' ', 'I', 'n', 'i', 't', 'i', 'a', 't', 'i', 'v', 'e'};
    much_storage_string.replace(3, 14, &initiative[0]);
    printf("\tActual: %s\n", much_storage_string.string);
    printf("\tExpected: 14");
    printf("\tActual: %d\n", much_storage_string.strlen);

    new_test("Clear", "(blank)");
    much_storage_string.clear();
    printf("\tActual: %s(blank)\n", much_storage_string.string);

    return 0;
}
#pragma clang diagnostic pop


