param([switch]$Test, [switch]$GuiOnly, [switch]$ConsoleOnly, [string]$Compiler = "g++")
$ErrorActionPreference = "Stop"
$projectRoot = Split-Path -Parent $PSScriptRoot
Push-Location $projectRoot
try {
    New-Item -ItemType Directory -Path build -Force | Out-Null
    $coreSources = @(Get-ChildItem src/models,src/managers,src/storage -Filter *.cpp -File |
        ForEach-Object { $_.FullName })
    if ($GuiOnly -and $ConsoleOnly) { throw "Chi chon GuiOnly hoac ConsoleOnly." }
    if (!$GuiOnly) {
        & $Compiler -std=c++17 -Wall -Wextra -Wpedantic -I include @coreSources src/ui/Menu.cpp src/main.cpp -o build/student_management.exe
        if ($LASTEXITCODE -ne 0) { throw "Build chuong trinh that bai." }
        Write-Host "Da build: build/student_management.exe"
    }
    if (!$ConsoleOnly) {
        windres -I resources resources/desktop.rc -o build/desktop_resources.o
        if ($LASTEXITCODE -ne 0) { throw "Build Windows resources that bai." }
        & $Compiler -std=c++17 -Wall -Wextra -Wpedantic -DUNICODE -D_UNICODE -municode -mwindows -I include @coreSources src/ui/DesktopControls.cpp src/ui/DesktopApp.cpp src/desktop_main.cpp build/desktop_resources.o -lcomctl32 -luxtheme -lcomdlg32 -lgdi32 -luser32 -lshell32 -o build/student_management_gui.exe
        if ($LASTEXITCODE -ne 0) { throw "Build giao dien Windows that bai." }
        Write-Host "Da build: build/student_management_gui.exe"
    }
    if ($Test) {
        & $Compiler -std=c++17 -Wall -Wextra -Wpedantic -I include @coreSources tests/SchoolManagerTests.cpp -o build/student_management_tests.exe
        if ($LASTEXITCODE -ne 0) { throw "Build tests that bai." }
        & .\build\student_management_tests.exe
        if ($LASTEXITCODE -ne 0) { throw "Tests that bai." }
    }
} finally { Pop-Location }
