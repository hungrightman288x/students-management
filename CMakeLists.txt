cmake_minimum_required(VERSION 3.16)

project(StudentManagement LANGUAGES CXX)

add_executable(student_management
    src/main.cpp
    src/models/Date.cpp
)

target_compile_features(student_management PRIVATE cxx_std_17)

target_include_directories(student_management PRIVATE include)