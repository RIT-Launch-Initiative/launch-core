#include <iostream>
#include "string/string.h"


int main() {
    char* buffer = "RIT Launch Initiative";

    std::cout << "Full String Test: " << std::endl;
    String full_string = String(buffer, 22);
    std::cout << "\tExpected: RIT Launch Initiative" << std::endl;
    std::cout << "\tActual: " << full_string.string << std::endl;

    std::cout << "Extra Space String Test: " << std::endl;
    String much_storage_string = String(buffer, 999);
    std::cout << "\tExpected: RIT Launch Initiative" << std::endl;
    std::cout << "\tActual: " << much_storage_string.string << std::endl;

    std::cout << "Truncated String Test: " << std::endl;
    String trunc_string = String(buffer, 4);
    std::cout << "\tExpected: RIT" << std::endl;
    std::cout << "\tActual: " << trunc_string.string << std::endl;

    return 0;
}