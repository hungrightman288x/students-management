#pragma once

#ifdef _WIN32
#include <windows.h>
#include "managers/SchoolManager.h"

// The console entry point remains available as student_management.exe.
int RunDesktopApplication(HINSTANCE instance, int showCommand, SchoolManager& system);
#endif
