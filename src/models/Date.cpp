#include <sstream>
#include <iomanip>
#include "models/Date.h"
using namespace std;
Date::Date(int day, int month, int year) : day(day), month(month), year(year){
}
Date::~Date(){}
int Date::getDay(){
    return this->day;
}
int Date::getMonth(){
    return this->month;
}
int Date::getYear(){
    return this->year;
}
int Date::dayspermonth(const int& month, const int& year) const{
    int day;
    if(month == 4 || month == 6 || month == 9 || month == 11){
        day = 30;
        return day;
    }
    if(year % 400 == 0 && year % 100 != 0 && year % 4 == 0){
        if(month == 2){
            day = 29;
            return day;
        }
    }
    else{
        if(month == 2){
            day = 28;
            return day;
        }
    }
    day = 31;
    return day;
}
string Date::to_string(){
    stringstream ss;

    ss << setfill('0')
       << setw(2) << day << '/'
       << setw(2) << month << '/'
       << setw(4) << year;

    return ss.str();
}
