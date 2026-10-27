#include <iostream>
#include <exception>
#include "managers/AccountManager.h"
using namespace std;
int main() {
    try {
        AccountManager accounts("data/accounts.txt");
        accounts.PrintAll();
    } catch (const exception& error) {
        cerr << "Loi: " << error.what() << '\n';
        return 1;
    }
}
