cmake_minimum_required(VERSION 3.22)

project(zCompiler
VERSION 0.1.0.0
DESCRIPTION "Compiler for UFABC Compilers' class"
LANGUAGES CXX )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(zCompiler src/main.cpp)