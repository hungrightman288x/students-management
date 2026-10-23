param([switch]$Test, [string]$Compiler = "g++")
$ErrorActionPreference = "Stop"
$projectRoot = Split-Path -Parent $PSScriptRoot
Push-Location $projectRoot
try {
    New-Item -ItemType Directory -Path build -Force | Out-Null
    $coreSources = @(Get-ChildItem src/models,src/managers,src/storage -Filter *.cpp -File |
        ForEach-Object { $_.FullName })
    & $Compiler -std=c++17 -Wall -Wextra -Wpedantic -I include @coreSources src/ui/Menu.cpp src/main.cpp -o build/student_management.exe
    if ($LASTEXITCODE -ne 0) { throw "Build chuong trinh that bai." }
    Write-Host "Da build: build/student_management.exe"
    if ($Test) {
        & $Compiler -std=c++17 -Wall -Wextra -Wpedantic -I include @coreSources tests/SchoolManagerTests.cpp -o build/student_management_tests.exe
        if ($LASTEXITCODE -ne 0) { throw "Build tests that bai." }
        & .\build\student_management_tests.exe
        if ($LASTEXITCODE -ne 0) { throw "Tests that bai." }
    }
} finally { Pop-Location }
