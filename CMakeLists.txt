cmake_minimum_required(VERSION 3.13)
project(EsqueletoVM VERSION 0.1.0 LANGUAGES C)
set(CMAKE_C_STANDARD 90)
add_executable(${PROJECT_NAME} src/main.c src/RV32I/RV32I.c src/RV32I/RV32I.h)