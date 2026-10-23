#include <sstream>
#include <iomanip>
#include <stdexcept>
#include "models/Date.h"
using namespace std;

Date::Date(int day, int month, int year) : day(day), month(month), year(year){
    //Hàm này kiểm tra tháng, năm trước khi trả số ngày
    int maxDay = dayspermonth(month, year);
    if (day < 1 || day > maxDay){
        throw std::invalid_argument("Ngay khong hop le");
    }        
}

Date::~Date(){}

int Date::getDay() const{
    return this->day;
}
int Date::getMonth() const{
    return this->month;
}
int Date::getYear() const{
    return this->year;
}

int Date::dayspermonth(const int& month, const int& year) const{
    if (month < 1 || month > 12 || year < 1 || year > 9999) {
        throw std::invalid_argument("Thang hoac nam khong hop le");
    }
    if (month == 2){
        bool leapYear = year % 400 == 0 ||
                        (year % 4 == 0 && year % 100 != 0);

        return leapYear ? 29 : 28;
    }
    if (month == 4 || month == 6 || month == 9 || month == 11){
        return 30;
    }
    return 31;
}

string Date::to_string() const{
    stringstream ss;

    ss << setfill('0')
       << setw(2) << day << '/'
       << setw(2) << month << '/'
       << setw(4) << year;

    return ss.str();
}
