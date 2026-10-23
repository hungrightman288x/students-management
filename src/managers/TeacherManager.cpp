#include "managers/TeacherManager.h"

TeacherManager::TeacherManager(const std::string& fileName)
    : PersistentManager<Teacher>(fileName) {}
