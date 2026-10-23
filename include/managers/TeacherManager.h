#pragma once
#include "managers/PersistentManager.h"
#include "models/Teacher.h"

class TeacherManager : public PersistentManager<Teacher> {
public:
    explicit TeacherManager(const std::string& fileName = "");
};
