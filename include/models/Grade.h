
#pragma once

class Grade {
    private:
        float Attendance;
        float Midterm;
        float Final;
        float Total;

    public:
        Grade(float = 0, float = 0, float = 0);

        float GetAttendance() const;
        float GetMidterm() const;
        float GetFinal() const;
        float GetTotal() const;

        void SetAttendance(float att);
        void SetMidterm(float mid);
        void SetFinal(float final);

        void CalculateTotal();
};
