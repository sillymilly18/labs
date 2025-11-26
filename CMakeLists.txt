cmake_minimum_required(VERSION 3.16)
project(crm_light C)

set(CMAKE_C_STANDARD 11)

# 📁 подключаем заголовочные файлы
include_directories(include)

# 📁 указываем все .c-файлы проекта
add_executable(crm_light
        src/main.c
        src/input.c
        src/repository/deal.c
        src/utils.c
        src/console.c
        include/utils.h
        src/repository/client.c
        src/repository/file.c
        include/file.h
        # ← новый модуль
)
