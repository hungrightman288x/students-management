#include "ui/Menu.h"
#include <iostream>
#include <stdexcept>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    try {
        std::string directory = "data";
        bool checkOnly = false;
        for (int i = 1; i < argc; ++i) {
            std::string argument = argv[i];
            if (argument == "--help") {
                std::cout << "student_management [--data-dir PATH] [--check-data]\n";
                return 0;
            }
            if (argument == "--data-dir" && i + 1 < argc) directory = argv[++i];
            else if (argument == "--check-data") checkOnly = true;
            else throw std::invalid_argument("Tham so khong hop le: " + argument);
        }
        SchoolManager system(directory);
        if (checkOnly) { std::cout << "Du lieu hop le.\n"; return 0; }
        Menu menu(system);
        menu.Run();
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Loi: " << error.what() << '\n';
        return 1;
    }
}
