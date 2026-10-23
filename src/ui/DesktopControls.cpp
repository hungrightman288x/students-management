#include "ui/DesktopControls.h"
#include "models/Role.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace desktop {
std::wstring Wide(const std::string& value) {
    if (value.empty()) return {};
    int length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(), static_cast<int>(value.size()), nullptr, 0);
    if (!length) throw std::runtime_error("Du lieu khong phai UTF-8 hop le");
    std::wstring result(length, L'\0');
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(), static_cast<int>(value.size()), result.data(), length);
    return result;
}
std::string Utf8(const std::wstring& value) {
    if (value.empty()) return {};
    int length = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value.data(), static_cast<int>(value.size()), nullptr, 0, nullptr, nullptr);
    if (!length) throw std::runtime_error("Ky tu Unicode khong hop le");
    std::string result(length, '\0');
    WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value.data(), static_cast<int>(value.size()), result.data(), length, nullptr, nullptr);
    return result;
}
std::wstring WindowText(HWND window) {
    int length = GetWindowTextLengthW(window);
    std::wstring result(length + 1, L'\0');
    GetWindowTextW(window, result.data(), length + 1);
    result.resize(length);
    return result;
}
std::wstring Decimal(float value) {
    std::wostringstream out;
    out << std::fixed << std::setprecision(2) << value;
    return out.str();
}
std::wstring RoleName(Role role) {
    return role == ADMIN ? L"Quản trị viên" : role == TEACHER ? L"Giáo viên" : L"Sinh viên";
}
Date ReadDate(const std::wstring& value) {
    std::wistringstream input(value);
    int day, month, year;
    wchar_t first, second;
    if (!(input >> day >> first >> month >> second >> year) || first != L'/' || second != L'/')
        throw std::invalid_argument("Ngay phai theo dinh dang dd/mm/yyyy");
    input >> std::ws;
    if (!input.eof()) throw std::invalid_argument("Ngay phai theo dinh dang dd/mm/yyyy");
    return Date(day, month, year);
}
int ReadInteger(const std::wstring& value) {
    std::wistringstream input(value);
    int result;
    if (!(input >> result)) throw std::invalid_argument("Hay nhap so nguyen hop le");
    input >> std::ws;
    if (!input.eof()) throw std::invalid_argument("Hay nhap so nguyen hop le");
    return result;
}
float ReadScore(const std::wstring& value) {
    auto normalized = value;
    std::replace(normalized.begin(), normalized.end(), L',', L'.');
    std::wistringstream input(normalized);
    float result;
    if (!(input >> result) || !std::isfinite(result) || result < 0 || result > 10)
        throw std::invalid_argument("Diem phai la so tu 0 den 10");
    input >> std::ws;
    if (!input.eof()) throw std::invalid_argument("Diem phai la so tu 0 den 10");
    return result;
}

Theme::Theme(HWND window) {
    HDC dc = GetDC(window);
    Dpi = GetDeviceCaps(dc, LOGPIXELSY);
    ReleaseDC(window, dc);
    auto font = [&](int size, int weight) {
        return CreateFontW(-S(size), 0, 0, 0, weight, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    };
    Normal = font(14, FW_NORMAL); Bold = font(14, FW_SEMIBOLD);
    Heading = font(23, FW_SEMIBOLD); Brand = font(27, FW_BOLD); Small = font(12, FW_NORMAL);
}
Theme::~Theme() {
    for (auto font : {Normal, Bold, Heading, Brand, Small}) if (font) DeleteObject(font);
}
int Theme::S(int value) const { return MulDiv(value, Dpi, 96); }

void Fill(HDC dc, RECT rectangle, COLORREF color) {
    HBRUSH brush = CreateSolidBrush(color);
    FillRect(dc, &rectangle, brush);
    DeleteObject(brush);
}
void Card(HDC dc, RECT rectangle, COLORREF color, int radius) {
    HPEN pen = CreatePen(PS_SOLID, 1, Border);
    HBRUSH brush = CreateSolidBrush(color);
    auto oldPen = SelectObject(dc, pen);
    auto oldBrush = SelectObject(dc, brush);
    RoundRect(dc, rectangle.left, rectangle.top, rectangle.right, rectangle.bottom, radius, radius);
    SelectObject(dc, oldPen); SelectObject(dc, oldBrush);
    DeleteObject(pen); DeleteObject(brush);
}
void Label(HDC dc, const std::wstring& text, RECT rectangle, HFONT font, COLORREF color, UINT format) {
    auto previous = SelectObject(dc, font);
    SetBkMode(dc, TRANSPARENT); SetTextColor(dc, color);
    DrawTextW(dc, text.c_str(), static_cast<int>(text.size()), &rectangle, format | DT_NOPREFIX);
    SelectObject(dc, previous);
}
void DrawIcon(HDC dc, RECT rectangle, int kind, COLORREF color) {
    int x = rectangle.left, y = rectangle.top;
    int w = rectangle.right - x, h = rectangle.bottom - y;
    auto px = [&](int value) { return x + MulDiv(w, value, 24); };
    auto py = [&](int value) { return y + MulDiv(h, value, 24); };
    HPEN pen = CreatePen(PS_SOLID, std::max(1, w / 13), color);
    auto oldPen = SelectObject(dc, pen);
    auto oldBrush = SelectObject(dc, GetStockObject(NULL_BRUSH));
    if (kind == 1) { // profile
        Ellipse(dc, px(8), py(3), px(16), py(11));
        Arc(dc, px(4), py(11), px(20), py(29), px(4), py(21), px(20), py(21));
    } else if (kind == 2) { // book
        RoundRect(dc, px(3), py(4), px(21), py(21), 3, 3);
        MoveToEx(dc, px(12), py(4), nullptr); LineTo(dc, px(12), py(21));
    } else if (kind == 3) { // logout
        MoveToEx(dc, px(10), py(3), nullptr); LineTo(dc, px(3), py(3)); LineTo(dc, px(3), py(21)); LineTo(dc, px(10), py(21));
        MoveToEx(dc, px(9), py(12), nullptr); LineTo(dc, px(22), py(12));
        MoveToEx(dc, px(17), py(7), nullptr); LineTo(dc, px(22), py(12)); LineTo(dc, px(17), py(17));
    } else if (kind == 4) { // collapse
        RoundRect(dc, px(3), py(3), px(21), py(21), 3, 3);
        MoveToEx(dc, px(9), py(3), nullptr); LineTo(dc, px(9), py(21));
        MoveToEx(dc, px(16), py(8), nullptr); LineTo(dc, px(12), py(12)); LineTo(dc, px(16), py(16));
    } else if (kind == 5) { // key
        Ellipse(dc, px(3), py(3), px(13), py(13));
        MoveToEx(dc, px(11), py(11), nullptr); LineTo(dc, px(21), py(21));
        MoveToEx(dc, px(16), py(16), nullptr); LineTo(dc, px(19), py(13));
    } else {
        RoundRect(dc, px(5), py(4), px(20), py(22), 3, 3);
        Rectangle(dc, px(9), py(2), px(16), py(6));
        for (int i = 10; i <= 18; i += 4) {
            MoveToEx(dc, px(9), py(i), nullptr); LineTo(dc, px(16), py(i));
        }
    }
    SelectObject(dc, oldPen); SelectObject(dc, oldBrush); DeleteObject(pen);
}
void PaintButton(const DRAWITEMSTRUCT& item, const Theme& theme, bool primary, bool danger,
                 bool navigation, bool selected, bool iconOnly, int icon) {
    const bool disabled = (item.itemState & ODS_DISABLED) != 0;
    const bool pressed = (item.itemState & ODS_SELECTED) != 0;
    COLORREF color = disabled ? Muted : primary ? White : danger ? Red : selected ? Blue : Ink;
    COLORREF background = primary ? (pressed ? RGB(0, 91, 127) : Teal) :
        selected ? RGB(220, 234, 255) : pressed ? RGB(235, 241, 248) : White;
    RECT rectangle = item.rcItem;
    Fill(item.hDC, rectangle, White);
    if (primary || selected || !navigation) Card(item.hDC, rectangle, background, theme.S(8));
    RECT text = rectangle;
    if (icon >= 0) {
        RECT image{rectangle.left + theme.S(iconOnly ? 12 : 14), rectangle.top + (rectangle.bottom - rectangle.top - theme.S(21)) / 2, 0, 0};
        image.right = image.left + theme.S(21); image.bottom = image.top + theme.S(21);
        DrawIcon(item.hDC, image, icon, color);
        text.left += theme.S(44);
    }
    if (!iconOnly) Label(item.hDC, WindowText(item.hwndItem), text, theme.Bold, color,
        (navigation ? DT_LEFT : DT_CENTER) | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    if (item.itemState & ODS_FOCUS) {
        RECT focus = rectangle; InflateRect(&focus, -theme.S(4), -theme.S(4)); DrawFocusRect(item.hDC, &focus);
    }
}
HWND MakeControl(HWND parent, HINSTANCE instance, const wchar_t* className, const std::wstring& text,
                 DWORD style, int id, HFONT font, DWORD extendedStyle) {
    HWND result = CreateWindowExW(extendedStyle, className, text.c_str(), WS_CHILD | WS_VISIBLE | style,
        0, 0, 0, 0, parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), instance, nullptr);
    if (!result) throw std::runtime_error("Khong tao duoc dieu khien giao dien");
    SendMessageW(result, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
    return result;
}

Form::Form(HINSTANCE instance, const Theme& theme, std::wstring title, std::vector<Field> fields,
           std::function<void(const Values&)> submit, bool readOnly)
    : Instance(instance), Style(theme), Fields(std::move(fields)), Submit(std::move(submit)), Title(std::move(title)), ReadOnly(readOnly) {}
void Form::SetProfileOptions(Options students, Options teachers) {
    StudentOptions = std::move(students); TeacherOptions = std::move(teachers);
}
void Form::SetChoices(std::size_t index) {
    auto& field = Fields[index]; HWND control = Inputs[index];
    SendMessageW(control, CB_RESETCONTENT, 0, 0);
    int selection = -1;
    for (std::size_t i = 0; i < field.Choices.size(); ++i) {
        SendMessageW(control, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(field.Choices[i].second.c_str()));
        if (field.Choices[i].first == field.Value) selection = static_cast<int>(i);
    }
    if (selection < 0 && !field.Choices.empty()) selection = 0;
    SendMessageW(control, CB_SETCURSEL, selection, 0);
}
void Form::UpdateProfileChoices(bool initial) {
    int roleIndex = -1, profileIndex = -1;
    for (std::size_t i = 0; i < Fields.size(); ++i) {
        if (Fields[i].Key == L"role") roleIndex = static_cast<int>(i);
        if (Fields[i].Key == L"profile") profileIndex = static_cast<int>(i);
    }
    if (roleIndex < 0 || profileIndex < 0) return;
    int selected = static_cast<int>(SendMessageW(Inputs[roleIndex], CB_GETCURSEL, 0, 0));
    const auto role = selected < 0 ? L"2" : Fields[roleIndex].Choices[selected].first;
    auto& profile = Fields[profileIndex];
    if (!initial) profile.Value.clear();
    profile.Choices = role == L"0" ? StudentOptions : role == L"1" ? TeacherOptions : Options{{L"", L"Không liên kết hồ sơ"}};
    SetChoices(profileIndex);
    EnableWindow(Inputs[profileIndex], role != L"2" && !ReadOnly && !profile.ReadOnly);
}
Values Form::Collect() const {
    Values result;
    for (std::size_t i = 0; i < Fields.size(); ++i) {
        const auto& field = Fields[i];
        if (field.Kind == FieldKind::Combo) {
            int index = static_cast<int>(SendMessageW(Inputs[i], CB_GETCURSEL, 0, 0));
            if (index < 0 || index >= static_cast<int>(field.Choices.size()))
                throw std::invalid_argument(Utf8(L"Chưa chọn: " + field.Label));
            result[field.Key] = field.Choices[index].first;
        } else result[field.Key] = WindowText(Inputs[i]);
    }
    return result;
}
void Form::Initialize() {
    SetWindowTextW(Window, Title.c_str());
    int width = Style.S(520), height = Style.S(136 + static_cast<int>(Fields.size()) * 68);
    RECT parent{}; GetWindowRect(GetParent(Window), &parent);
    RECT bounds{0, 0, width, height}; AdjustWindowRectEx(&bounds, GetWindowLongW(Window, GWL_STYLE), FALSE, 0);
    int outerWidth = bounds.right - bounds.left, outerHeight = bounds.bottom - bounds.top;
    HMONITOR monitor = MonitorFromWindow(GetParent(Window), MONITOR_DEFAULTTONEAREST);
    MONITORINFO info{sizeof(info)}; GetMonitorInfoW(monitor, &info);
    int x = std::clamp(parent.left + (parent.right - parent.left - outerWidth) / 2, info.rcWork.left, std::max(info.rcWork.left, info.rcWork.right - outerWidth));
    int y = std::clamp(parent.top + (parent.bottom - parent.top - outerHeight) / 2, info.rcWork.top, std::max(info.rcWork.top, info.rcWork.bottom - outerHeight));
    SetWindowPos(Window, nullptr, x, y, outerWidth, outerHeight, SWP_NOZORDER);
    for (std::size_t i = 0; i < Fields.size(); ++i) {
        auto& field = Fields[i]; int top = Style.S(20 + static_cast<int>(i) * 68);
        auto label = MakeControl(Window, Instance, L"STATIC", field.Label, SS_LEFT, 3000 + static_cast<int>(i), Style.Normal);
        MoveWindow(label, Style.S(24), top, width - Style.S(48), Style.S(22), TRUE);
        DWORD flags = WS_TABSTOP | (field.Kind == FieldKind::Combo ? CBS_DROPDOWNLIST | WS_VSCROLL : ES_AUTOHSCROLL | WS_BORDER);
        if (field.Kind == FieldKind::Password) flags |= ES_PASSWORD;
        if ((ReadOnly || field.ReadOnly) && field.Kind != FieldKind::Combo) flags |= ES_READONLY;
        HWND control = MakeControl(Window, Instance, field.Kind == FieldKind::Combo ? L"COMBOBOX" : L"EDIT", field.Value,
            flags, 2000 + static_cast<int>(i), Style.Normal);
        MoveWindow(control, Style.S(24), top + Style.S(25), width - Style.S(48), Style.S(field.Kind == FieldKind::Combo ? 220 : 31), TRUE);
        if (field.Kind != FieldKind::Combo) SendMessageW(control, EM_SETLIMITTEXT, 512, 0);
        Inputs.push_back(control);
        if (field.Kind == FieldKind::Combo) {
            SetChoices(i);
            EnableWindow(control, !ReadOnly && !field.ReadOnly);
        }
    }
    UpdateProfileChoices(true);
    ErrorLabel = MakeControl(Window, Instance, L"STATIC", L"", SS_LEFT, 3100, Style.Small);
    MoveWindow(ErrorLabel, Style.S(24), height - Style.S(92), width - Style.S(48), Style.S(38), TRUE);
    auto okay = MakeControl(Window, Instance, L"BUTTON", ReadOnly ? L"Đóng" : L"Lưu thay đổi", BS_OWNERDRAW | WS_TABSTOP, IDOK, Style.Bold);
    MoveWindow(okay, width - Style.S(166), height - Style.S(49), Style.S(142), Style.S(34), TRUE);
    if (!ReadOnly) {
        auto cancel = MakeControl(Window, Instance, L"BUTTON", L"Hủy", BS_OWNERDRAW | WS_TABSTOP, IDCANCEL, Style.Normal);
        MoveWindow(cancel, width - Style.S(270), height - Style.S(49), Style.S(92), Style.S(34), TRUE);
    }
    if (!Inputs.empty()) SetFocus(Inputs.front());
}
INT_PTR CALLBACK Form::Procedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    auto self = reinterpret_cast<Form*>(GetWindowLongPtrW(window, DWLP_USER));
    if (message == WM_INITDIALOG) {
        self = reinterpret_cast<Form*>(lParam); self->Window = window;
        SetWindowLongPtrW(window, DWLP_USER, reinterpret_cast<LONG_PTR>(self));
        try { self->Initialize(); }
        catch (const std::exception& error) { MessageBoxW(window, Wide(error.what()).c_str(), L"SRMS", MB_ICONERROR); EndDialog(window, IDCANCEL); }
        return FALSE;
    }
    if (!self) return FALSE;
    switch (message) {
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                try {
                    if (!self->ReadOnly && self->Submit) self->Submit(self->Collect());
                    EndDialog(window, IDOK);
                } catch (const std::exception& error) { SetWindowTextW(self->ErrorLabel, Wide(error.what()).c_str()); }
                return TRUE;
            }
            if (LOWORD(wParam) == IDCANCEL) { EndDialog(window, IDCANCEL); return TRUE; }
            if (HIWORD(wParam) == CBN_SELCHANGE) {
                int index = static_cast<int>(LOWORD(wParam)) - 2000;
                if (index >= 0 && index < static_cast<int>(self->Fields.size()) && self->Fields[index].Key == L"role")
                    self->UpdateProfileChoices();
            }
            return FALSE;
        case WM_CTLCOLORDLG: return reinterpret_cast<INT_PTR>(GetStockObject(WHITE_BRUSH));
        case WM_CTLCOLORSTATIC: {
            HDC dc = reinterpret_cast<HDC>(wParam); SetBkColor(dc, White);
            SetTextColor(dc, reinterpret_cast<HWND>(lParam) == self->ErrorLabel ? Red : Ink);
            return reinterpret_cast<INT_PTR>(GetStockObject(WHITE_BRUSH));
        }
        case WM_DRAWITEM:
            PaintButton(*reinterpret_cast<DRAWITEMSTRUCT*>(lParam), self->Style, wParam == IDOK);
            return TRUE;
        case WM_CLOSE: EndDialog(window, IDCANCEL); return TRUE;
    }
    return FALSE;
}
bool Form::Show(HWND parent) {
    // An empty dialog template; controls are created during WM_INITDIALOG.
    struct alignas(DWORD) Template { DLGTEMPLATE Dialog; WORD Menu, Class, Title; } layout{};
    layout.Dialog.style = WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME;
    layout.Dialog.dwExtendedStyle = WS_EX_CONTROLPARENT;
    return DialogBoxIndirectParamW(Instance, &layout.Dialog, parent, Procedure, reinterpret_cast<LPARAM>(this)) == IDOK;
}
}
