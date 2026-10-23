#pragma once
#include "managers/PersistentManager.h"
#include "models/Account.h"

class AccountManager : public PersistentManager<Account> {
public:
    explicit AccountManager(const std::string& fileName = "");
    const Account* Authenticate(const std::string& username, const std::string& password) const;
};
