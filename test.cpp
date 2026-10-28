// Test rieng StudentManager. Bien dich file nay thay cho src/main.cpp.
// Du lieu mau duoc tao trong thu muc tam, khong sua data/students.txt.

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "managers/StudentManager.h"

namespace fs = std::filesystem;

int passed = 0;
int failed = 0;

void Check(bool condition, const std::string& description) {
    if (condition) {
        ++passed;
        std::cout << "[PASS] " << description << '\n';
    } else {
        ++failed;
        std::cout << "[FAIL] " << description << '\n';
    }
}

// Tu don file mau khi test ket thuc, ke ca khi co exception.
struct TestData {
    fs::path directory;
    fs::path file;

    TestData() {
        const auto stamp = std::chrono::steady_clock::now()
                               .time_since_epoch().count();
        directory = fs::temp_directory_path() /
                    ("student-manager-test-" + std::to_string(stamp));

        if (!fs::create_directory(directory)) {
            throw std::runtime_error("Khong tao duoc thu muc test");
        }

        file = directory / "students.txt";
    }

    ~TestData() {
        std::error_code ignored;
        fs::remove(file, ignored);
        fs::remove(directory, ignored);
    }

    void Write(const std::string& contents) const {
        std::ofstream output(file, std::ios::binary);
        if (!output) {
            throw std::runtime_error("Khong mo duoc file du lieu test");
        }

        output << contents;
        output.close();
        if (!output) {
            throw std::runtime_error("Khong ghi duoc file du lieu test");
        }
    }
};

void TestLoadAndManage(const TestData& data) {
    // Co dong trong, xuong dong CRLF va dong cuoi khong co newline.
    data.Write(
        "\r\nSV001|Nguyen Van An|20|1|2005|an@example.com\r\n"
        "\r\nSV002|Tran Thi Binh|29|2|2004|binh@example.com"
    );

    StudentManager students(data.file.string());
    Student* first = students.FindByID("SV001");
    Student* second = students.FindByID("SV002");

    Check(first != nullptr, "Nap sinh vien SV001 tu file");
    Check(second != nullptr, "Nap dong cuoi khong co newline");
    Check(first && first->GetFullName() == "Nguyen Van An",
          "Doc dung ho ten co khoang trang");
    Check(second && second->GetEmail() == "binh@example.com",
          "Doc dung email");
    Check(students.FindByID("SV999") == nullptr,
          "Tim ID khong ton tai tra ve nullptr");
    Check(!students.Add(nullptr), "Tu choi them con tro nullptr");

    Student* added = new Student(
        "SV003", "Le Van Cuong", 1, 6, 2005, "cuong@example.com"
    );
    const bool addSucceeded = students.Add(added);
    if (!addSucceeded) {
        delete added; // Add that bai: manager chua nhan quyen so huu.
    }
    Check(addSucceeded, "Them sinh vien moi thanh cong");
    Check(addSucceeded && students.FindByID("SV003") == added,
          "Map giu dung dia chi doi tuong vua them");

    Student* duplicate = new Student(
        "SV001", "Ten Khac", 1, 1, 2005, "other@example.com"
    );
    const bool duplicateAdded = students.Add(duplicate);
    if (!duplicateAdded) {
        delete duplicate;
    }
    Check(!duplicateAdded, "Tu choi them sinh vien trung ID");
    Student* original = students.FindByID("SV001");
    Check(original == first && original &&
              original->GetFullName() == "Nguyen Van An",
          "Them trung ID khong thay the sinh vien cu");

    Student updated(
        "SV001", "Nguyen Van An Moi", 20, 1, 2005, "new@example.com"
    );
    Check(students.Update(updated), "Cap nhat sinh vien da ton tai");
    Student* afterUpdate = students.FindByID("SV001");
    Check(afterUpdate && afterUpdate == first &&
              afterUpdate->GetFullName() == "Nguyen Van An Moi" &&
              afterUpdate->GetEmail() == "new@example.com",
          "Cap nhat dung du lieu va giu nguyen dia chi doi tuong");

    Student missing("SV999", "Khong Ton Tai", 1, 1, 2005, "none@example.com");
    Check(!students.Update(missing), "Tu choi cap nhat ID khong ton tai");
    Check(!students.Remove(nullptr), "Tu choi xoa con tro nullptr");

    Student* toRemove = students.FindByID("SV003");
    Check(toRemove && students.Remove(toRemove), "Xoa sinh vien da them");
    // Remove thanh cong da delete doi tuong: khong dung lai toRemove.
    Check(students.FindByID("SV003") == nullptr,
          "Khong con tim thay sinh vien sau khi xoa");

    std::cout << "\n--- Kiem tra hien thi danh sach con lai ---\n";
    students.PrintAll();
    std::cout << "--- Kiem tra hien thi rieng SV001 ---\n";
    students.PrintByID("SV001");
}

void TestInvalidFiles(const TestData& data) {
    bool missingFileRejected = false;
    try {
        StudentManager students((data.directory / "missing.txt").string());
    } catch (const std::runtime_error&) {
        missingFileRejected = true;
    }
    Check(missingFileRejected, "Bao loi khi file khong ton tai");

    data.Write("");
    {
        StudentManager empty(data.file.string());
        Check(empty.FindByID("SV001") == nullptr,
              "File rong tao danh sach rong");
    }

    const std::vector<std::pair<std::string, std::string>> cases = {
        {"ID sai dinh dang", "XX001|An|20|1|2005|an@example.com\n"},
        {"Thieu truong email", "SV001|An|20|1|2005\n"},
        {"Thua truong du lieu", "SV001|An|20|1|2005|an@example.com|extra\n"},
        {"Trung ID trong file",
         "SV001|An|20|1|2005|an@example.com\n"
         "SV001|Binh|1|1|2005|binh@example.com\n"},
        {"Ngay 31/2 khong hop le", "SV001|An|31|2|2005|an@example.com\n"},
        {"Ngay 29/2 o nam khong nhuan", "SV001|An|29|2|2005|an@example.com\n"},
        {"Ngay khong phai so", "SV001|An|abc|1|2005|an@example.com\n"},
        {"Nam vuot mien so nguyen",
         "SV001|An|20|1|999999999999999999999999|an@example.com\n"},
        // Cac ca nay phat hien stoi() chap nhan phan dau cua chuoi.
        {"Ngay co ky tu thua: 20abc", "SV001|An|20abc|1|2005|an@example.com\n"},
        {"Thang co ky tu thua: 1abc", "SV001|An|20|1abc|2005|an@example.com\n"},
        {"Nam co ky tu thua: 2005abc", "SV001|An|20|1|2005abc|an@example.com\n"}
    };

    for (const auto& test : cases) {
        data.Write(test.second);
        bool rejected = false;
        try {
            StudentManager students(data.file.string());
        } catch (const std::invalid_argument&) {
            rejected = true;
        } catch (const std::out_of_range&) {
            rejected = true;
        }
        Check(rejected, "Tu choi: " + test.first);
    }
}

int main() {
    try {
        TestData data;
        TestLoadAndManage(data);
        std::cout << "\n--- Kiem tra file rong va du lieu loi ---\n";
        TestInvalidFiles(data);
    } catch (const std::exception& error) {
        ++failed;
        std::cerr << "[FAIL] Loi ngoai du kien: " << error.what() << '\n';
    }

    std::cout << "\nKet qua: " << passed << " PASS, " << failed << " FAIL\n";
    return failed == 0 ? 0 : 1;
}
