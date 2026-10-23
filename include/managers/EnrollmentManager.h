#pragma once
#include "managers/PersistentManager.h"
#include "models/Enrollment.h"

class EnrollmentManager : public PersistentManager<Enrollment> {
public:
    explicit EnrollmentManager(const std::string& fileName = "");
    const Enrollment* FindRegistration(const std::string& studentID, const std::string& sectionID) const;
    std::vector<const Enrollment*> GetByStudent(const std::string& studentID) const;
    std::vector<const Enrollment*> GetBySection(const std::string& sectionID) const;
};
