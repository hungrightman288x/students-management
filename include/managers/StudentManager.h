#pragma once
#include "managers/PersistentManager.h"
#include "models/Student.h"

class StudentManager : public PersistentManager<Student> {
public:
    explicit StudentManager(const std::string& fileName = "");
};
