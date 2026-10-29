#include <iostream>
#include <exception>
#include "managers/AccountManager.h"
#include "managers/StudentManager.h"
#include "models/Student.h"
#include "managers/TeacherManager.h"
using namespace std;
int main() {
    // try {
    //     AccountManager accounts("data/accounts.txt");
    //     Account* check = accounts.Login("SV005", "xin chao");
    //     if(check == nullptr){
    //         cout << "Ten dang nhap hoac mat khau khong dung";
    //     }
    //     else check->DisplayInfo();
    // } catch (const exception& error) {
    //     cerr << "Loi: " << error.what() << '\n';
    //     return 1;
    // }
    // try{
    //     StudentManager hocsinh("data/students.txt");
    //     Student* student = new Student("SV002", "Nguyen Cong Dung", 20, 12, 2007, "nguyencongdung@codeforces.com");
    //     bool check = hocsinh.Add(student);
    //     if(check == false){
    //         cout << "Them that bai";
    //     }
    //     else student->DisplayInfo();
    // }
    // catch (const exception& error) {
    // cerr << "Loi: " << error.what() << '\n';
    // return 1;
// }
    try{
        TeacherManager teacher("data/teachers.txt");
        Teacher* check = teacher.FindByID("GV001");
        teacher.PrintAll();
    }
    catch (const exception& error) {
    cerr << "Loi: " << error.what() << '\n';
    return 1;}
}
