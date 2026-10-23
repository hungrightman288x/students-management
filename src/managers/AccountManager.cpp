#include "managers/AccountManager.h"

AccountManager::AccountManager(const std::string& fileName)
    : PersistentManager<Account>(fileName) {}

const Account* AccountManager::Authenticate(const std::string& username, const std::string& password) const {
    auto account = FindByID(username);
    return account && account->CheckPassword(password) ? account : nullptr;
}
