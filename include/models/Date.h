#pragma once
#include <string>
class Date{
    private:
        int day, month, year;
    public:
        Date(int = 1, int = 1, int = 2000);
        ~Date();

        int getDay() const;
        int getMonth() const;
        int getYear() const;
        void SetDay(int value);
        void SetMonth(int value);
        void SetYear(int value);

        int dayspermonth(const int&, const int&) const;
        std::string to_string() const; //Bao gồm cả chuẩn hóa ngày tháng năm
};
