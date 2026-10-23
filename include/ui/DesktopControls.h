#pragma once

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <commctrl.h>
#include <functional>
#include <map>
#include <string>
#include <vector>
#include "models/Date.h"

namespace desktop {
constexpr COLORREF Background = RGB(247, 249, 252);
constexpr COLORREF White = RGB(255, 255, 255);
constexpr COLORREF Ink = RGB(24, 34, 52);
constexpr COLORREF Muted = RGB(117, 130, 150);
constexpr COLORREF Border = RGB(228, 233, 240);
constexpr COLORREF Blue = RGB(37, 91, 249);
constexpr COLORREF Teal = RGB(0, 119, 162);
constexpr COLORREF Red = RGB(224, 61, 71);

std::wstring Wide(const std::string& value);
std::string Utf8(const std::wstring& value);
std::wstring WindowText(HWND window);
std::wstring Decimal(float value);
std::wstring RoleName(Role role);
Date ReadDate(const std::wstring& value);
int ReadInteger(const std::wstring& value);
float ReadScore(const std::wstring& value);

struct Theme {
    int Dpi = 96;
    HFONT Normal = nullptr, Bold = nullptr, Heading = nullptr, Brand = nullptr, Small = nullptr;
    explicit Theme(HWND window);
    ~Theme();
    Theme(const Theme&) = delete;
    Theme& operator=(const Theme&) = delete;
    int S(int value) const;
};

void Fill(HDC dc, RECT rectangle, COLORREF color);
void Card(HDC dc, RECT rectangle, COLORREF color = White, int radius = 10);
void Label(HDC dc, const std::wstring& text, RECT rectangle, HFONT font,
           COLORREF color = Ink, UINT format = DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
void DrawIcon(HDC dc, RECT rectangle, int kind, COLORREF color);
void PaintButton(const DRAWITEMSTRUCT& item, const Theme& theme, bool primary, bool danger = false,
                 bool navigation = false, bool selected = false, bool iconOnly = false, int icon = -1);
HWND MakeControl(HWND parent, HINSTANCE instance, const wchar_t* className, const std::wstring& text,
                 DWORD style, int id, HFONT font, DWORD extendedStyle = 0);

using Options = std::vector<std::pair<std::wstring, std::wstring>>; // stored value, display label
enum class FieldKind { Text, Password, Combo };
struct Field {
    std::wstring Key, Label, Value;
    FieldKind Kind = FieldKind::Text;
    Options Choices;
    bool ReadOnly = false;
};
using Values = std::map<std::wstring, std::wstring>;

// Native modal form. The callback validates/mutates in memory before accepting.
// Disk saving happens after acceptance so a failed save cannot add twice.
class Form {
    HWND Window = nullptr, ErrorLabel = nullptr;
    HINSTANCE Instance;
    const Theme& Style;
    std::vector<Field> Fields;
    std::vector<HWND> Inputs;
    std::function<void(const Values&)> Submit;
    std::wstring Title;
    bool ReadOnly;
    Options StudentOptions, TeacherOptions;
    void Initialize();
    void SetChoices(std::size_t index);
    void UpdateProfileChoices(bool initial = false);
    Values Collect() const;
    static INT_PTR CALLBACK Procedure(HWND, UINT, WPARAM, LPARAM);

public:
    Form(HINSTANCE instance, const Theme& theme, std::wstring title, std::vector<Field> fields,
         std::function<void(const Values&)> submit = {}, bool readOnly = false);
    void SetProfileOptions(Options students, Options teachers);
    bool Show(HWND parent);
};
}
