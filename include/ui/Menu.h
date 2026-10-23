#pragma once

#include "managers/SchoolManager.h"

class Menu {
    SchoolManager& System;
    void DisplayAdminMenu();
    void DisplayTeacherMenu();
    void DisplayStudentMenu();
    void ManageStudents();
    void ManageTeachers();
    void ManageSubjects();
    void ManageSections();
    void ManageEnrollments();
    void ManageAccounts();
    void ChangePassword();
    void SaveChanges();
    void Logout();

public:
    explicit Menu(SchoolManager& system);
    void DisplayMenu() const;
    void Run();
};
