#include "managers/SubjectManager.h"

SubjectManager::SubjectManager(const std::string& fileName)
    : PersistentManager<Subject>(fileName) {}
