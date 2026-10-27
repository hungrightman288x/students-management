#include <iostream>
#include <exception>
#include "managers/AccountManager.h"
using namespace std;
int main() {
    try {
        AccountManager accounts("data/accounts.txt");
        Account* check = accounts.Login("SV005", "xin chao");
        if(check == nullptr){
            cout << "Ten dang nhap hoac mat khau khong dung";
        }
        else check->DisplayInfo();
    } catch (const exception& error) {
        cerr << "Loi: " << error.what() << '\n';
        return 1;
    }
}
