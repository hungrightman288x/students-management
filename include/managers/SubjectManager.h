#pragma once
#include "managers/PersistentManager.h"
#include "models/Subject.h"

class SubjectManager : public PersistentManager<Subject> {
public:
    explicit SubjectManager(const std::string& fileName = "");
};
