#pragma once
#include "managers/PersistentManager.h"
#include "models/CourseSection.h"

class CourseSectionManager : public PersistentManager<CourseSection> {
public:
    explicit CourseSectionManager(const std::string& fileName = "");
};
