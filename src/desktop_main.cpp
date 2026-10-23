#include "ui/DesktopControls.h"
#include "ui/DesktopApp.h"
#include <shellapi.h>
#include <filesystem>
#include <stdexcept>

namespace {
std::filesystem::path DefaultDataDirectory() {
    wchar_t filename[32768]{};
    DWORD length = GetModuleFileNameW(nullptr, filename, static_cast<DWORD>(std::size(filename)));
    if (!length || length >= std::size(filename)) throw std::runtime_error("Khong xac dinh duoc thu muc chuong trinh");
    auto directory = std::filesystem::path(filename).parent_path();
    auto candidate = directory;
    for (int i = 0; i < 5; ++i) {
        if (std::filesystem::is_directory(candidate / "data")) return candidate / "data";
        if (candidate == candidate.parent_path()) break;
        candidate = candidate.parent_path();
    }
    return directory / "data";
}
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand) {
    SetProcessDPIAware();
    try {
        auto directory = DefaultDataDirectory();
        int count = 0;
        auto arguments = CommandLineToArgvW(GetCommandLineW(), &count);
        if (!arguments) throw std::runtime_error("Khong doc duoc tham so dong lenh");
        struct ArgumentGuard { LPWSTR* Value; ~ArgumentGuard() { LocalFree(Value); } } guard{arguments};
        for (int i = 1; i < count; ++i) {
            std::wstring argument = arguments[i];
            if (argument == L"--data-dir" && i + 1 < count) directory = arguments[++i];
            else if (argument == L"--hidden") showCommand = SW_HIDE;
            else throw std::invalid_argument("Tham so khong hop le");
        }
        SchoolManager system(directory.u8string());
        return RunDesktopApplication(instance, showCommand, system);
    } catch (const std::exception& error) {
        MessageBoxW(nullptr, desktop::Wide(error.what()).c_str(), L"Không thể mở SRMS", MB_OK | MB_ICONERROR);
        return 1;
    }
}
