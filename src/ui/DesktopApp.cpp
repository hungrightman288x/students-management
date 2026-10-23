#include "ui/DesktopControls.h"
#include "ui/DesktopApp.h"
#include <commdlg.h>
#include <uxtheme.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <memory>
#include <numeric>
#include <stdexcept>

namespace {
using namespace desktop;
constexpr int NavBase = 100, LogoutID = 190, CollapseID = 191, SaveID = 192;
constexpr int AddID = 200, EditID = 201, DeleteID = 202, ResetID = 203, ExportID = 204, DetailID = 205, HelpID = 206;
constexpr int SearchID = 210, FilterID = 211, TableID = 220, EmptyID = 221, NoticeID = 4000;
constexpr int UserID = 1000, PasswordID = 1001, OldPasswordID = 1010, NewPasswordID = 1011, ConfirmPasswordID = 1012;
enum class Page { Login, Results, Profile, Teaching, Grades, Students, Teachers, Subjects, Sections, Enrollments, Accounts };
struct Navigation { Page Target; std::wstring Text; int Icon; };
struct Row { std::string ID, SectionID; std::vector<std::wstring> Cells; float Total = 0; };

Field TextField(const wchar_t* key, const wchar_t* label, std::wstring value = L"", bool readOnly = false) {
    return {key, label, std::move(value), FieldKind::Text, {}, readOnly};
}
Field PasswordField(const wchar_t* key, const wchar_t* label) {
    return {key, label, L"", FieldKind::Password, {}, false};
}
Field ComboField(const wchar_t* key, const wchar_t* label, Options choices, std::wstring value = L"") {
    return {key, label, std::move(value), FieldKind::Combo, std::move(choices), false};
}
std::wstring Lower(std::wstring value) {
    if (!value.empty()) CharLowerBuffW(value.data(), static_cast<DWORD>(value.size()));
    return value;
}
std::string CSV(std::wstring value) {
    if (!value.empty() && (value.front() == L'=' || value.front() == L'+' || value.front() == L'-' || value.front() == L'@'))
        value.insert(value.begin(), L'\'');
    std::string result = "\"";
    for (char c : Utf8(value)) { if (c == '"') result += '"'; result += c; }
    return result + '"';
}

class DesktopApp {
    HINSTANCE Instance;
    HWND Window = nullptr, Table = nullptr, NoticeControl = nullptr, EmptyControl = nullptr, Tooltip = nullptr;
    SchoolManager& System;
    std::unique_ptr<Theme> Style;
    HBRUSH BackgroundBrush = CreateSolidBrush(Background);
    HIMAGELIST RowImages = nullptr;
    Page CurrentPage = Page::Login;
    bool Collapsed = false, Building = false;
    std::vector<HWND> Controls;
    std::vector<Navigation> NavigationItems;
    std::map<HWND, std::wstring> Tooltips;
    std::vector<Row> Rows;
    std::vector<int> VisibleRows, ColumnWeights;
    std::vector<std::wstring> Columns, FilterKeys;
    std::wstring Notice;
    bool NoticeError = false;
    RECT Content{}, TableBounds{}, ProfileBounds{}, LoginBounds{};
    int ClientWidth = 0, ClientHeight = 0;

    int S(int value) const { return Style ? Style->S(value) : value; }
    int SidebarWidth() const { return S(Collapsed ? 72 : 244); }
    HWND Control(int id) const { return GetDlgItem(Window, id); }
    void Place(int id, int x, int y, int width, int height) {
        if (auto control = Control(id)) MoveWindow(control, x, y, std::max(1, width), std::max(1, height), TRUE);
    }
    HWND AddControl(const wchar_t* type, const std::wstring& text, DWORD flags, int id) {
        HWND control = MakeControl(Window, Instance, type, text, flags, id, Style->Normal);
        Controls.push_back(control);
        if (std::wstring(type) == L"EDIT") SendMessageW(control, EM_SETLIMITTEXT, 512, 0);
        return control;
    }
    HWND Button(const std::wstring& text, int id) { return AddControl(L"BUTTON", text, BS_OWNERDRAW | WS_TABSTOP, id); }
    void Tip(HWND target, const std::wstring& text) {
        Tooltips[target] = text;
        TOOLINFOW tip{}; tip.cbSize = sizeof(tip); tip.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
        tip.hwnd = Window; tip.uId = reinterpret_cast<UINT_PTR>(target);
        tip.lpszText = Tooltips[target].data();
        SendMessageW(Tooltip, TTM_ADDTOOLW, 0, reinterpret_cast<LPARAM>(&tip));
    }
    void ShowNotice(const std::wstring& text, bool error = false) {
        Notice = text; NoticeError = error;
        if (NoticeControl) { SetWindowTextW(NoticeControl, text.c_str()); InvalidateRect(NoticeControl, nullptr, TRUE); }
        if (Control(SaveID)) EnableWindow(Control(SaveID), System.HasUnsavedChanges());
        SetWindowTextW(Window, System.HasUnsavedChanges() ? L"SRMS | Quản lý sinh viên — Chưa lưu" : L"SRMS | Quản lý sinh viên");
        InvalidateRect(Window, nullptr, FALSE);
    }
    std::wstring PageTitle() const {
        switch (CurrentPage) {
            case Page::Results: return L"Kết quả học tập";
            case Page::Profile: return L"Hồ sơ cá nhân";
            case Page::Teaching: return L"Lớp học phần phụ trách";
            case Page::Grades: return L"Bảng điểm lớp học phần";
            case Page::Students: return L"Quản lý sinh viên";
            case Page::Teachers: return L"Quản lý giáo viên";
            case Page::Subjects: return L"Quản lý môn học";
            case Page::Sections: return L"Quản lý lớp học phần";
            case Page::Enrollments: return L"Đăng ký học & kết quả";
            case Page::Accounts: return L"Quản lý tài khoản";
            default: return L"Đăng nhập";
        }
    }
    std::wstring Caption() const {
        if (CurrentPage == Page::Results) return L"Theo dõi các học phần đã đăng ký và kết quả học tập của bạn.";
        if (CurrentPage == Page::Profile) return L"Quản lý thông tin tài khoản của bạn.";
        if (CurrentPage == Page::Grades) return L"Nhập và cập nhật điểm cho các lớp bạn được phân công.";
        if (CurrentPage == Page::Teaching) return L"Chọn một lớp để xem danh sách sinh viên và bảng điểm.";
        if (CurrentPage == Page::Enrollments) return L"Quản lý đăng ký và ngày đăng ký. Giáo viên phụ trách cập nhật điểm.";
        return L"Tìm kiếm, thêm và cập nhật dữ liệu trong hệ thống.";
    }
    void Build();
    void Layout();
    void Paint(HDC dc);
    void DrawBrand(HDC dc, int x, int y);
    void DrawProfile(HDC dc);
    void DrawStudentSummary(HDC dc);
    void LoadRows();
    void FilterRows();
    void UpdateActions();
    std::string SelectedID() const;
    void Navigate(Page page);
    void Command(int id, int notification);
    void Login();
    void Logout();
    void ChangePassword();
    void Commit();
    void OpenEditor(bool adding);
    void OpenGrade();
    void ShowDetails();
    void DeleteSelected();
    void ResetPassword();
    void Export();
    static LRESULT CALLBACK Procedure(HWND, UINT, WPARAM, LPARAM);
    static LRESULT CALLBACK HeaderProcedure(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);
    LRESULT Notify(NMHDR* header);

public:
    DesktopApp(HINSTANCE instance, SchoolManager& system) : Instance(instance), System(system) {}
    ~DesktopApp() {
        if (RowImages) ImageList_Destroy(RowImages);
        DeleteObject(BackgroundBrush);
    }
    int Run(int showCommand);
};

void DesktopApp::Build() {
    Building = true;
    if (Tooltip) { DestroyWindow(Tooltip); Tooltip = nullptr; }
    for (auto control : Controls) DestroyWindow(control);
    Controls.clear(); Tooltips.clear(); Table = nullptr; EmptyControl = nullptr; NoticeControl = nullptr;
    if (RowImages) { ImageList_Destroy(RowImages); RowImages = nullptr; }
    Rows.clear(); VisibleRows.clear(); Columns.clear(); ColumnWeights.clear(); FilterKeys.clear();
    NavigationItems.clear();
    NoticeControl = AddControl(L"STATIC", Notice, SS_LEFT, NoticeID);
    if (CurrentPage == Page::Login) {
        AddControl(L"EDIT", L"", WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL, UserID);
        AddControl(L"EDIT", L"", WS_TABSTOP | WS_BORDER | ES_PASSWORD | ES_AUTOHSCROLL, PasswordID);
        Button(System.NeedsInitialAdmin() ? L"Tạo tài khoản quản trị" : L"Đăng nhập", IDOK);
    } else {
        const Role role = System.GetCurrentAccount()->GetUserRole();
        if (role == STUDENT) NavigationItems = {{Page::Results, L"Kết quả học tập", 0}, {Page::Profile, L"Hồ sơ cá nhân", 1}};
        if (role == TEACHER) NavigationItems = {{Page::Teaching, L"Lớp phụ trách", 2}, {Page::Grades, L"Bảng điểm", 0}, {Page::Profile, L"Hồ sơ cá nhân", 1}};
        if (role == ADMIN) NavigationItems = {
            {Page::Students, L"Sinh viên", 1}, {Page::Teachers, L"Giáo viên", 1}, {Page::Subjects, L"Môn học", 2},
            {Page::Sections, L"Lớp học phần", 2}, {Page::Enrollments, L"Đăng ký học", 0},
            {Page::Accounts, L"Tài khoản", 5}, {Page::Profile, L"Hồ sơ cá nhân", 1}};
        Tooltip = CreateWindowExW(WS_EX_TOPMOST, TOOLTIPS_CLASSW, nullptr, WS_POPUP | TTS_ALWAYSTIP,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, Window, nullptr, Instance, nullptr);
        for (std::size_t i = 0; i < NavigationItems.size(); ++i) {
            auto control = Button(NavigationItems[i].Text, NavBase + static_cast<int>(i));
            Tip(control, NavigationItems[i].Text);
        }
        Tip(Button(L"Thu gọn / mở rộng thanh bên", CollapseID), L"Thu gọn / mở rộng thanh bên");
        Button(L"Đăng xuất", LogoutID);
        Button(L"Lưu dữ liệu", SaveID);
        EnableWindow(Control(SaveID), System.HasUnsavedChanges());
        if (CurrentPage == Page::Profile) {
            for (int id : {OldPasswordID, NewPasswordID, ConfirmPasswordID})
                AddControl(L"EDIT", L"", WS_TABSTOP | WS_BORDER | ES_PASSWORD | ES_AUTOHSCROLL, id);
            Button(L"Lưu thay đổi", IDOK);
        } else {
            AddControl(L"EDIT", L"", WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL, SearchID);
            SendMessageW(Control(SearchID), EM_SETCUEBANNER, FALSE, reinterpret_cast<LPARAM>(L"Tìm kiếm theo mã hoặc tên…"));
            if (CurrentPage == Page::Results || CurrentPage == Page::Grades) {
                auto filter = AddControl(L"COMBOBOX", L"", WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL, FilterID);
                if (CurrentPage == Page::Results) {
                    for (auto label : {L"Tất cả học phần", L"Đạt (từ 5.00)", L"Chưa đạt (dưới 5.00)"})
                        SendMessageW(filter, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(label));
                } else {
                    SendMessageW(filter, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Tất cả lớp phụ trách"));
                    FilterKeys.push_back(L"");
                    for (const auto& section : System.GetTeachingSectionDetails()) {
                        FilterKeys.push_back(Wide(section.ID));
                        auto text = Wide(section.ID + " — " + section.SubjectName);
                        SendMessageW(filter, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(text.c_str()));
                    }
                }
                SendMessageW(filter, CB_SETCURSEL, 0, 0);
            }
            if (role == ADMIN) {
                Button(L"+  Thêm mới", AddID); Button(L"Sửa", EditID); Button(L"Xóa", DeleteID);
                if (CurrentPage == Page::Accounts) Button(L"Đặt lại mật khẩu", ResetID);
            } else if (CurrentPage == Page::Grades) Button(L"Nhập / sửa điểm", EditID);
            else if (CurrentPage == Page::Teaching) Button(L"Xem bảng điểm", DetailID);
            else Button(L"Chi tiết điểm", DetailID);
            Button(L"Xuất CSV", ExportID);
            if (CurrentPage == Page::Results) Button(L"Giải thích", HelpID);
            Table = AddControl(WC_LISTVIEWW, L"", WS_TABSTOP | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER | LVS_SHAREIMAGELISTS, TableID);
            ListView_SetExtendedListViewStyle(Table, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_GRIDLINES | LVS_EX_LABELTIP);
            ListView_SetBkColor(Table, White); ListView_SetTextBkColor(Table, White); ListView_SetTextColor(Table, Ink);
            SetWindowTheme(Table, L"Explorer", nullptr);
            RowImages = ImageList_Create(1, S(39), ILC_COLOR32, 1, 1);
            ListView_SetImageList(Table, RowImages, LVSIL_SMALL);
            SetWindowSubclass(ListView_GetHeader(Table), HeaderProcedure, 1, reinterpret_cast<DWORD_PTR>(this));
            EmptyControl = AddControl(L"STATIC", L"Chưa có dữ liệu phù hợp.\r\nThử thay đổi bộ lọc hoặc thêm dữ liệu.", SS_CENTER, EmptyID);
        }
    }
    Building = false;
    if (Table) LoadRows();
    Layout();
    InvalidateRect(Window, nullptr, TRUE);
    if (CurrentPage == Page::Login) SetFocus(Control(UserID));
}

void DesktopApp::Layout() {
    if (!Style) return;
    RECT client{}; GetClientRect(Window, &client); ClientWidth = client.right; ClientHeight = client.bottom;
    if (CurrentPage == Page::Login) {
        int width = S(440), height = S(464);
        LoginBounds = {(ClientWidth - width) / 2, std::max(S(20), (ClientHeight - height) / 2), 0, 0};
        LoginBounds.right = LoginBounds.left + width; LoginBounds.bottom = LoginBounds.top + height;
        int x = LoginBounds.left + S(36), y = LoginBounds.top;
        Place(UserID, x, y + S(190), width - S(72), S(38));
        Place(PasswordID, x, y + S(272), width - S(72), S(38));
        Place(IDOK, x, y + S(350), width - S(72), S(42));
        Place(NoticeID, x, y + S(406), width - S(72), S(42));
        return;
    }
    int sidebar = SidebarWidth(), margin = S(24);
    Content = {sidebar + margin, S(20), ClientWidth - margin, ClientHeight - S(24)};
    for (std::size_t i = 0; i < NavigationItems.size(); ++i)
        Place(NavBase + static_cast<int>(i), S(12), S(86 + static_cast<int>(i) * 44), sidebar - S(24), S(36));
    Place(CollapseID, sidebar - S(38), S(28), S(28), S(28));
    if (Collapsed) Place(CollapseID, S(22), S(61), S(28), S(24));
    Place(LogoutID, S(12), ClientHeight - S(49), sidebar - S(24), S(36));
    Place(SaveID, Content.right - S(120), S(25), S(120), S(32));
    Place(NoticeID, Content.left, S(77), Content.right - Content.left - S(10), S(25));
    if (CurrentPage == Page::Profile) {
        int totalWidth = std::min<int>(Content.right - Content.left, S(870));
        int left = Content.left + (Content.right - Content.left - totalWidth) / 2;
        ProfileBounds = {left, S(116), left + totalWidth, S(560)};
        int profileWidth = S(276), gap = S(20);
        int x = left + profileWidth + gap + S(28), width = totalWidth - profileWidth - gap - S(56);
        Place(OldPasswordID, x, S(213), width, S(36));
        Place(NewPasswordID, x, S(293), width, S(36));
        Place(ConfirmPasswordID, x, S(394), width, S(36));
        Place(IDOK, ProfileBounds.right - S(176), S(475), S(148), S(38));
        return;
    }
    int toolbar = CurrentPage == Page::Results ? S(328) : S(114);
    int right = Content.right;
    Place(ExportID, right - S(104), toolbar, S(104), S(34)); right -= S(114);
    if (Control(ResetID)) { Place(ResetID, right - S(148), toolbar, S(148), S(34)); right -= S(158); }
    if (Control(DeleteID)) { Place(DeleteID, right - S(64), toolbar, S(64), S(34)); right -= S(74); }
    if (Control(EditID)) {
        int width = S(CurrentPage == Page::Grades ? 148 : 64);
        Place(EditID, right - width, toolbar, width, S(34)); right -= width + S(10);
    }
    if (Control(AddID)) { Place(AddID, right - S(116), toolbar, S(116), S(34)); right -= S(126); }
    if (Control(DetailID)) { Place(DetailID, right - S(134), toolbar, S(134), S(34)); right -= S(144); }
    int searchWidth = std::min<int>(S(270), right - Content.left - S(Control(FilterID) ? 240 : 8));
    searchWidth = std::max(S(150), searchWidth);
    Place(SearchID, Content.left, toolbar, searchWidth, S(34));
    if (Control(FilterID)) Place(FilterID, Content.left + searchWidth + S(12), toolbar, std::max<int>(S(140), right - Content.left - searchWidth - S(24)), S(220));
    if (Control(HelpID)) Place(HelpID, Content.right - S(120), S(259), S(120), S(34));
    TableBounds = {Content.left, toolbar + S(52), Content.right, Content.bottom - S(30)};
    Place(TableID, TableBounds.left, TableBounds.top, TableBounds.right - TableBounds.left, TableBounds.bottom - TableBounds.top);
    Place(EmptyID, TableBounds.left + S(20), TableBounds.top + S(95), TableBounds.right - TableBounds.left - S(40), S(55));
    if (Table && !ColumnWeights.empty()) {
        RECT tableClient{}; GetClientRect(Table, &tableClient);
        int available = tableClient.right - S(2);
        const int sum = std::accumulate(ColumnWeights.begin(), ColumnWeights.end(), 0);
        int used = 0;
        for (std::size_t i = 0; i < ColumnWeights.size(); ++i) {
            int width = i + 1 == ColumnWeights.size() ? available - used : available * ColumnWeights[i] / sum;
            ListView_SetColumnWidth(Table, static_cast<int>(i), width); used += width;
        }
    }
}

void DesktopApp::DrawBrand(HDC dc, int x, int y) {
    RECT logo{x, y, x + S(34), y + S(38)};
    Card(dc, logo, Blue, S(8));
    Label(dc, L"S", logo, Style->Heading, White, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    if (!Collapsed || CurrentPage == Page::Login)
        Label(dc, L"SRMS", {x + S(46), y - S(1), x + S(160), y + S(38)}, Style->Brand, Blue);
}

void DesktopApp::DrawStudentSummary(HDC dc) {
    RECT box{Content.left, S(124), Content.right, S(242)};
    Card(dc, box, White, S(10));
    std::vector<int> weights{15, 35, 16, 16, 18};
    std::vector<std::wstring> names{L"MÃ SINH VIÊN", L"HỌ VÀ TÊN", L"SỐ HỌC PHẦN", L"TỔNG TÍN CHỈ", L"ĐIỂM TRUNG BÌNH"};
    const auto& student = System.GetOwnStudentProfile();
    auto results = System.GetAcademicResults();
    int credits = 0; float sum = 0;
    for (const auto& result : results) { credits += result.Credits; sum += result.Score.GetTotal() * result.Credits; }
    std::vector<std::wstring> values{Wide(student.GetID()), Wide(student.GetFullName()), std::to_wstring(results.size()),
        std::to_wstring(credits), credits ? Decimal(sum / credits) : L"—"};
    int x = box.left;
    for (std::size_t i = 0; i < weights.size(); ++i) {
        int width = (box.right - box.left) * weights[i] / 100;
        if (i + 1 == weights.size()) width = box.right - x;
        RECT header{x + 1, box.top + 1, x + width, box.top + S(36)};
        Fill(dc, header, i + 1 == weights.size() ? RGB(0, 96, 133) : Teal);
        Label(dc, names[i], header, Style->Small, White, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        Label(dc, values[i], {x + S(8), box.top + S(42), x + width - S(8), box.top + S(83)}, Style->Bold, Ink,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        x += width;
    }
    Label(dc, L"Điểm trung bình tính theo số tín chỉ của các học phần đã đăng ký.",
        {box.left + S(12), box.top + S(86), box.right - S(12), box.bottom - S(9)}, Style->Small, Muted,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
}

void DesktopApp::DrawProfile(HDC dc) {
    const auto* account = System.GetCurrentAccount();
    std::wstring name = Wide(account->GetID()), birth, email;
    if (account->GetUserRole() == STUDENT) {
        const auto& person = System.GetOwnStudentProfile();
        name = Wide(person.GetFullName()); birth = Wide(person.GetDateOfBirth().to_string()); email = Wide(person.GetEmail());
    } else if (account->GetUserRole() == TEACHER) {
        const auto& person = System.GetOwnTeacherProfile();
        name = Wide(person.GetFullName()); birth = Wide(person.GetDateOfBirth().to_string()); email = Wide(person.GetEmail());
    }
    int x = ProfileBounds.left, top = ProfileBounds.top, width = S(276);
    Card(dc, {x, top, x + width, top + S(346)}, White, S(10));
    int avatarX = x + (width - S(82)) / 2;
    Card(dc, {avatarX, top + S(25), avatarX + S(82), top + S(107)}, Background, S(82));
    DrawIcon(dc, {avatarX + S(20), top + S(43), avatarX + S(62), top + S(85)}, 1, RGB(150, 160, 177));
    Label(dc, name, {x + S(15), top + S(121), x + width - S(15), top + S(151)}, Style->Bold, Ink, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    Label(dc, RoleName(account->GetUserRole()), {x, top + S(157), x + width, top + S(180)}, Style->Small, Muted, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    Fill(dc, {x + S(22), top + S(199), x + width - S(22), top + S(200)}, Border);
    Label(dc, L"Tài khoản", {x + S(22), top + S(213), x + width - S(22), top + S(234)}, Style->Small, Muted);
    Label(dc, Wide(account->GetID()), {x + S(22), top + S(236), x + width - S(22), top + S(260)}, Style->Normal);
    std::wstring details = account->GetProfileID().empty() ? L"Quản trị hệ thống" : Wide(account->GetProfileID());
    if (!birth.empty()) details += L"  ·  " + birth;
    Label(dc, details, {x + S(22), top + S(270), x + width - S(22), top + S(293)}, Style->Small, Muted);
    Label(dc, email, {x + S(22), top + S(300), x + width - S(22), top + S(322)}, Style->Small, Muted);
    int formX = x + width + S(20);
    Card(dc, {formX, top, ProfileBounds.right, S(540)}, White, S(10));
    DrawIcon(dc, {formX + S(27), top + S(27), formX + S(48), top + S(48)}, 5, Muted);
    Label(dc, L"Đổi mật khẩu", {formX + S(57), top + S(22), ProfileBounds.right - S(20), top + S(55)}, Style->Bold);
    int labelX = formX + S(28);
    Label(dc, L"Mật khẩu cũ", {labelX, S(183), ProfileBounds.right - S(28), S(207)}, Style->Normal);
    Label(dc, L"Mật khẩu mới", {labelX, S(263), ProfileBounds.right - S(28), S(287)}, Style->Normal);
    Label(dc, L"Dùng mật khẩu riêng và giữ bí mật tài khoản.", {labelX, S(337), ProfileBounds.right - S(24), S(360)}, Style->Small, Muted);
    Label(dc, L"Xác nhận mật khẩu mới", {labelX, S(364), ProfileBounds.right - S(28), S(388)}, Style->Normal);
}

void DesktopApp::Paint(HDC dc) {
    RECT client{0, 0, ClientWidth, ClientHeight}; Fill(dc, client, Background);
    if (CurrentPage == Page::Login) {
        Card(dc, LoginBounds, White, S(14));
        int x = LoginBounds.left + S(36), y = LoginBounds.top;
        DrawBrand(dc, x, y + S(28));
        Label(dc, System.NeedsInitialAdmin() ? L"Thiết lập hệ thống" : L"Chào mừng trở lại", {x, y + S(91), LoginBounds.right - S(25), y + S(125)}, Style->Heading);
        Label(dc, System.NeedsInitialAdmin() ? L"Tạo tài khoản quản trị viên đầu tiên." : L"Đăng nhập để truy cập thông tin học tập.",
            {x, y + S(130), LoginBounds.right - S(25), y + S(154)}, Style->Small, Muted);
        Label(dc, L"Tên đăng nhập", {x, y + S(162), LoginBounds.right, y + S(186)}, Style->Normal);
        Label(dc, L"Mật khẩu", {x, y + S(244), LoginBounds.right, y + S(268)}, Style->Normal);
        Label(dc, L"HỆ THỐNG QUẢN LÝ SINH VIÊN", {0, LoginBounds.bottom + S(18), ClientWidth, LoginBounds.bottom + S(42)}, Style->Small, Muted,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        return;
    }
    int sidebar = SidebarWidth(); Fill(dc, {0, 0, sidebar, ClientHeight}, White);
    Fill(dc, {sidebar - 1, 0, sidebar, ClientHeight}, Border);
    DrawBrand(dc, S(Collapsed ? 19 : 18), S(24));
    Fill(dc, {0, ClientHeight - S(60), sidebar, ClientHeight - S(59)}, Border);
    if (!Collapsed) {
        auto* account = System.GetCurrentAccount();
        Label(dc, Wide(account->GetID()), {S(25), ClientHeight - S(118), sidebar - S(18), ClientHeight - S(94)}, Style->Bold);
        Label(dc, RoleName(account->GetUserRole()), {S(25), ClientHeight - S(91), sidebar - S(18), ClientHeight - S(72)}, Style->Small, Muted);
    }
    RECT heading{Content.left, S(22), Content.right - S(140), S(57)};
    Label(dc, PageTitle(), heading, Style->Heading);
    Label(dc, Caption(), {Content.left, S(59), Content.right, S(80)}, Style->Normal, Muted);
    if (CurrentPage == Page::Profile) DrawProfile(dc);
    else {
        if (CurrentPage == Page::Results) {
            Label(dc, L"Thông tin học tập", {Content.left, S(96), Content.right, S(121)}, Style->Bold);
            DrawStudentSummary(dc);
            Label(dc, L"Kết quả học tập học phần", {Content.left, S(261), Content.right - S(135), S(287)}, Style->Bold);
            Label(dc, L"Nhấp đúp vào một học phần để xem chi tiết điểm thành phần.", {Content.left, S(293), Content.right, S(315)}, Style->Small, Muted);
        }
        Label(dc, std::to_wstring(VisibleRows.size()) + L" bản ghi", {Content.left, Content.bottom - S(19), Content.right, Content.bottom + S(3)}, Style->Small, Muted);
        if (CurrentPage == Page::Grades)
            Label(dc, L"Tổng = 10% chuyên cần + 30% giữa kỳ + 60% cuối kỳ", {Content.left + S(115), Content.bottom - S(19), Content.right, Content.bottom + S(3)}, Style->Small, Muted);
    }
}

void DesktopApp::LoadRows() {
    Rows.clear();
    std::vector<std::wstring> columns;
    auto add = [&](const std::string& id, std::vector<std::wstring> cells, float total = 0, const std::string& section = "") {
        cells.insert(cells.begin(), std::to_wstring(Rows.size() + 1));
        Rows.push_back({id, section, std::move(cells), total});
    };
    switch (CurrentPage) {
        case Page::Results:
            columns = {L"STT", L"HỌC PHẦN", L"MÃ LHP", L"TÍN CHỈ", L"CHUYÊN CẦN", L"GIỮA KỲ", L"CUỐI KỲ", L"ĐIỂM TỔNG HỢP"};
            ColumnWeights = {5, 27, 15, 8, 12, 10, 10, 15};
            for (const auto& item : System.GetAcademicResults())
                add(item.EnrollmentID, {Wide(item.SubjectName), Wide(item.SectionID), std::to_wstring(item.Credits),
                    Decimal(item.Score.GetAttendance()), Decimal(item.Score.GetMidterm()), Decimal(item.Score.GetFinal()), Decimal(item.Score.GetTotal())}, item.Score.GetTotal(), item.SectionID);
            break;
        case Page::Grades:
        case Page::Enrollments:
            columns = {L"STT", L"MÃ ĐĂNG KÝ", L"SINH VIÊN", L"MÃ SV", L"HỌC PHẦN / LỚP", L"CC", L"GK", L"CK", L"TỔNG"};
            ColumnWeights = {5, 12, 20, 10, 25, 7, 7, 7, 8};
            for (const auto& item : System.GetAcademicResults())
                add(item.EnrollmentID, {Wide(item.EnrollmentID), Wide(item.StudentName), Wide(item.StudentID), Wide(item.SubjectName + " / " + item.SectionID),
                    Decimal(item.Score.GetAttendance()), Decimal(item.Score.GetMidterm()), Decimal(item.Score.GetFinal()), Decimal(item.Score.GetTotal())}, item.Score.GetTotal(), item.SectionID);
            break;
        case Page::Students:
            columns = {L"STT", L"MÃ SINH VIÊN", L"HỌ VÀ TÊN", L"NGÀY SINH", L"EMAIL"}; ColumnWeights = {6, 17, 30, 16, 31};
            for (auto item : System.GetStudents().GetAll()) add(item->GetID(), {Wide(item->GetID()), Wide(item->GetFullName()), Wide(item->GetDateOfBirth().to_string()), Wide(item->GetEmail())});
            break;
        case Page::Teachers:
            columns = {L"STT", L"MÃ GIÁO VIÊN", L"HỌ VÀ TÊN", L"NGÀY SINH", L"EMAIL"}; ColumnWeights = {6, 17, 30, 16, 31};
            for (auto item : System.GetTeachers().GetAll()) add(item->GetID(), {Wide(item->GetID()), Wide(item->GetFullName()), Wide(item->GetDateOfBirth().to_string()), Wide(item->GetEmail())});
            break;
        case Page::Subjects:
            columns = {L"STT", L"MÃ MÔN HỌC", L"TÊN MÔN HỌC", L"SỐ TÍN CHỈ"}; ColumnWeights = {6, 22, 52, 20};
            for (auto item : System.GetSubjects().GetAll()) add(item->GetID(), {Wide(item->GetID()), Wide(item->GetSubjectName()), std::to_wstring(item->GetCredits())});
            break;
        case Page::Sections:
            columns = {L"STT", L"MÃ LỚP", L"MÔN HỌC", L"GIÁO VIÊN", L"SĨ SỐ", L"TỐI ĐA"}; ColumnWeights = {6, 18, 33, 25, 9, 9};
            for (auto item : System.GetSections().GetAll()) add(item->GetID(), {Wide(item->GetID()), Wide(System.GetSubjects().FindByID(item->GetSubjectId())->GetSubjectName()),
                Wide(System.GetTeachers().FindByID(item->GetTeacherId())->GetFullName()), std::to_wstring(item->GetStudentNumber()), std::to_wstring(item->GetMaxStudents())});
            break;
        case Page::Teaching:
            columns = {L"STT", L"MÃ LỚP", L"MÃ MÔN", L"TÊN HỌC PHẦN", L"TÍN CHỈ", L"SĨ SỐ", L"TỐI ĐA"}; ColumnWeights = {6, 18, 14, 35, 9, 9, 9};
            for (const auto& item : System.GetTeachingSectionDetails()) add(item.ID, {Wide(item.ID), Wide(item.SubjectID), Wide(item.SubjectName), std::to_wstring(item.Credits), std::to_wstring(item.StudentNumber), std::to_wstring(item.MaxStudents)});
            break;
        case Page::Accounts:
            columns = {L"STT", L"TÊN ĐĂNG NHẬP", L"VAI TRÒ", L"MÃ HỒ SƠ"}; ColumnWeights = {6, 35, 29, 30};
            for (auto item : System.GetAccounts().GetAll()) add(item->GetID(), {Wide(item->GetID()), RoleName(item->GetUserRole()), Wide(item->GetProfileID())});
            break;
        default: return;
    }
    if (Columns != columns) {
        Columns = std::move(columns);
        while (Header_GetItemCount(ListView_GetHeader(Table)) > 0) ListView_DeleteColumn(Table, 0);
        for (std::size_t i = 0; i < Columns.size(); ++i) {
            LVCOLUMNW column{}; column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
            column.pszText = Columns[i].data(); column.cx = S(100); column.fmt = i == 0 ? LVCFMT_CENTER : LVCFMT_LEFT;
            ListView_InsertColumn(Table, static_cast<int>(i), &column);
        }
    }
    FilterRows();
}

void DesktopApp::FilterRows() {
    if (!Table) return;
    auto selected = SelectedID();
    auto query = Lower(WindowText(Control(SearchID)));
    int filter = Control(FilterID) ? static_cast<int>(SendMessageW(Control(FilterID), CB_GETCURSEL, 0, 0)) : 0;
    Building = true; SendMessageW(Table, WM_SETREDRAW, FALSE, 0); ListView_DeleteAllItems(Table); VisibleRows.clear();
    for (std::size_t index = 0; index < Rows.size(); ++index) {
        const auto& row = Rows[index];
        std::wstring text;
        for (const auto& cell : row.Cells) text += cell + L" ";
        if (!query.empty() && Lower(text).find(query) == std::wstring::npos) continue;
        if (CurrentPage == Page::Results && ((filter == 1 && row.Total < 5) || (filter == 2 && row.Total >= 5))) continue;
        if (CurrentPage == Page::Grades && filter > 0 && filter < static_cast<int>(FilterKeys.size()) && Wide(row.SectionID) != FilterKeys[filter]) continue;
        int position = static_cast<int>(VisibleRows.size()); VisibleRows.push_back(static_cast<int>(index));
        auto number = std::to_wstring(position + 1);
        LVITEMW item{}; item.mask = LVIF_TEXT | LVIF_PARAM; item.iItem = position;
        item.pszText = number.data(); item.lParam = static_cast<LPARAM>(index);
        ListView_InsertItem(Table, &item);
        for (std::size_t cell = 1; cell < row.Cells.size(); ++cell)
            ListView_SetItemText(Table, position, static_cast<int>(cell), const_cast<wchar_t*>(row.Cells[cell].c_str()));
    }
    int selection = VisibleRows.empty() ? -1 : 0;
    for (std::size_t i = 0; i < VisibleRows.size(); ++i) if (Rows[VisibleRows[i]].ID == selected) selection = static_cast<int>(i);
    if (selection >= 0) ListView_SetItemState(Table, selection, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    SendMessageW(Table, WM_SETREDRAW, TRUE, 0); InvalidateRect(Table, nullptr, TRUE);
    ShowWindow(EmptyControl, VisibleRows.empty() ? SW_SHOW : SW_HIDE);
    Building = false; UpdateActions(); Layout(); InvalidateRect(Window, nullptr, FALSE);
}

std::string DesktopApp::SelectedID() const {
    if (!Table) return {};
    int index = ListView_GetNextItem(Table, -1, LVNI_SELECTED);
    if (index < 0 || index >= static_cast<int>(VisibleRows.size())) return {};
    int row = VisibleRows[index];
    return row >= 0 && row < static_cast<int>(Rows.size()) ? Rows[row].ID : "";
}
void DesktopApp::UpdateActions() {
    bool selected = !SelectedID().empty();
    for (int id : {EditID, DeleteID, ResetID, DetailID}) if (Control(id)) EnableWindow(Control(id), selected);
    if (Control(ExportID)) EnableWindow(Control(ExportID), !VisibleRows.empty());
}
void DesktopApp::Navigate(Page page) {
    CurrentPage = page; Notice.clear(); NoticeError = false; Build();
}
void DesktopApp::Login() {
    auto username = Utf8(WindowText(Control(UserID))), password = Utf8(WindowText(Control(PasswordID)));
    ValidateText(username, "Ten dang nhap"); ValidateText(password, "Mat khau");
    if (System.NeedsInitialAdmin()) System.CreateInitialAdmin(username, password);
    if (System.HasUnsavedChanges()) System.Save();
    if (!System.Login(username, password)) {
        SetWindowTextW(Control(PasswordID), L""); SetFocus(Control(PasswordID));
        throw std::invalid_argument("Sai ten dang nhap hoac mat khau");
    }
    Role role = System.GetCurrentAccount()->GetUserRole();
    Navigate(role == STUDENT ? Page::Results : role == TEACHER ? Page::Teaching : Page::Students);
}
void DesktopApp::Logout() {
    if (System.HasUnsavedChanges()) System.Save();
    System.Logout(); Collapsed = false; Navigate(Page::Login);
}
void DesktopApp::Commit() {
    try {
        System.Save();
        if (Table) LoadRows();
        ShowNotice(L"Đã lưu thay đổi thành công.");
    } catch (const std::exception& error) {
        if (Table) LoadRows();
        ShowNotice(L"Chưa lưu được. Dùng nút Lưu dữ liệu để thử lại. " + Wide(error.what()), true);
    }
}
void DesktopApp::ChangePassword() {
    auto password = WindowText(Control(NewPasswordID));
    if (password != WindowText(Control(ConfirmPasswordID))) throw std::invalid_argument("Hai mat khau moi khong khop");
    System.ChangeOwnPassword(Utf8(WindowText(Control(OldPasswordID))), Utf8(password));
    for (int id : {OldPasswordID, NewPasswordID, ConfirmPasswordID}) SetWindowTextW(Control(id), L"");
    Commit();
}

void DesktopApp::OpenEditor(bool adding) {
    if (System.GetCurrentAccount()->GetUserRole() != ADMIN) { OpenGrade(); return; }
    const auto id = adding ? "" : SelectedID();
    if (!adding && id.empty()) return;
    std::vector<Field> fields;
    std::function<void(const Values&)> submit;
    auto string = [](const Values& values, const wchar_t* key) { return Utf8(values.at(key)); };
    if (CurrentPage == Page::Students || CurrentPage == Page::Teachers) {
        std::wstring name, date = L"01/01/2005", email;
        if (!adding && CurrentPage == Page::Students) {
            auto item = System.GetStudents().FindByID(id); name = Wide(item->GetFullName()); date = Wide(item->GetDateOfBirth().to_string()); email = Wide(item->GetEmail());
        } else if (!adding) {
            auto item = System.GetTeachers().FindByID(id); name = Wide(item->GetFullName()); date = Wide(item->GetDateOfBirth().to_string()); email = Wide(item->GetEmail());
        }
        fields = {TextField(L"id", CurrentPage == Page::Students ? L"Mã sinh viên" : L"Mã giáo viên", Wide(id), !adding),
            TextField(L"name", L"Họ và tên", name), TextField(L"date", L"Ngày sinh (dd/mm/yyyy)", date), TextField(L"email", L"Email (có thể để trống)", email)};
        submit = [&, string](const Values& values) {
            auto birth = ReadDate(values.at(L"date"));
            if (CurrentPage == Page::Students) {
                Student item(string(values, L"id"), string(values, L"name"), birth.getDay(), birth.getMonth(), birth.getYear(), string(values, L"email"));
                if (adding) System.AddStudent(item); else System.UpdateStudent(item);
            } else {
                Teacher item(string(values, L"id"), string(values, L"name"), birth.getDay(), birth.getMonth(), birth.getYear(), string(values, L"email"));
                if (adding) System.AddTeacher(item); else System.UpdateTeacher(item);
            }
        };
    } else if (CurrentPage == Page::Subjects) {
        auto item = adding ? nullptr : System.GetSubjects().FindByID(id);
        fields = {TextField(L"id", L"Mã môn học", Wide(id), !adding), TextField(L"name", L"Tên môn học", item ? Wide(item->GetSubjectName()) : L""),
            TextField(L"credits", L"Số tín chỉ", item ? std::to_wstring(item->GetCredits()) : L"3")};
        submit = [&, string](const Values& values) {
            Subject subject(string(values, L"id"), string(values, L"name"), ReadInteger(values.at(L"credits")));
            if (adding) System.AddSubject(subject); else System.UpdateSubject(subject);
        };
    } else if (CurrentPage == Page::Sections) {
        Options subjects, teachers;
        for (auto item : System.GetSubjects().GetAll()) subjects.push_back({Wide(item->GetID()), Wide(item->GetID() + " — " + item->GetSubjectName())});
        for (auto item : System.GetTeachers().GetAll()) teachers.push_back({Wide(item->GetID()), Wide(item->GetID() + " — " + item->GetFullName())});
        auto item = adding ? nullptr : System.GetSections().FindByID(id);
        fields = {TextField(L"id", L"Mã lớp học phần", Wide(id), !adding),
            ComboField(L"subject", L"Môn học", subjects, item ? Wide(item->GetSubjectId()) : L""),
            ComboField(L"teacher", L"Giáo viên phụ trách", teachers, item ? Wide(item->GetTeacherId()) : L""),
            TextField(L"maximum", L"Sĩ số tối đa", item ? std::to_wstring(item->GetMaxStudents()) : L"40")};
        submit = [&, string](const Values& values) {
            int maximum = ReadInteger(values.at(L"maximum"));
            if (adding) System.AddSection(string(values, L"id"), string(values, L"subject"), string(values, L"teacher"), maximum);
            else System.UpdateSection(string(values, L"id"), string(values, L"subject"), string(values, L"teacher"), maximum);
        };
    } else if (CurrentPage == Page::Enrollments) {
        Options students, sections;
        for (auto item : System.GetStudents().GetAll()) students.push_back({Wide(item->GetID()), Wide(item->GetID() + " — " + item->GetFullName())});
        for (auto item : System.GetSections().GetAll()) sections.push_back({Wide(item->GetID()), Wide(item->GetID()) + L"  (" + std::to_wstring(item->GetStudentNumber()) + L"/" + std::to_wstring(item->GetMaxStudents()) + L")"});
        auto item = adding ? nullptr : System.GetEnrollments().FindByID(id);
        SYSTEMTIME now{}; GetLocalTime(&now);
        fields = {TextField(L"id", L"Mã đăng ký", Wide(id), !adding),
            ComboField(L"student", L"Sinh viên", students, item ? Wide(item->GetStudentId()) : L""),
            ComboField(L"section", L"Lớp học phần", sections, item ? Wide(item->GetCourseSectionId()) : L""),
            TextField(L"date", L"Ngày đăng ký (dd/mm/yyyy)", item ? Wide(item->GetEnrollmentDate().to_string()) : Wide(Date(now.wDay, now.wMonth, now.wYear).to_string()))};
        fields[1].ReadOnly = !adding; fields[2].ReadOnly = !adding;
        submit = [&, string](const Values& values) {
            auto date = ReadDate(values.at(L"date"));
            if (adding) System.Register(string(values, L"id"), string(values, L"student"), string(values, L"section"), date);
            else System.UpdateRegistrationDate(string(values, L"id"), date);
        };
    } else if (CurrentPage == Page::Accounts) {
        auto item = adding ? nullptr : System.GetAccounts().FindByID(id);
        fields.push_back(TextField(L"id", L"Tên đăng nhập", Wide(id), !adding));
        if (adding) fields.push_back(PasswordField(L"password", L"Mật khẩu"));
        fields.push_back(ComboField(L"role", L"Vai trò", {{L"0", L"Sinh viên"}, {L"1", L"Giáo viên"}, {L"2", L"Quản trị viên"}},
            std::to_wstring(item ? static_cast<int>(item->GetUserRole()) : 0)));
        fields.push_back(ComboField(L"profile", L"Hồ sơ liên kết", {}, item ? Wide(item->GetProfileID()) : L""));
        submit = [&, string](const Values& values) {
            Role role = static_cast<Role>(ReadInteger(values.at(L"role")));
            if (adding) System.AddAccount(Account(string(values, L"id"), string(values, L"password"), role, string(values, L"profile")));
            else System.UpdateAccountProfile(string(values, L"id"), role, string(values, L"profile"));
        };
    } else return;
    Form form(Instance, *Style, (adding ? L"Thêm mới · " : L"Chỉnh sửa · ") + PageTitle(), std::move(fields), std::move(submit));
    if (CurrentPage == Page::Accounts) {
        Options students, teachers;
        for (auto item : System.GetStudents().GetAll()) students.push_back({Wide(item->GetID()), Wide(item->GetID() + " — " + item->GetFullName())});
        for (auto item : System.GetTeachers().GetAll()) teachers.push_back({Wide(item->GetID()), Wide(item->GetID() + " — " + item->GetFullName())});
        form.SetProfileOptions(std::move(students), std::move(teachers));
    }
    if (form.Show(Window)) Commit();
}

void DesktopApp::OpenGrade() {
    const auto id = SelectedID(); if (id.empty()) return;
    auto results = System.GetAcademicResults();
    auto found = std::find_if(results.begin(), results.end(), [&](const AcademicResult& item) { return item.EnrollmentID == id; });
    if (found == results.end()) return;
    Form form(Instance, *Style, L"Nhập / sửa điểm", {
        TextField(L"student", L"Sinh viên", Wide(found->StudentID + " — " + found->StudentName), true),
        TextField(L"section", L"Học phần / lớp", Wide(found->SubjectName + " / " + found->SectionID), true),
        TextField(L"attendance", L"Chuyên cần · 10% (0–10)", Decimal(found->Score.GetAttendance())),
        TextField(L"midterm", L"Giữa kỳ · 30% (0–10)", Decimal(found->Score.GetMidterm())),
        TextField(L"final", L"Cuối kỳ · 60% (0–10)", Decimal(found->Score.GetFinal()))},
        [&](const Values& values) { System.SetGrade(id, Grade(ReadScore(values.at(L"attendance")), ReadScore(values.at(L"midterm")), ReadScore(values.at(L"final")))); });
    if (form.Show(Window)) Commit();
}

void DesktopApp::ShowDetails() {
    const auto id = SelectedID(); if (id.empty()) return;
    if (CurrentPage == Page::Teaching) {
        Navigate(Page::Grades);
        auto key = Wide(id);
        auto found = std::find(FilterKeys.begin(), FilterKeys.end(), key);
        if (found != FilterKeys.end()) SendMessageW(Control(FilterID), CB_SETCURSEL, std::distance(FilterKeys.begin(), found), 0);
        FilterRows(); return;
    }
    auto results = System.GetAcademicResults();
    auto found = std::find_if(results.begin(), results.end(), [&](const AcademicResult& item) { return item.EnrollmentID == id; });
    if (found == results.end()) return;
    Form form(Instance, *Style, L"Chi tiết kết quả học tập", {
        TextField(L"subject", L"Học phần", Wide(found->SubjectName)), TextField(L"section", L"Mã lớp / mã đăng ký", Wide(found->SectionID + " / " + found->EnrollmentID)),
        TextField(L"teacher", L"Giáo viên phụ trách", Wide(found->TeacherName)),
        TextField(L"attendance", L"Chuyên cần · 10%", Decimal(found->Score.GetAttendance())),
        TextField(L"midterm", L"Giữa kỳ · 30%", Decimal(found->Score.GetMidterm())),
        TextField(L"final", L"Cuối kỳ · 60%", Decimal(found->Score.GetFinal())),
        TextField(L"total", L"Điểm tổng hợp", Decimal(found->Score.GetTotal()))}, {}, true);
    form.Show(Window);
}
void DesktopApp::DeleteSelected() {
    auto id = SelectedID(); if (id.empty()) return;
    auto text = L"Bạn có chắc muốn xóa bản ghi “" + Wide(id) + L"”?\nThao tác chỉ thực hiện khi không còn liên kết dữ liệu.";
    if (MessageBoxW(Window, text.c_str(), L"Xác nhận xóa", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) != IDYES) return;
    switch (CurrentPage) {
        case Page::Students: System.RemoveStudent(id); break;
        case Page::Teachers: System.RemoveTeacher(id); break;
        case Page::Subjects: System.RemoveSubject(id); break;
        case Page::Sections: System.RemoveSection(id); break;
        case Page::Enrollments: System.CancelRegistration(id); break;
        case Page::Accounts: System.RemoveAccount(id); break;
        default: return;
    }
    Commit();
}
void DesktopApp::ResetPassword() {
    auto id = SelectedID(); if (id.empty()) return;
    Form form(Instance, *Style, L"Đặt lại mật khẩu · " + Wide(id), {PasswordField(L"password", L"Mật khẩu mới"), PasswordField(L"confirm", L"Xác nhận mật khẩu mới")},
        [&](const Values& values) {
            if (values.at(L"password") != values.at(L"confirm")) throw std::invalid_argument("Hai mat khau moi khong khop");
            System.ResetPassword(id, Utf8(values.at(L"password")));
        });
    if (form.Show(Window)) Commit();
}
void DesktopApp::Export() {
    wchar_t path[32768] = L"ket-qua-hoc-tap.csv";
    OPENFILENAMEW dialog{}; dialog.lStructSize = sizeof(dialog); dialog.hwndOwner = Window;
    dialog.lpstrFilter = L"CSV UTF-8 (*.csv)\0*.csv\0"; dialog.lpstrFile = path;
    dialog.nMaxFile = static_cast<DWORD>(std::size(path)); dialog.lpstrDefExt = L"csv";
    dialog.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    if (!GetSaveFileNameW(&dialog)) return;
    std::ofstream output(std::filesystem::path(path), std::ios::binary);
    output << "\xEF\xBB\xBF";
    auto line = [&](const std::vector<std::wstring>& cells) {
        for (std::size_t i = 0; i < cells.size(); ++i) { if (i) output << ','; output << CSV(cells[i]); }
        output << "\r\n";
    };
    line(Columns);
    for (std::size_t i = 0; i < VisibleRows.size(); ++i) {
        auto cells = Rows[VisibleRows[i]].Cells; cells[0] = std::to_wstring(i + 1); line(cells);
    }
    output.close();
    if (!output) throw std::runtime_error("Khong ghi duoc file CSV");
    ShowNotice(L"Đã xuất " + std::to_wstring(VisibleRows.size()) + L" bản ghi ra CSV.");
}

void DesktopApp::Command(int id, int notification) {
    if (Building) return;
    if (id == IDOK) { if (CurrentPage == Page::Login) Login(); else if (CurrentPage == Page::Profile) ChangePassword(); return; }
    if (id >= NavBase && id < NavBase + static_cast<int>(NavigationItems.size())) { Navigate(NavigationItems[id - NavBase].Target); return; }
    if (id == CollapseID) { Collapsed = !Collapsed; Layout(); InvalidateRect(Window, nullptr, TRUE); return; }
    if (id == LogoutID) { Logout(); return; }
    if (id == SaveID) { Commit(); return; }
    if (id == SearchID && notification == EN_CHANGE) { FilterRows(); return; }
    if (id == FilterID && notification == CBN_SELCHANGE) { FilterRows(); return; }
    if (id == AddID) OpenEditor(true);
    if (id == EditID) OpenEditor(false);
    if (id == DeleteID) DeleteSelected();
    if (id == ResetID) ResetPassword();
    if (id == DetailID) ShowDetails();
    if (id == ExportID) Export();
    if (id == HelpID) MessageBoxW(Window,
        L"CC: Điểm chuyên cần (10%).\nGK: Điểm giữa kỳ (30%).\nCK: Điểm cuối kỳ (60%).\n\nTổng = CC × 0,1 + GK × 0,3 + CK × 0,6.\nĐiểm trung bình được tính theo số tín chỉ.\n\nĐiểm khởi tạo là 0. Hệ thống hiện chưa phân biệt điểm 0 với chưa nhập điểm.\nChọn hoặc nhấp đúp một học phần để xem chi tiết.",
        L"Giải thích kết quả học tập", MB_OK | MB_ICONINFORMATION);
}

LRESULT DesktopApp::Notify(NMHDR* header) {
    if (header->hwndFrom != Table) return 0;
    if (header->code == LVN_ITEMCHANGED && !Building) UpdateActions();
    if (header->code == NM_DBLCLK && !SelectedID().empty()) {
        if (CurrentPage == Page::Results || CurrentPage == Page::Teaching) ShowDetails();
        else OpenEditor(false);
    }
    if (header->code == NM_CUSTOMDRAW) {
        auto draw = reinterpret_cast<NMLVCUSTOMDRAW*>(header);
        if (draw->nmcd.dwDrawStage == CDDS_PREPAINT) return CDRF_NOTIFYITEMDRAW;
        if (draw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT) return CDRF_NOTIFYSUBITEMDRAW;
        if (draw->nmcd.dwDrawStage == (CDDS_ITEMPREPAINT | CDDS_SUBITEM)) {
            int index = static_cast<int>(draw->nmcd.dwItemSpec);
            bool selected = (ListView_GetItemState(Table, index, LVIS_SELECTED) & LVIS_SELECTED) != 0;
            draw->clrTextBk = selected ? RGB(233, 242, 255) : index % 2 ? RGB(248, 250, 252) : White;
            bool score = (CurrentPage == Page::Results && draw->iSubItem >= 4) ||
                ((CurrentPage == Page::Grades || CurrentPage == Page::Enrollments) && draw->iSubItem >= 5);
            bool total = score && draw->iSubItem + 1 == static_cast<int>(Columns.size());
            draw->clrText = score ? (total ? Ink : Blue) : draw->iSubItem == 0 ? Muted : Ink;
            if (total && !selected) draw->clrTextBk = RGB(242, 247, 254);
            SelectObject(draw->nmcd.hdc, total ? Style->Bold : Style->Normal);
            return CDRF_NEWFONT;
        }
    }
    return 0;
}

LRESULT CALLBACK DesktopApp::HeaderProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR data) {
    auto* self = reinterpret_cast<DesktopApp*>(data);
    if (message == HDM_LAYOUT) {
        LRESULT result = DefSubclassProc(window, message, wParam, lParam);
        auto* layout = reinterpret_cast<HDLAYOUT*>(lParam);
        int difference = self->S(38) - layout->pwpos->cy;
        layout->pwpos->cy = self->S(38); layout->prc->top += difference;
        return result;
    }
    if (message == WM_ERASEBKGND) return 1;
    if (message == WM_PAINT || message == WM_PRINTCLIENT) {
        PAINTSTRUCT paint{}; HDC dc = message == WM_PAINT ? BeginPaint(window, &paint) : reinterpret_cast<HDC>(wParam);
        RECT client{}; GetClientRect(window, &client); Fill(dc, client, Teal);
        for (int i = 0; i < Header_GetItemCount(window); ++i) {
            RECT rectangle{}; Header_GetItemRect(window, i, &rectangle);
            if (i + 1 == Header_GetItemCount(window)) Fill(dc, rectangle, RGB(0, 96, 133));
            if (i > 0) Fill(dc, {rectangle.left, rectangle.top, rectangle.left + 1, rectangle.bottom}, RGB(91, 166, 192));
            rectangle.left += self->S(8); rectangle.right -= self->S(6);
            if (i < static_cast<int>(self->Columns.size())) Label(dc, self->Columns[i], rectangle, self->Style->Small, White,
                DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        }
        if (message == WM_PAINT) EndPaint(window, &paint);
        return 0;
    }
    return DefSubclassProc(window, message, wParam, lParam);
}

LRESULT CALLBACK DesktopApp::Procedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    auto* self = reinterpret_cast<DesktopApp*>(GetWindowLongPtrW(window, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
        self = reinterpret_cast<DesktopApp*>(reinterpret_cast<CREATESTRUCTW*>(lParam)->lpCreateParams);
        self->Window = window; SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    if (!self) return DefWindowProcW(window, message, wParam, lParam);
    try {
        switch (message) {
            case WM_CREATE:
                self->Style = std::make_unique<Theme>(window); self->Build(); return 0;
            case WM_SIZE: self->Layout(); InvalidateRect(window, nullptr, TRUE); return 0;
            case WM_GETMINMAXINFO: {
                auto info = reinterpret_cast<MINMAXINFO*>(lParam); info->ptMinTrackSize = {self->S(1040), self->S(720)}; return 0;
            }
            case WM_COMMAND: self->Command(LOWORD(wParam), HIWORD(wParam)); return 0;
            case DM_GETDEFID:
                return self->Control(IDOK) ? MAKELRESULT(IDOK, DC_HASDEFID) : 0;
            case DM_SETDEFID: return 0;
            case WM_NOTIFY: return self->Notify(reinterpret_cast<NMHDR*>(lParam));
            case WM_DRAWITEM: {
                auto* item = reinterpret_cast<DRAWITEMSTRUCT*>(lParam); int id = static_cast<int>(wParam);
                bool nav = id >= NavBase && id < NavBase + static_cast<int>(self->NavigationItems.size());
                bool selected = nav && self->NavigationItems[id - NavBase].Target == self->CurrentPage;
                int icon = nav ? self->NavigationItems[id - NavBase].Icon : id == LogoutID ? 3 : id == CollapseID ? 4 : -1;
                PaintButton(*item, *self->Style, id == IDOK || id == AddID || (id == EditID && self->CurrentPage == Page::Grades),
                    id == DeleteID || id == LogoutID, nav || id == LogoutID || id == CollapseID, selected,
                    id == CollapseID || ((nav || id == LogoutID) && self->Collapsed), icon);
                return TRUE;
            }
            case WM_CTLCOLORSTATIC: {
                HDC dc = reinterpret_cast<HDC>(wParam); HWND control = reinterpret_cast<HWND>(lParam);
                SetTextColor(dc, control == self->NoticeControl ? self->NoticeError ? Red : RGB(32, 138, 99) : Muted);
                COLORREF color = control == self->EmptyControl || self->CurrentPage == Page::Login ? White : Background;
                SetBkColor(dc, color);
                return reinterpret_cast<LRESULT>(color == White ? GetStockObject(WHITE_BRUSH) : self->BackgroundBrush);
            }
            case WM_ERASEBKGND: return 1;
            case WM_PRINTCLIENT: self->Paint(reinterpret_cast<HDC>(wParam)); return 0;
            case WM_PAINT: {
                PAINTSTRUCT paint{}; HDC dc = BeginPaint(window, &paint);
                HDC buffer = CreateCompatibleDC(dc); HBITMAP bitmap = CreateCompatibleBitmap(dc, std::max(1, self->ClientWidth), std::max(1, self->ClientHeight));
                auto old = SelectObject(buffer, bitmap); self->Paint(buffer);
                BitBlt(dc, 0, 0, self->ClientWidth, self->ClientHeight, buffer, 0, 0, SRCCOPY);
                SelectObject(buffer, old); DeleteObject(bitmap); DeleteDC(buffer); EndPaint(window, &paint); return 0;
            }
            case WM_CLOSE:
                if (self->System.HasUnsavedChanges()) self->System.Save();
                DestroyWindow(window); return 0;
            case WM_DESTROY: PostQuitMessage(0); return 0;
        }
    } catch (const std::exception& error) {
        if (message == WM_CREATE) return -1;
        self->ShowNotice(Wide(error.what()), true);
    }
    return DefWindowProcW(window, message, wParam, lParam);
}

int DesktopApp::Run(int showCommand) {
    INITCOMMONCONTROLSEX controls{sizeof(controls), ICC_LISTVIEW_CLASSES | ICC_STANDARD_CLASSES}; InitCommonControlsEx(&controls);
    WNDCLASSEXW type{}; type.cbSize = sizeof(type); type.hInstance = Instance;
    type.lpfnWndProc = Procedure; type.lpszClassName = L"SRMSDesktopWindow";
    type.hCursor = LoadCursorW(nullptr, IDC_ARROW); type.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    type.style = CS_HREDRAW | CS_VREDRAW;
    if (!RegisterClassExW(&type) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) throw std::runtime_error("Khong dang ky duoc cua so Windows");
    RECT work{}; SystemParametersInfoW(SPI_GETWORKAREA, 0, &work, 0);
    int width = std::min(1380, static_cast<int>(work.right - work.left - 30));
    int height = std::min(880, static_cast<int>(work.bottom - work.top - 30));
    HWND window = CreateWindowExW(WS_EX_CONTROLPARENT, type.lpszClassName, L"SRMS | Quản lý sinh viên", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        work.left + (work.right - work.left - width) / 2, work.top + (work.bottom - work.top - height) / 2,
        width, height, nullptr, nullptr, Instance, this);
    if (!window) throw std::runtime_error("Khong khoi tao duoc giao dien Windows");
    ShowWindow(window, showCommand); UpdateWindow(window);
    MSG message{}; int result;
    while ((result = GetMessageW(&message, nullptr, 0, 0)) > 0) {
        if (!IsDialogMessageW(window, &message)) { TranslateMessage(&message); DispatchMessageW(&message); }
    }
    return result < 0 ? 1 : static_cast<int>(message.wParam);
}
}

int RunDesktopApplication(HINSTANCE instance, int showCommand, SchoolManager& system) {
    DesktopApp application(instance, system);
    return application.Run(showCommand);
}
