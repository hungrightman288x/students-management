$ErrorActionPreference = "Stop"
$projectRoot = Split-Path -Parent $PSScriptRoot
$executable = Join-Path $projectRoot 'build/student_management_gui.exe'
if (!(Test-Path -LiteralPath $executable)) {
    & (Join-Path $PSScriptRoot 'build.ps1') -GuiOnly
    if ($LASTEXITCODE -ne 0) { throw "Khong build duoc giao dien." }
}
Start-Process -FilePath $executable -WorkingDirectory $projectRoot -WindowStyle Normal
