#include "managers/SchoolManager.h"
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <type_traits>

namespace fs = std::filesystem;
namespace {
int Checks = 0;
void Check(bool condition, const std::string& message) {
    ++Checks;
    if (!condition) throw std::runtime_error(message);
}
template<class F> void Reject(F operation, const std::string& message) {
    bool rejected = false;
    try { operation(); } catch (const std::exception&) { rejected = true; }
    Check(rejected, message);
}

class TestDirectory {
public:
    fs::path Path;
    TestDirectory() {
        Path = fs::temp_directory_path() / ("student-management-tests-" +
            std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
        if (!fs::create_directory(Path)) throw std::runtime_error("Cannot create isolated test directory");
    }
    ~TestDirectory() { std::error_code ignored; fs::remove_all(Path, ignored); }
};

void Put(const fs::path& path, const std::string& data) {
    std::ofstream output(path);
    output << data;
    output.close();
    if (!output) throw std::runtime_error("Cannot write test fixture");
}
std::string Read(const fs::path& path) {
    std::ifstream input(path);
    return std::string(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
}

void Seed(SchoolManager& system) {
    system.CreateInitialAdmin("admin", "admin-pass");
    Check(system.Login("admin", "admin-pass"), "Admin login");
    system.AddStudent(Student("S1", "Student One", 29, 2, 2004, "one@example.test"));
    system.AddStudent(Student("S2", "Student Two", 1, 1, 2005, "two@example.test"));
    system.AddStudent(Student("S3", "Student Three", 1, 1, 2005));
    system.AddTeacher(Teacher("T1", "Teacher One", 1, 1, 1980));
    system.AddTeacher(Teacher("T2", "Teacher Two", 1, 1, 1980));
    system.AddSubject(Subject("M1", "C++", 3));
    system.AddSubject(Subject("M2", "Data structures", 4));
    system.AddSection("C1", "M1", "T1", 1);
    system.AddSection("C2", "M2", "T2", 2);
    system.AddAccount(Account("teacher1", "teacher-pass", TEACHER, "T1"));
    system.AddAccount(Account("teacher2", "teacher-pass", TEACHER, "T2"));
    system.AddAccount(Account("student1", "student-pass", STUDENT, "S1"));
    system.AddAccount(Account("student2", "student-pass", STUDENT, "S2"));
    system.Register("E1", "S1", "C1", Date(23, 10, 2026));
    system.Register("E2", "S2", "C2", Date(23, 10, 2026));
}

void ModelsAndOwnership() {
    static_assert(!std::is_copy_constructible<Manager<Subject>>::value, "Managers must not share ownership");
    Check(Date(29, 2, 2000).to_string() == "29/02/2000", "Leap year formatting");
    Reject([] { Date invalid(29, 2, 1900); }, "Century leap rule");
    Reject([] { Date invalid(31, 4, 2026); }, "Invalid day");
    Reject([] { Date invalid(1, 13, 2026); }, "Invalid month");
    Reject([] { Date invalid(1, 1, 0); }, "Invalid year");
    Date date(31, 1, 2026);
    Reject([&] { date.SetMonth(2); }, "Date setter validates before mutation");
    Check(date.to_string() == "31/01/2026", "Date unchanged after invalid setter");
    Grade grade(10, 8, 7);
    Check(std::abs(grade.GetTotal() - 7.6f) < 0.0001f, "Weighted total");
    Reject([&] { grade.SetFinal(std::numeric_limits<float>::quiet_NaN()); }, "Reject NaN");
    Reject([&] { grade.SetMidterm(std::numeric_limits<float>::infinity()); }, "Reject infinity");
    Reject([&] { grade.SetAttendance(-1); }, "Reject negative grade");
    Reject([&] { grade.SetFinal(11); }, "Reject grade over ten");
    Check(grade.GetFinal() == 7, "Invalid grade preserves old value");
    Reject([] { Account invalid("user", "pass", static_cast<Role>(99)); }, "Reject invalid role");
    Account account("user", "pass");
    Reject([&] { account.ChangePassword("   "); }, "Reject whitespace password");
    Reject([&] { account.ChangePassword("a\nb"); }, "Reject multiline password");
    Check(account.CheckPassword("pass"), "Invalid password leaves old one");
    Manager<Subject> manager;
    auto item = std::make_unique<Subject>("M", "Subject", 3);
    auto pointer = item.get();
    Check(manager.Add(std::move(item)), "Add owned item");
    Check(!manager.Add(nullptr), "Null add");
    Subject duplicate("M", "Duplicate", 4);
    Check(!manager.Add(&duplicate), "Duplicate raw add retains caller ownership");
    Check(!manager.Remove(&duplicate), "Remove checks object identity");
    Check(manager.Update(Subject("M", "Updated", 4)), "Update object");
    Check(manager.FindByID("M") == pointer && pointer->GetCredits() == 4, "Pointer stable on update");
    Check(manager.Remove(pointer) && manager.Empty(), "Remove owned object");
}

void BusinessRules() {
    TestDirectory directory;
    SchoolManager system(directory.Path.string());
    Check(system.NeedsInitialAdmin(), "Empty data supports bootstrap");
    Reject([&] { system.AddStudent(Student("X", "Test")); }, "No anonymous writes");
    Seed(system);
    Reject([&] { system.CreateInitialAdmin("other", "pass"); }, "No second bootstrap");
    Reject([&] { system.AddStudent(Student("S1", "Duplicate")); }, "Duplicate IDs");
    Reject([&] { system.AddStudent(Student(" ", "Blank ID")); }, "Whitespace ID");
    Reject([&] { system.AddStudent(Student("X", "")); }, "Empty name");
    Reject([&] { system.AddSubject(Subject("M3", "Zero", 0)); }, "Positive credits");
    Reject([&] { system.AddSection("C3", "missing", "T1", 10); }, "Subject must exist");
    Reject([&] { system.AddSection("C3", "M1", "missing", 10); }, "Teacher must exist");
    Reject([&] { system.AddSection("C3", "M1", "T1", 0); }, "Positive capacity");
    Reject([&] { system.Register("E3", "S1", "C1", Date()); }, "Duplicate student/section");
    Reject([&] { system.Register("E3", "S3", "C1", Date()); }, "Class full");
    Reject([&] { system.Register("E3", "missing", "C2", Date()); }, "Student must exist");
    Reject([&] { system.Register("E3", "S3", "missing", Date()); }, "Section must exist");
    Reject([&] { system.Register("E1", "S3", "C2", Date()); }, "Registration ID unique");
    Check(system.GetSections().FindByID("C2")->GetStudentNumber() == 1, "Rejected registration preserves count");
    Reject([&] { system.RemoveStudent("S1"); }, "Cannot delete enrolled student");
    Reject([&] { system.RemoveSection("C1"); }, "Cannot delete occupied section");
    Reject([&] { system.RemoveSubject("M1"); }, "Cannot delete referenced subject");
    Reject([&] { system.RemoveTeacher("T1"); }, "Cannot delete assigned teacher");
    Reject([&] { system.UpdateSection("C2", "M2", "T2", 0); }, "Cannot shrink below enrollment count");
    Reject([&] { system.UpdateSection("C2", "M1", "T2", 2); }, "Cannot switch occupied section subject");
    Reject([&] { system.AddAccount(Account("bad", "pass", STUDENT, "missing")); }, "Account profile must exist");
    Reject([&] { system.AddAccount(Account("bad", "pass", ADMIN, "S1")); }, "Admin cannot impersonate profile");
    Reject([&] { system.AddAccount(Account("bad", "", STUDENT, "S1")); }, "Account needs password");
    Reject([&] { system.RemoveAccount("admin"); }, "Cannot delete active admin");
    Reject([&] { system.UpdateAccountProfile("admin", STUDENT, "S1"); }, "Cannot demote active admin");
    Reject([&] { system.SetGrade("E1", Grade(10, 10, 10)); }, "Only assigned teacher sets grades");
    system.UpdateStudent(Student("S3", "Updated Student", 1, 2, 2005, "new@example.test"));
    Check(system.GetStudents().FindByID("S3")->GetFullName() == "Updated Student", "Student CRUD update");
    system.UpdateTeacher(Teacher("T1", "Updated Teacher", 1, 1, 1980));
    system.UpdateSubject(Subject("M1", "Updated Subject", 5));
    Check(system.GetSubjects().FindByID("M1")->GetCredits() == 5, "Subject credits update");
    system.UpdateSection("C1", "M1", "T1", 3);
    Check(system.GetSections().FindByID("C1")->GetStudentNumber() == 1, "Section update preserves count");
    system.AddAccount(Account("extra", "pass", STUDENT, "S3"));
    Reject([&] { system.RemoveStudent("S3"); }, "Cannot delete account-linked profile");
    system.UpdateAccountProfile("extra", TEACHER, "T2");
    Check(system.GetAccounts().FindByID("extra")->GetProfileID() == "T2", "Account profile update");
    system.RemoveAccount("extra");
    system.RemoveStudent("S3");
    Check(!system.GetStudents().FindByID("S3"), "Delete unreferenced student");
    system.CancelRegistration("E1");
    Check(system.GetSections().FindByID("C1")->GetStudentNumber() == 0, "Cancel reduces count");
    system.RemoveSection("C1");
    system.RemoveSubject("M1");
    system.RemoveAccount("teacher1");
    system.RemoveTeacher("T1");
    Check(!system.GetTeachers().FindByID("T1"), "Delete after unlinking dependencies");
    system.Save();
    Check(!system.HasUnsavedChanges(), "Save clears dirty state");
}

void PermissionsAndRoundTrip() {
    TestDirectory directory;
    {
        SchoolManager system(directory.Path.string());
        Seed(system);
        system.UpdateStudent(Student("S1", "Nguyen \"A\" \\ B", 29, 2, 2004, "a@example.test"));
        system.AddAccount(Account("second-admin", "pass", ADMIN));
        system.ResetPassword("student2", "new-pass");
        Check(!system.Login("student2", "student-pass") && !system.GetCurrentAccount(), "Failed login clears session");
        Check(system.Login("student2", "new-pass"), "Admin password reset");
        Check(system.GetVisibleEnrollments().size() == 1 && system.GetVisibleEnrollments()[0]->GetID() == "E2", "Student only sees own registration");
        Check(system.GetOwnStudentProfile().GetID() == "S2", "Username maps to explicit profile");
        auto studentResults = system.GetAcademicResults();
        Check(studentResults.size() == 1 && studentResults[0].StudentID == "S2" && studentResults[0].SubjectName == "Data structures", "Desktop results include only own course metadata");
        Reject([&] { system.GetStudents(); }, "Student cannot read all profiles");
        Reject([&] { system.GetAccounts(); }, "Student cannot read account list");
        Reject([&] { system.Register("E3", "S2", "C1", Date()); }, "Student cannot register via admin API");
        Reject([&] { system.SetGrade("E2", Grade(10, 10, 10)); }, "Student cannot edit grade");
        Reject([&] { system.ResetPassword("admin", "bad"); }, "Student cannot reset other passwords");
        Check(system.Login("teacher2", "teacher-pass"), "Teacher login");
        Check(system.GetTeachingSections().size() == 1 && system.GetTeachingSections()[0]->GetID() == "C2", "Teacher sees assigned sections");
        Check(system.GetVisibleEnrollments().size() == 1 && system.GetVisibleEnrollments()[0]->GetID() == "E2", "Teacher sees assigned grades");
        Check(system.GetTeachingSectionDetails().size() == 1 && system.GetTeachingSectionDetails()[0].ID == "C2", "Desktop section metadata respects teacher assignment");
        Reject([&] { system.SetGrade("E1", Grade(10, 10, 10)); }, "Teacher cannot grade another section");
        Reject([&] { system.GetStudents(); }, "Teacher cannot read all profiles");
        Reject([&] { system.RemoveStudent("S1"); }, "Teacher cannot delete student");
        system.SetGrade("E2", Grade(10, 8, 7));
        Check(std::abs(system.GetVisibleEnrollments()[0]->GetScore().GetTotal() - 7.6f) < 0.0001f, "Teacher updates grade");
        Check(system.Login("student2", "new-pass"), "Student relogin");
        Reject([&] { system.ChangeOwnPassword("wrong", "pass"); }, "Old password verified");
        system.ChangeOwnPassword("new-pass", "quoted\"password\\value");
        system.Logout();
        Reject([&] { system.GetVisibleEnrollments(); }, "Logout revokes read access");
        Reject([&] { system.GetAcademicResults(); }, "Logout revokes desktop result access");
        Check(system.Login("admin", "admin-pass"), "Admin relogin");
        system.UpdateRegistrationDate("E2", Date(24, 10, 2026));
        Check(std::abs(system.GetEnrollments().FindByID("E2")->GetScore().GetTotal() - 7.6f) < 0.0001f, "Registration date update preserves grade");
        system.Save();
    }
    SchoolManager loaded(directory.Path.string());
    Check(loaded.Login("student2", "quoted\"password\\value"), "Escaped password survives restart");
    auto visible = loaded.GetVisibleEnrollments();
    Check(visible.size() == 1 && visible[0]->GetEnrollmentDate().to_string() == "24/10/2026", "Registration date survives restart");
    Check(std::abs(visible[0]->GetScore().GetTotal() - 7.6f) < 0.0001f, "Grades survive restart");
    Check(loaded.Login("admin", "admin-pass"), "Persisted admin login");
    Check(loaded.GetStudents().FindByID("S1")->GetFullName() == "Nguyen \"A\" \\ B", "Quoted and backslash text round trip");
    Check(loaded.GetSections().FindByID("C2")->GetStudentNumber() == 1, "Capacity survives restart");
    Check(!loaded.HasUnsavedChanges(), "Read does not dirty data");
}

void CorruptDataAndFailedSave() {
    TestDirectory directory;
    const auto file = directory.Path / "students.txt";
    Put(file, "\"S1\" \"Student\" 1 1 2000 \"\"\n");
    StudentManager manager(file.string());
    Check(manager.Size() == 1, "Load valid record");
    Put(file, "\xEF\xBB\xBF# UTF-8 BOM and CRLF\r\n\"S1\" \"Student\" 1 1 2000 \"\"\r\n");
    manager.LoadFromFile();
    Check(manager.Size() == 1 && manager.FindByID("S1"), "Windows UTF-8 BOM and CRLF accepted");
    Put(file, "\"S2\" \"Student\" 1 1 2000 \"\"\nmalformed\n");
    Reject([&] { manager.LoadFromFile(); }, "Malformed line rejected");
    Check(manager.Size() == 1 && manager.FindByID("S1"), "Failed load preserves old list");
    Put(file, "\"S1\" \"Student\" 1 1 2000 \"\"\n\"S1\" \"Duplicate\" 1 1 2000 \"\"\n");
    Reject([&] { manager.LoadFromFile(); }, "Duplicate file IDs rejected");
    Put(file, "\"S1\" \"Student\" 29 2 1900 \"\"\n");
    Reject([&] { manager.LoadFromFile(); }, "Invalid persisted date rejected");
    Put(file, "\"S1\" \"Student\" 1 1 2000 \"\" extra\n");
    Reject([&] { manager.LoadFromFile(); }, "Extra fields rejected");
    fs::create_directory(file.string() + ".tmp");
    const auto before = Read(file);
    Reject([&] { manager.SaveToFile(); }, "Save reports blocked temporary path");
    Check(Read(file) == before, "Failed save preserves original file");
    Check(fs::is_directory(file.string() + ".tmp"), "Existing temporary paths are not deleted");
    fs::remove(file.string() + ".tmp");
    manager.SaveToFile();
    StudentManager valid(file.string());
    Check(valid.FindByID("S1"), "Retry save succeeds");

    TestDirectory schoolDirectory;
    SchoolManager school(schoolDirectory.Path.string());
    Seed(school);
    school.Save();
    const auto subjects = schoolDirectory.Path / "subjects.txt";
    const auto savedSubjects = Read(subjects);
    Put(subjects, "\"M2\" \"Data structures\" 4\n");
    Reject([&] { SchoolManager invalid(schoolDirectory.Path.string()); }, "Broken foreign key rejected");
    Put(subjects, savedSubjects);
    Put(schoolDirectory.Path / "course_sections.txt", "\"C1\" \"M1\" \"T1\" 1 0\n\"C2\" \"M2\" \"T2\" 2 1\n");
    Reject([&] { SchoolManager invalid(schoolDirectory.Path.string()); }, "Persisted count mismatch rejected");
    school.Save();
    const auto oldAccounts = Read(schoolDirectory.Path / "accounts.txt");
    const auto oldStudents = Read(schoolDirectory.Path / "students.txt");
    fs::remove(subjects);
    fs::create_directory(subjects);
    school.UpdateStudent(Student("S1", "Not yet saved", 1, 1, 2005));
    Reject([&] { school.Save(); }, "Multi-file save reports invalid target");
    Check(school.HasUnsavedChanges(), "Failed save retains dirty state");
    Check(Read(schoolDirectory.Path / "accounts.txt") == oldAccounts &&
          Read(schoolDirectory.Path / "students.txt") == oldStudents, "Staging failure preserves other files");
    fs::remove(subjects);
    Put(subjects, savedSubjects);
    school.Save();
    SchoolManager retried(schoolDirectory.Path.string());
    Check(retried.Login("admin", "admin-pass"), "Can reload after retry");
    Check(retried.GetStudents().FindByID("S1")->GetFullName() == "Not yet saved", "Retry writes pending state");
}

void InterruptedSaveRecovery() {
    TestDirectory directory;
    SchoolManager system(directory.Path.string());
    Seed(system);
    system.Save();
    const auto journal = directory.Path / ".student-management-save";
    fs::create_directories(journal / "old");
    for (const auto& file : {"accounts.txt", "students.txt", "teachers.txt", "subjects.txt", "course_sections.txt", "enrollments.txt"})
        fs::copy_file(directory.Path / file, journal / "old" / file);
    Put(journal / "prepared", "complete\n");
    Put(directory.Path / "students.txt", "partially written invalid state\n");
    SchoolManager recovered(directory.Path.string());
    Check(recovered.Login("admin", "admin-pass"), "Recovery runs before loading");
    Check(recovered.GetStudents().Size() == 3, "Interrupted save restores old snapshot");
    Check(!fs::exists(journal), "Recovery cleans journal");
    recovered.UpdateStudent(Student("S1", "Committed Name", 1, 1, 2005));
    recovered.Save();
    fs::create_directories(journal / "old");
    Put(journal / "prepared", "complete\n");
    Put(journal / "committed", "complete\n");
    Put(journal / "old" / "students.txt", "obsolete snapshot\n");
    SchoolManager committed(directory.Path.string());
    Check(committed.Login("admin", "admin-pass"), "Committed journal reload");
    Check(committed.GetStudents().FindByID("S1")->GetFullName() == "Committed Name", "Committed snapshot is not rolled back");
}
}

int main() {
    try {
        ModelsAndOwnership();
        BusinessRules();
        PermissionsAndRoundTrip();
        CorruptDataAndFailedSave();
        InterruptedSaveRecovery();
        std::cout << Checks << " checks passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAILED after " << Checks << " checks: " << error.what() << '\n';
        return 1;
    }
}
