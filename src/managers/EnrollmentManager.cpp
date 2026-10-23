#include "managers/EnrollmentManager.h"

EnrollmentManager::EnrollmentManager(const std::string& fileName)
    : PersistentManager<Enrollment>(fileName) {}

const Enrollment* EnrollmentManager::FindRegistration(const std::string& studentID, const std::string& sectionID) const {
    for (auto item : GetAll())
        if (item->GetStudentId() == studentID && item->GetCourseSectionId() == sectionID) return item;
    return nullptr;
}

std::vector<const Enrollment*> EnrollmentManager::GetByStudent(const std::string& studentID) const {
    std::vector<const Enrollment*> result;
    for (auto item : GetAll()) if (item->GetStudentId() == studentID) result.push_back(item);
    return result;
}

std::vector<const Enrollment*> EnrollmentManager::GetBySection(const std::string& sectionID) const {
    std::vector<const Enrollment*> result;
    for (auto item : GetAll()) if (item->GetCourseSectionId() == sectionID) result.push_back(item);
    return result;
}
