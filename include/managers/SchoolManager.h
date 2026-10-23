#pragma once

#include "managers/AccountManager.h"
#include "managers/CourseSectionManager.h"
#include "managers/EnrollmentManager.h"
#include "managers/StudentManager.h"
#include "managers/SubjectManager.h"
#include "managers/TeacherManager.h"

struct AcademicResult {
    std::string EnrollmentID, StudentID, StudentName, SectionID;
    std::string SubjectID, SubjectName, TeacherName;
    int Credits;
    Date RegistrationDate;
    Grade Score;
};

struct TeachingSection {
    std::string ID, SubjectID, SubjectName;
    int Credits, StudentNumber, MaxStudents;
};

// Coordinates cross-manager rules and permissions. UI code mutates data here.
class SchoolManager {
    std::string DataDirectory;
    AccountManager Accounts;
    StudentManager Students;
    TeacherManager Teachers;
    SubjectManager Subjects;
    CourseSectionManager Sections;
    EnrollmentManager Enrollments;
    std::string CurrentUser;
    bool Dirty = false;

    void RequireRole(Role role) const;
    void ValidateAccount(const Account& account) const;
    void ValidateSection(const CourseSection& section) const;
    void ValidateAll() const;

public:
    explicit SchoolManager(const std::string& dataDirectory = "data");
    bool NeedsInitialAdmin() const;
    void CreateInitialAdmin(const std::string& username, const std::string& password);
    bool Login(const std::string& username, const std::string& password);
    void Logout();
    const Account* GetCurrentAccount() const;
    bool HasUnsavedChanges() const;
    void Save();

    const AccountManager& GetAccounts() const;
    const StudentManager& GetStudents() const;
    const TeacherManager& GetTeachers() const;
    const SubjectManager& GetSubjects() const;
    const CourseSectionManager& GetSections() const;
    const EnrollmentManager& GetEnrollments() const;

    void AddStudent(const Student& item);
    void UpdateStudent(const Student& item);
    void RemoveStudent(const std::string& id);
    void AddTeacher(const Teacher& item);
    void UpdateTeacher(const Teacher& item);
    void RemoveTeacher(const std::string& id);
    void AddSubject(const Subject& item);
    void UpdateSubject(const Subject& item);
    void RemoveSubject(const std::string& id);
    void AddSection(const std::string& id, const std::string& subjectID,
                    const std::string& teacherID, int maximum);
    void UpdateSection(const std::string& id, const std::string& subjectID,
                       const std::string& teacherID, int maximum);
    void RemoveSection(const std::string& id);

    void AddAccount(const Account& item);
    void UpdateAccountProfile(const std::string& username, Role role, const std::string& profileID);
    void RemoveAccount(const std::string& username);
    void ResetPassword(const std::string& username, const std::string& password);
    void ChangeOwnPassword(const std::string& oldPassword, const std::string& newPassword);

    void Register(const std::string& id, const std::string& studentID,
                  const std::string& sectionID, const Date& date);
    void CancelRegistration(const std::string& id);
    void UpdateRegistrationDate(const std::string& id, const Date& date);
    void SetGrade(const std::string& enrollmentID, const Grade& grade);
    std::vector<const Enrollment*> GetVisibleEnrollments() const;
    std::vector<const CourseSection*> GetTeachingSections() const;
    std::vector<AcademicResult> GetAcademicResults() const;
    std::vector<TeachingSection> GetTeachingSectionDetails() const;
    const Student& GetOwnStudentProfile() const;
    const Teacher& GetOwnTeacherProfile() const;
};
