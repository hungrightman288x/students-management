#include "managers/SchoolManager.h"
#include <array>
#include <filesystem>
#include <fstream>
#include <set>
#include <stdexcept>
#include <utility>

namespace {
namespace fs = std::filesystem;
const std::array<std::string, 6> Files = {
    "accounts.txt", "students.txt", "teachers.txt", "subjects.txt", "course_sections.txt", "enrollments.txt"
};

// A prepared journal contains every old file. Recover an interrupted multi-file
// save before any manager reads data, so related records cannot be half updated.
void Recover(const fs::path& directory) {
    const auto journal = directory / ".student-management-save";
    if (!fs::exists(journal)) return;
    if (fs::exists(journal / "prepared") && !fs::exists(journal / "committed")) {
        for (const auto& file : Files) {
            const auto backup = journal / "old" / file;
            const auto target = directory / file;
            if (fs::exists(backup)) {
                auto temporary = target;
                temporary += ".recover";
                fs::copy_file(backup, temporary, fs::copy_options::overwrite_existing);
                AtomicReplaceFile(temporary, target);
            } else if (fs::exists(target)) {
                fs::remove(target);
            }
        }
    }
    fs::remove_all(journal);
}

std::string PrepareDirectory(const std::string& directory) {
    if (directory.empty()) throw std::invalid_argument("Thu muc du lieu khong duoc rong");
    Recover(directory);
    return directory;
}

void Marker(const fs::path& path) {
    std::ofstream output(path);
    output << "complete\n";
    output.close();
    if (!output) throw std::runtime_error("Khong ghi duoc trang thai luu: " + path.string());
}

template<class T> void ValidatePerson(const T& item) {
    ValidateText(item.GetID(), "Ma");
    ValidateText(item.GetFullName(), "Ho ten");
    ValidateText(item.GetEmail(), "Email", false);
}

void ValidateSubject(const Subject& item) {
    ValidateText(item.GetID(), "Ma mon");
    ValidateText(item.GetSubjectName(), "Ten mon");
    if (item.GetCredits() <= 0) throw std::invalid_argument("So tin chi phai lon hon 0");
}

template<class T> void AddItem(Manager<T>& manager, const T& item) {
    if (!manager.Add(std::make_unique<T>(item))) throw std::invalid_argument("Ma da ton tai: " + item.GetID());
}

template<class T> void UpdateItem(Manager<T>& manager, const T& item) {
    if (!manager.Update(item)) throw std::invalid_argument("Khong tim thay ma: " + item.GetID());
}

template<class T> T& Existing(Manager<T>& manager, const std::string& id) {
    auto item = manager.FindByID(id);
    if (!item) throw std::invalid_argument("Khong tim thay ma: " + id);
    return *item;
}
}

SchoolManager::SchoolManager(const std::string& directory)
    : DataDirectory(PrepareDirectory(directory)),
      Accounts((fs::path(DataDirectory) / Files[0]).string()),
      Students((fs::path(DataDirectory) / Files[1]).string()),
      Teachers((fs::path(DataDirectory) / Files[2]).string()),
      Subjects((fs::path(DataDirectory) / Files[3]).string()),
      Sections((fs::path(DataDirectory) / Files[4]).string()),
      Enrollments((fs::path(DataDirectory) / Files[5]).string()) {
    ValidateAll();
}

void SchoolManager::RequireRole(Role role) const {
    auto account = GetCurrentAccount();
    if (!account || account->GetUserRole() != role) throw std::runtime_error("Khong co quyen thuc hien thao tac nay");
}

bool SchoolManager::NeedsInitialAdmin() const { return Accounts.Empty(); }
void SchoolManager::CreateInitialAdmin(const std::string& username, const std::string& password) {
    if (!NeedsInitialAdmin()) throw std::runtime_error("He thong da co tai khoan");
    Account account(username, password, ADMIN);
    ValidateAccount(account);
    AddItem(Accounts, account);
    Dirty = true;
}

bool SchoolManager::Login(const std::string& username, const std::string& password) {
    Logout();
    auto account = Accounts.Authenticate(username, password);
    if (!account) return false;
    CurrentUser = account->GetID();
    return true;
}
void SchoolManager::Logout() { CurrentUser.clear(); }
const Account* SchoolManager::GetCurrentAccount() const { return Accounts.FindByID(CurrentUser); }
bool SchoolManager::HasUnsavedChanges() const { return Dirty; }

void SchoolManager::ValidateAccount(const Account& item) const {
    ValidateText(item.GetID(), "Ten dang nhap");
    ValidateText(item.GetProfileID(), "Ma ho so", false);
    if (!item.HasPassword()) throw std::invalid_argument("Mat khau khong duoc rong");
    if (item.GetUserRole() == STUDENT && !Students.FindByID(item.GetProfileID()))
        throw std::invalid_argument("Tai khoan phai lien ket voi sinh vien ton tai");
    if (item.GetUserRole() == TEACHER && !Teachers.FindByID(item.GetProfileID()))
        throw std::invalid_argument("Tai khoan phai lien ket voi giao vien ton tai");
    if (item.GetUserRole() == ADMIN && !item.GetProfileID().empty())
        throw std::invalid_argument("Tai khoan admin khong gan ma ho so");
}

void SchoolManager::ValidateSection(const CourseSection& item) const {
    ValidateText(item.GetID(), "Ma lop");
    if (!Subjects.FindByID(item.GetSubjectId())) throw std::invalid_argument("Mon hoc khong ton tai");
    if (!Teachers.FindByID(item.GetTeacherId())) throw std::invalid_argument("Giao vien khong ton tai");
    if (item.GetMaxStudents() <= 0) throw std::invalid_argument("Si so toi da phai lon hon 0");
}

void SchoolManager::ValidateAll() const {
    for (auto item : Students.GetAll()) ValidatePerson(*item);
    for (auto item : Teachers.GetAll()) ValidatePerson(*item);
    for (auto item : Subjects.GetAll()) ValidateSubject(*item);
    bool hasAdmin = false;
    for (auto item : Accounts.GetAll()) {
        ValidateAccount(*item);
        hasAdmin = hasAdmin || item->GetUserRole() == ADMIN;
    }
    if (!Accounts.Empty() && !hasAdmin) throw std::runtime_error("Du lieu phai co it nhat mot admin");
    std::set<std::pair<std::string, std::string>> registrations;
    std::unordered_map<std::string, int> counts;
    for (auto item : Enrollments.GetAll()) {
        ValidateText(item->GetID(), "Ma dang ky");
        if (!Students.FindByID(item->GetStudentId()) || !Sections.FindByID(item->GetCourseSectionId()))
            throw std::runtime_error("Dang ky " + item->GetID() + " tham chieu sinh vien/lop khong ton tai");
        if (!registrations.emplace(item->GetStudentId(), item->GetCourseSectionId()).second)
            throw std::runtime_error("Trung sinh vien trong lop: " + item->GetCourseSectionId());
        ++counts[item->GetCourseSectionId()];
    }
    for (auto item : Sections.GetAll()) {
        ValidateSection(*item);
        if (counts[item->GetID()] != item->GetStudentNumber())
            throw std::runtime_error("Si so khong khop danh sach dang ky cua lop: " + item->GetID());
    }
}

void SchoolManager::Save() {
    ValidateAll();
    fs::create_directories(DataDirectory);
    Recover(DataDirectory);
    const auto journal = fs::path(DataDirectory) / ".student-management-save";
    fs::create_directories(journal / "new");
    fs::create_directories(journal / "old");
    try {
        Accounts.SaveToFile((journal / "new" / Files[0]).string());
        Students.SaveToFile((journal / "new" / Files[1]).string());
        Teachers.SaveToFile((journal / "new" / Files[2]).string());
        Subjects.SaveToFile((journal / "new" / Files[3]).string());
        Sections.SaveToFile((journal / "new" / Files[4]).string());
        Enrollments.SaveToFile((journal / "new" / Files[5]).string());
        for (const auto& file : Files) {
            const auto target = fs::path(DataDirectory) / file;
            if (fs::exists(target)) {
                if (!fs::is_regular_file(target)) throw std::runtime_error("Duong dan khong phai file: " + target.string());
                fs::copy_file(target, journal / "old" / file);
            }
        }
        Marker(journal / "prepared");
        for (const auto& file : Files) AtomicReplaceFile(journal / "new" / file, fs::path(DataDirectory) / file);
        Marker(journal / "committed");
        Dirty = false;
        std::error_code ignored;
        fs::remove_all(journal, ignored);
    } catch (...) {
        Recover(DataDirectory);
        throw;
    }
}

const AccountManager& SchoolManager::GetAccounts() const { RequireRole(ADMIN); return Accounts; }
const StudentManager& SchoolManager::GetStudents() const { RequireRole(ADMIN); return Students; }
const TeacherManager& SchoolManager::GetTeachers() const { RequireRole(ADMIN); return Teachers; }
const SubjectManager& SchoolManager::GetSubjects() const { RequireRole(ADMIN); return Subjects; }
const CourseSectionManager& SchoolManager::GetSections() const { RequireRole(ADMIN); return Sections; }
const EnrollmentManager& SchoolManager::GetEnrollments() const { RequireRole(ADMIN); return Enrollments; }

void SchoolManager::AddStudent(const Student& item) {
    RequireRole(ADMIN); ValidatePerson(item); AddItem(Students, item); Dirty = true;
}
void SchoolManager::UpdateStudent(const Student& item) {
    RequireRole(ADMIN); ValidatePerson(item); UpdateItem(Students, item); Dirty = true;
}
void SchoolManager::RemoveStudent(const std::string& id) {
    RequireRole(ADMIN);
    auto& item = Existing(Students, id);
    if (!Enrollments.GetByStudent(id).empty()) throw std::runtime_error("Sinh vien dang co dang ky hoc");
    for (auto account : Accounts.GetAll())
        if (account->GetUserRole() == STUDENT && account->GetProfileID() == id)
            throw std::runtime_error("Hay xoa tai khoan lien ket truoc");
    Students.Remove(&item); Dirty = true;
}
void SchoolManager::AddTeacher(const Teacher& item) {
    RequireRole(ADMIN); ValidatePerson(item); AddItem(Teachers, item); Dirty = true;
}
void SchoolManager::UpdateTeacher(const Teacher& item) {
    RequireRole(ADMIN); ValidatePerson(item); UpdateItem(Teachers, item); Dirty = true;
}
void SchoolManager::RemoveTeacher(const std::string& id) {
    RequireRole(ADMIN);
    auto& item = Existing(Teachers, id);
    for (auto section : Sections.GetAll())
        if (section->GetTeacherId() == id) throw std::runtime_error("Giao vien dang phu trach lop");
    for (auto account : Accounts.GetAll())
        if (account->GetUserRole() == TEACHER && account->GetProfileID() == id)
            throw std::runtime_error("Hay xoa tai khoan lien ket truoc");
    Teachers.Remove(&item); Dirty = true;
}
void SchoolManager::AddSubject(const Subject& item) {
    RequireRole(ADMIN); ValidateSubject(item); AddItem(Subjects, item); Dirty = true;
}
void SchoolManager::UpdateSubject(const Subject& item) {
    RequireRole(ADMIN); ValidateSubject(item); UpdateItem(Subjects, item); Dirty = true;
}
void SchoolManager::RemoveSubject(const std::string& id) {
    RequireRole(ADMIN);
    auto& item = Existing(Subjects, id);
    for (auto section : Sections.GetAll())
        if (section->GetSubjectId() == id) throw std::runtime_error("Mon hoc dang co lop hoc phan");
    Subjects.Remove(&item); Dirty = true;
}

void SchoolManager::AddSection(const std::string& id, const std::string& subjectID,
                               const std::string& teacherID, int maximum) {
    RequireRole(ADMIN);
    CourseSection item(id, subjectID, teacherID, maximum);
    ValidateSection(item); AddItem(Sections, item); Dirty = true;
}
void SchoolManager::UpdateSection(const std::string& id, const std::string& subjectID,
                                  const std::string& teacherID, int maximum) {
    RequireRole(ADMIN);
    const auto& old = Existing(Sections, id);
    if (old.GetSubjectId() != subjectID && old.GetStudentNumber() > 0)
        throw std::runtime_error("Khong doi mon khi lop da co sinh vien");
    CourseSection item(id, subjectID, teacherID, maximum, old.GetStudentNumber());
    ValidateSection(item); UpdateItem(Sections, item); Dirty = true;
}
void SchoolManager::RemoveSection(const std::string& id) {
    RequireRole(ADMIN);
    auto& item = Existing(Sections, id);
    if (!Enrollments.GetBySection(id).empty()) throw std::runtime_error("Lop dang co sinh vien dang ky");
    Sections.Remove(&item); Dirty = true;
}

void SchoolManager::AddAccount(const Account& item) {
    RequireRole(ADMIN); ValidateAccount(item); AddItem(Accounts, item); Dirty = true;
}
void SchoolManager::UpdateAccountProfile(const std::string& username, Role role, const std::string& profileID) {
    RequireRole(ADMIN);
    Account replacement = Existing(Accounts, username);
    if (username == CurrentUser && (role != ADMIN || !profileID.empty()))
        throw std::runtime_error("Khong doi vai tro cua tai khoan dang dang nhap");
    replacement.SetProfile(role, profileID);
    ValidateAccount(replacement); UpdateItem(Accounts, replacement); Dirty = true;
}
void SchoolManager::RemoveAccount(const std::string& username) {
    RequireRole(ADMIN);
    if (username == CurrentUser) throw std::runtime_error("Khong xoa tai khoan dang dang nhap");
    auto& item = Existing(Accounts, username);
    Accounts.Remove(&item); Dirty = true;
}
void SchoolManager::ResetPassword(const std::string& username, const std::string& password) {
    RequireRole(ADMIN);
    Existing(Accounts, username).ChangePassword(password); Dirty = true;
}
void SchoolManager::ChangeOwnPassword(const std::string& oldPassword, const std::string& newPassword) {
    auto account = Accounts.FindByID(CurrentUser);
    if (!account) throw std::runtime_error("Chua dang nhap");
    if (!account->CheckPassword(oldPassword)) throw std::invalid_argument("Mat khau cu khong dung");
    account->ChangePassword(newPassword); Dirty = true;
}

void SchoolManager::Register(const std::string& id, const std::string& studentID,
                            const std::string& sectionID, const Date& date) {
    RequireRole(ADMIN);
    ValidateText(id, "Ma dang ky");
    Existing(Students, studentID);
    auto& section = Existing(Sections, sectionID);
    if (Enrollments.FindRegistration(studentID, sectionID)) throw std::runtime_error("Sinh vien da dang ky lop nay");
    if (section.GetStudentNumber() >= section.GetMaxStudents()) throw std::runtime_error("Lop da day");
    Enrollment item(id, studentID, sectionID, date.getDay(), date.getMonth(), date.getYear());
    AddItem(Enrollments, item);
    section.IncreaseStudentNumber(); Dirty = true;
}
void SchoolManager::CancelRegistration(const std::string& id) {
    RequireRole(ADMIN);
    auto& item = Existing(Enrollments, id);
    auto& section = Existing(Sections, item.GetCourseSectionId());
    section.DecreaseStudentNumber();
    Enrollments.Remove(&item); Dirty = true;
}
void SchoolManager::UpdateRegistrationDate(const std::string& id, const Date& date) {
    RequireRole(ADMIN);
    const auto& old = Existing(Enrollments, id);
    const auto& grade = old.GetScore();
    Enrollment item(id, old.GetStudentId(), old.GetCourseSectionId(), date.getDay(), date.getMonth(), date.getYear(),
                    grade.GetAttendance(), grade.GetMidterm(), grade.GetFinal());
    UpdateItem(Enrollments, item); Dirty = true;
}
void SchoolManager::SetGrade(const std::string& enrollmentID, const Grade& grade) {
    RequireRole(TEACHER);
    auto& item = Existing(Enrollments, enrollmentID);
    const auto sectionID = item.GetCourseSectionId();
    const auto& section = Existing(Sections, sectionID);
    if (section.GetTeacherId() != GetCurrentAccount()->GetProfileID())
        throw std::runtime_error("Ban khong phu trach lop nay");
    item.GetScore() = grade; Dirty = true;
}

std::vector<const Enrollment*> SchoolManager::GetVisibleEnrollments() const {
    const auto account = GetCurrentAccount();
    if (!account) throw std::runtime_error("Chua dang nhap");
    std::vector<const Enrollment*> result;
    for (auto item : Enrollments.GetAll()) {
        if (account->GetUserRole() == ADMIN ||
            (account->GetUserRole() == STUDENT && item->GetStudentId() == account->GetProfileID()) ||
            (account->GetUserRole() == TEACHER && Sections.FindByID(item->GetCourseSectionId())->GetTeacherId() == account->GetProfileID()))
            result.push_back(item);
    }
    return result;
}

std::vector<const CourseSection*> SchoolManager::GetTeachingSections() const {
    RequireRole(TEACHER);
    std::vector<const CourseSection*> result;
    for (auto item : Sections.GetAll())
        if (item->GetTeacherId() == GetCurrentAccount()->GetProfileID()) result.push_back(item);
    return result;
}
const Student& SchoolManager::GetOwnStudentProfile() const {
    RequireRole(STUDENT);
    return *Students.FindByID(GetCurrentAccount()->GetProfileID());
}
const Teacher& SchoolManager::GetOwnTeacherProfile() const {
    RequireRole(TEACHER);
    return *Teachers.FindByID(GetCurrentAccount()->GetProfileID());
}

std::vector<AcademicResult> SchoolManager::GetAcademicResults() const {
    std::vector<AcademicResult> result;
    for (const auto* item : GetVisibleEnrollments()) {
        const auto* section = Sections.FindByID(item->GetCourseSectionId());
        const auto* subject = Subjects.FindByID(section->GetSubjectId());
        const auto* student = Students.FindByID(item->GetStudentId());
        const auto* teacher = Teachers.FindByID(section->GetTeacherId());
        result.push_back({item->GetID(), student->GetID(), student->GetFullName(), section->GetID(),
            subject->GetID(), subject->GetSubjectName(), teacher->GetFullName(), subject->GetCredits(),
            item->GetEnrollmentDate(), item->GetScore()});
    }
    return result;
}

std::vector<TeachingSection> SchoolManager::GetTeachingSectionDetails() const {
    std::vector<TeachingSection> result;
    for (const auto* section : GetTeachingSections()) {
        const auto* subject = Subjects.FindByID(section->GetSubjectId());
        result.push_back({section->GetID(), subject->GetID(), subject->GetSubjectName(), subject->GetCredits(),
            section->GetStudentNumber(), section->GetMaxStudents()});
    }
    return result;
}
