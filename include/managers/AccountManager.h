#pragma once
#include <iostream>
#include <string>
#include "models/Account.h"
#include "managers/Manager.h"
using namespace std;
class AccountManager : public Manager<Account>{
    private:
        string FileName;
        void LoadFromFile();
    public:
        AccountManager(const string&);
        ~AccountManager();
        Account* Login(const string&, const string&) const;
};