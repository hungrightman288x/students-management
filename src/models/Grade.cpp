
#include "models/Grade.h"
#include <stdexcept>
#include <cmath>
using namespace std;

Grade::Grade(float att, float mid, float final)
    : Attendance(0), Midterm(0), Final(0), Total(0) {

    this->SetAttendance(att);
    this->SetMidterm(mid);
    this->SetFinal(final);
}

float Grade::GetAttendance() const {
    return this->Attendance;
}

float Grade::GetMidterm() const {
    return this->Midterm;
}

float Grade::GetFinal() const {
    return this->Final;
}

float Grade::GetTotal() const {
    return this->Total;
}

void Grade::SetAttendance(float att) {
    if (!std::isfinite(att) || att < 0 || att > 10) {
        throw invalid_argument("Diem chuyen can khong hop le");
    }
    this->Attendance = att;
    this->CalculateTotal();
}

void Grade::SetMidterm(float mid) {
    if (!std::isfinite(mid) || mid < 0 || mid > 10) {
        throw invalid_argument("Diem giua ky khong hop le");
    }
    this->Midterm = mid;
    this->CalculateTotal();
}

void Grade::SetFinal(float final) {
    if (!std::isfinite(final) || final < 0 || final > 10) {
        throw invalid_argument("Diem cuoi ky khong hop le");
    }
    this->Final = final;
    this->CalculateTotal();
}

void Grade::CalculateTotal() {
    this->Total = this->Attendance * 0.1f
                + this->Midterm * 0.3f
                + this->Final * 0.6f;
}
