#include "ui/Menu.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace {
struct EndOfInput {};

std::string Text(const std::string& prompt, bool required = true) {
    while (true) {
        std::cout << prompt;
        std::string result;
        if (!std::getline(std::cin, result)) throw EndOfInput{};
        if (!result.empty() && result.back() == '\r') result.pop_back();
        try {
            ValidateText(result, "Du lieu", required);
            return result;
        } catch (const std::exception& error) { std::cout << error.what() << '\n'; }
    }
}

template<class T> T Number(const std::string& prompt) {
    while (true) {
        std::istringstream input(Text(prompt));
        T result;
        if (input >> result) {
            input >> std::ws;
            if (input.eof()) return result;
        }
        std::cout << "Hay nhap mot so hop le.\n";
    }
}

int Choice(int maximum) {
    while (true) {
        int choice = Number<int>("Chon: ");
        if (choice >= 0 && choice <= maximum) return choice;
        std::cout << "Lua chon phai tu 0 den " << maximum << ".\n";
    }
}

Date ReadDate(const std::string& label) {
    std::cout << label << " (nhap ngay, thang, nam rieng tung dong)\n";
    int day = Number<int>("Ngay: ");
    int month = Number<int>("Thang: ");
    int year = Number<int>("Nam: ");
    return Date(day, month, year);
}

template<class T> T ReadPerson() {
    auto id = Text("Ma: ");
    auto name = Text("Ho ten: ");
    auto birth = ReadDate("Ngay sinh");
    auto email = Text("Email (co the de trong): ", false);
    return T(id, name, birth.getDay(), birth.getMonth(), birth.getYear(), email);
}

template<class F> void Action(F operation) {
    try { operation(); }
    catch (const std::exception& error) { std::cout << "Loi: " << error.what() << '\n'; }
}

template<class T, class Reader, class Add, class Update, class Remove, class Save>
void Manage(const std::string& title, const Manager<T>& manager, Reader read,
            Add add, Update update, Remove remove, Save save) {
    while (true) {
        std::cout << "\n=== " << title << " ===\n"
                  << "1. Danh sach\n2. Them\n3. Sua (giu nguyen ma)\n4. Xoa\n5. Tim theo ma\n0. Quay lai\n";
        int choice = Choice(5);
        if (!choice) return;
        Action([&] {
            if (choice == 1) manager.PrintAll();
            else if (choice == 5) manager.PrintByID(Text("Ma can tim: "));
            else if (choice == 4) { remove(Text("Ma can xoa: ")); save(); }
            else {
                T item = read();
                if (choice == 2) add(item); else update(item);
                save();
            }
        });
    }
}

void PrintGrades(const std::vector<const Enrollment*>& items) {
    if (items.empty()) { std::cout << "Chua co dang ky hoc.\n"; return; }
    std::cout << std::left << std::setw(15) << "Ma dang ky" << std::setw(15) << "Sinh vien"
              << std::setw(15) << "Lop" << std::setw(13) << "Ngay DK"
              << std::setw(10) << "Chuyen can" << std::setw(10) << "Giua ky"
              << std::setw(10) << "Cuoi ky" << "Tong\n";
    const auto oldFlags = std::cout.flags();
    const auto oldPrecision = std::cout.precision();
    std::cout << std::fixed << std::setprecision(2);
    for (auto item : items) {
        const auto& grade = item->GetScore();
        std::cout << std::setw(15) << item->GetID() << std::setw(15) << item->GetStudentId()
                  << std::setw(15) << item->GetCourseSectionId() << std::setw(13) << item->GetEnrollmentDate().to_string()
                  << std::setw(10) << grade.GetAttendance() << std::setw(10) << grade.GetMidterm()
                  << std::setw(10) << grade.GetFinal() << grade.GetTotal() << '\n';
    }
    std::cout.flags(oldFlags);
    std::cout.precision(oldPrecision);
    std::cout << "Tong = 10% chuyen can + 30% giua ky + 60% cuoi ky.\n";
}

Role ReadRole() {
    std::cout << "Vai tro: 0 = STUDENT, 1 = TEACHER, 2 = ADMIN\n";
    return static_cast<Role>(Choice(2));
}

std::string ReadProfile(Role role) {
    return role == ADMIN ? "" : Text(role == STUDENT ? "Ma sinh vien: " : "Ma giao vien: ");
}
}

Menu::Menu(SchoolManager& system) : System(system) {}

void Menu::DisplayMenu() const {
    std::cout << "\n=== QUAN LY SINH VIEN ===\n1. Dang nhap\n0. Thoat\n";
}

void Menu::SaveChanges() {
    try {
        System.Save();
        std::cout << "Da luu du lieu.\n";
    } catch (...) {
        std::cout << "Chua luu duoc. Thay doi van con trong bo nho; hay thu Luu lai.\n";
        throw;
    }
}

void Menu::Logout() {
    if (System.HasUnsavedChanges()) SaveChanges();
    System.Logout();
    std::cout << "Da dang xuat.\n";
}

void Menu::Run() {
    try {
        if (System.NeedsInitialAdmin()) {
            std::cout << "Lan chay dau: tao tai khoan admin.\n";
            while (System.NeedsInitialAdmin()) {
                Action([&] {
                    auto username = Text("Ten dang nhap: ");
                    auto password = Text("Mat khau: ");
                    System.CreateInitialAdmin(username, password);
                });
            }
            SaveChanges();
        }
        while (true) {
            DisplayMenu();
            if (Choice(1) == 0) break;
            auto username = Text("Ten dang nhap: ");
            auto password = Text("Mat khau: ");
            if (!System.Login(username, password)) {
                std::cout << "Sai ten dang nhap hoac mat khau.\n";
                continue;
            }
            std::cout << "Dang nhap thanh cong: " << username << '\n';
            switch (System.GetCurrentAccount()->GetUserRole()) {
                case ADMIN: DisplayAdminMenu(); break;
                case TEACHER: DisplayTeacherMenu(); break;
                case STUDENT: DisplayStudentMenu(); break;
            }
        }
    } catch (const EndOfInput&) { std::cout << "\nKet thuc nhap lieu.\n"; }
    if (System.HasUnsavedChanges()) SaveChanges();
    System.Logout();
}

void Menu::DisplayAdminMenu() {
    while (System.GetCurrentAccount()) {
        std::cout << "\n=== ADMIN ===\n1. Sinh vien\n2. Giao vien\n3. Mon hoc\n4. Lop hoc phan\n"
                  << "5. Dang ky hoc / xem diem\n6. Tai khoan\n7. Luu du lieu\n8. Doi mat khau\n0. Dang xuat\n";
        int choice = Choice(8);
        Action([&] {
            switch (choice) {
                case 0: Logout(); break;
                case 1: ManageStudents(); break;
                case 2: ManageTeachers(); break;
                case 3: ManageSubjects(); break;
                case 4: ManageSections(); break;
                case 5: ManageEnrollments(); break;
                case 6: ManageAccounts(); break;
                case 7: SaveChanges(); break;
                case 8: ChangePassword(); break;
            }
        });
    }
}

void Menu::ManageStudents() {
    Manage<Student>("SINH VIEN", System.GetStudents(), [] { return ReadPerson<Student>(); },
        [&](const Student& item) { System.AddStudent(item); },
        [&](const Student& item) { System.UpdateStudent(item); },
        [&](const std::string& id) { System.RemoveStudent(id); }, [&] { SaveChanges(); });
}
void Menu::ManageTeachers() {
    Manage<Teacher>("GIAO VIEN", System.GetTeachers(), [] { return ReadPerson<Teacher>(); },
        [&](const Teacher& item) { System.AddTeacher(item); },
        [&](const Teacher& item) { System.UpdateTeacher(item); },
        [&](const std::string& id) { System.RemoveTeacher(id); }, [&] { SaveChanges(); });
}
void Menu::ManageSubjects() {
    Manage<Subject>("MON HOC", System.GetSubjects(), [] {
        auto id = Text("Ma mon: ");
        auto name = Text("Ten mon: ");
        int credits = Number<int>("So tin chi: ");
        return Subject(id, name, credits);
    }, [&](const Subject& item) { System.AddSubject(item); },
       [&](const Subject& item) { System.UpdateSubject(item); },
       [&](const std::string& id) { System.RemoveSubject(id); }, [&] { SaveChanges(); });
}
void Menu::ManageSections() {
    Manage<CourseSection>("LOP HOC PHAN", System.GetSections(), [] {
        auto id = Text("Ma lop: ");
        auto subject = Text("Ma mon: ");
        auto teacher = Text("Ma giao vien: ");
        int maximum = Number<int>("Si so toi da: ");
        return CourseSection(id, subject, teacher, maximum);
    }, [&](const CourseSection& item) {
        System.AddSection(item.GetID(), item.GetSubjectId(), item.GetTeacherId(), item.GetMaxStudents());
    }, [&](const CourseSection& item) {
        System.UpdateSection(item.GetID(), item.GetSubjectId(), item.GetTeacherId(), item.GetMaxStudents());
    }, [&](const std::string& id) { System.RemoveSection(id); }, [&] { SaveChanges(); });
}

void Menu::ManageEnrollments() {
    while (true) {
        std::cout << "\n=== DANG KY HOC ===\n1. Danh sach / diem\n2. Dang ky\n3. Sua ngay dang ky\n"
                  << "4. Huy dang ky\n5. Tim theo ma\n0. Quay lai\n";
        int choice = Choice(5);
        if (!choice) return;
        Action([&] {
            if (choice == 1) PrintGrades(System.GetVisibleEnrollments());
            else if (choice == 5) System.GetEnrollments().PrintByID(Text("Ma dang ky: "));
            else if (choice == 4) { System.CancelRegistration(Text("Ma dang ky: ")); SaveChanges(); }
            else {
                auto id = Text("Ma dang ky: ");
                if (choice == 2) {
                    auto student = Text("Ma sinh vien: ");
                    auto section = Text("Ma lop: ");
                    auto date = ReadDate("Ngay dang ky");
                    System.Register(id, student, section, date);
                } else {
                    auto date = ReadDate("Ngay dang ky moi");
                    System.UpdateRegistrationDate(id, date);
                }
                SaveChanges();
            }
        });
    }
}

void Menu::ManageAccounts() {
    while (true) {
        std::cout << "\n=== TAI KHOAN ===\n1. Danh sach\n2. Them\n3. Sua vai tro / ho so\n"
                  << "4. Xoa\n5. Tim theo ten dang nhap\n6. Dat lai mat khau\n0. Quay lai\n";
        int choice = Choice(6);
        if (!choice) return;
        Action([&] {
            if (choice == 1) { System.GetAccounts().PrintAll(); return; }
            auto username = Text("Ten dang nhap: ");
            if (choice == 5) { System.GetAccounts().PrintByID(username); return; }
            if (choice == 2) {
                auto password = Text("Mat khau: ");
                auto role = ReadRole();
                auto profile = ReadProfile(role);
                System.AddAccount(Account(username, password, role, profile));
            } else if (choice == 3) {
                auto role = ReadRole();
                auto profile = ReadProfile(role);
                System.UpdateAccountProfile(username, role, profile);
            } else if (choice == 4) System.RemoveAccount(username);
            else System.ResetPassword(username, Text("Mat khau moi: "));
            SaveChanges();
        });
    }
}

void Menu::ChangePassword() {
    auto oldPassword = Text("Mat khau cu: ");
    auto newPassword = Text("Mat khau moi: ");
    auto confirmation = Text("Nhap lai mat khau moi: ");
    if (newPassword != confirmation) throw std::invalid_argument("Hai mat khau moi khong khop");
    System.ChangeOwnPassword(oldPassword, newPassword);
    SaveChanges();
}

void Menu::DisplayTeacherMenu() {
    while (System.GetCurrentAccount()) {
        std::cout << "\n=== GIAO VIEN ===\n1. Cac lop phu trach\n2. Xem diem theo lop\n3. Nhap / sua diem\n"
                  << "4. Thong tin ca nhan\n5. Doi mat khau\n0. Dang xuat\n";
        int choice = Choice(5);
        Action([&] {
            if (choice == 0) Logout();
            else if (choice == 1) {
                auto sections = System.GetTeachingSections();
                if (sections.empty()) std::cout << "Chua duoc phan cong lop.\n";
                for (auto item : sections) { item->DisplayInfo(); std::cout << "--------------------\n"; }
            } else if (choice == 2) {
                auto id = Text("Ma lop (de trong de xem tat ca lop phu trach): ", false);
                auto items = System.GetVisibleEnrollments();
                if (!id.empty()) {
                    auto sections = System.GetTeachingSections();
                    if (std::none_of(sections.begin(), sections.end(), [&](const CourseSection* item) { return item->GetID() == id; }))
                        throw std::runtime_error("Ban khong phu trach lop nay");
                    items.erase(std::remove_if(items.begin(), items.end(), [&](const Enrollment* item) {
                        return item->GetCourseSectionId() != id;
                    }), items.end());
                }
                PrintGrades(items);
            } else if (choice == 3) {
                auto id = Text("Ma dang ky can nhap diem: ");
                float attendance = Number<float>("Diem chuyen can (0-10): ");
                float midterm = Number<float>("Diem giua ky (0-10): ");
                float final = Number<float>("Diem cuoi ky (0-10): ");
                System.SetGrade(id, Grade(attendance, midterm, final));
                SaveChanges();
            } else if (choice == 4) System.GetOwnTeacherProfile().DisplayInfo();
            else ChangePassword();
        });
    }
}

void Menu::DisplayStudentMenu() {
    while (System.GetCurrentAccount()) {
        std::cout << "\n=== SINH VIEN ===\n1. Thong tin ca nhan\n2. Lop da dang ky / diem cua toi\n3. Doi mat khau\n0. Dang xuat\n";
        int choice = Choice(3);
        Action([&] {
            if (choice == 0) Logout();
            else if (choice == 1) System.GetOwnStudentProfile().DisplayInfo();
            else if (choice == 2) PrintGrades(System.GetVisibleEnrollments());
            else ChangePassword();
        });
    }
}
