#include "managers/CourseSectionManager.h"

CourseSectionManager::CourseSectionManager(const std::string& fileName)
    : PersistentManager<CourseSection>(fileName) {}
