#pragma once
#include <string>
using namespace std;
class Date{
    private:
        int day, month, year;
    public:
        Date(int = 1, int = 1, int = 2000);
        ~Date();

        int getDay() const;
        int getMonth() const;
        int getYear() const;

        int dayspermonth(const int&, const int&) const;
        string to_string() const; //Bao gồm cả chuẩn hóa ngày tháng năm
};