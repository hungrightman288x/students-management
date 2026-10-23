#include "managers/StudentManager.h"

StudentManager::StudentManager(const std::string& fileName)
    : PersistentManager<Student>(fileName) {}
