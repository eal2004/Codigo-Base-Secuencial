cmake_minimum_required(VERSION 3.10)
project(ProyectoFinal)

set(CMAKE_CXX_STANDARD 17)

# 1. Declarar tu ejecutable primero
add_executable(ProyectoFinal main.cpp)

# 2. Buscar el paquete de multiprocesamiento
find_package(OpenMP REQUIRED)

# 3. Vincular OpenMP al ejecutable (¡El orden es estricto aquí!)
target_link_libraries(ProyectoFinal PRIVATE OpenMP::OpenMP_CXX)