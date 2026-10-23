cmake_minimum_required(VERSION 3.16)

project(StudentManagement LANGUAGES CXX)

add_library(student_core
    src/models/Account.cpp
    src/models/CourseSection.cpp
    src/models/Date.cpp
    src/models/Enrollment.cpp
    src/models/Grade.cpp
    src/models/Student.cpp
    src/models/Subject.cpp
    src/models/Teacher.cpp
    src/managers/AccountManager.cpp
    src/managers/CourseSectionManager.cpp
    src/managers/EnrollmentManager.cpp
    src/managers/StudentManager.cpp
    src/managers/SubjectManager.cpp
    src/managers/TeacherManager.cpp
    src/managers/SchoolManager.cpp
    src/storage/RecordIO.cpp
)

target_compile_features(student_core PUBLIC cxx_std_17)
target_include_directories(student_core PUBLIC include)
if(MSVC)
    target_compile_options(student_core PRIVATE /W4 /utf-8)
else()
    target_compile_options(student_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(student_management src/main.cpp src/ui/Menu.cpp)
target_link_libraries(student_management PRIVATE student_core)

if(WIN32)
    enable_language(RC)
    add_executable(student_management_gui WIN32
        src/desktop_main.cpp src/ui/DesktopApp.cpp src/ui/DesktopControls.cpp resources/desktop.rc)
    target_compile_definitions(student_management_gui PRIVATE UNICODE _UNICODE NOMINMAX)
    target_include_directories(student_management_gui PRIVATE resources)
    target_link_libraries(student_management_gui PRIVATE student_core comctl32 uxtheme comdlg32 gdi32 user32 shell32)
    if(MINGW)
        target_link_options(student_management_gui PRIVATE -municode)
    endif()
    if(MSVC)
        target_compile_options(student_management_gui PRIVATE /W4 /utf-8)
    else()
        target_compile_options(student_management_gui PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(student_management_tests tests/SchoolManagerTests.cpp)
    target_link_libraries(student_management_tests PRIVATE student_core)
    add_test(NAME student_management_tests COMMAND student_management_tests)
    find_package(Python3 COMPONENTS Interpreter QUIET)
    if(Python3_Interpreter_FOUND)
        add_test(NAME student_management_cli
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/cli_smoke.py
                    $<TARGET_FILE:student_management>)
    endif()
endif()
