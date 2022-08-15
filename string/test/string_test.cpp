#include <iostream>
#include "string/string.h"

#include <memory>

int main() {
    char buffer[] = {'R','I','T',' ','L','a','u','n','c','h',' ','I','n','i','t','i','a','t','i','v','e', '\0'};

    std::cout << "Full String Test: " << std::endl;
    auto full_string = String(buffer, sizeof(buffer));
    std::cout << "\tExpected: RIT Launch Initiative" << std::endl;
    std::cout << "\tActual: " << full_string.string << std::endl;
    std::cout << buffer << std::endl;

    std::cout << "Extra Space String Test: " << std::endl;
    auto much_storage_string = String(buffer, 999);
    std::cout << "\tExpected: RIT Launch Initiative" << std::endl;
    std::cout << "\tActual: " << much_storage_string.string << std::endl;

    std::cout << "Truncated String Test: " << std::endl;
    auto trunc_string = String(buffer, 4);
    std::cout << "\tExpected: RIT" << std::endl;
    std::cout << "\tActual: " << trunc_string.string << std::endl;


    return 0;
}