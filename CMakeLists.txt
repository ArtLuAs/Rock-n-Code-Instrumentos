cmake_minimum_required(VERSION 3.16)

project(RockNCodeInstrumentos VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Hints para encontrar o PostgreSQL instalado via MSYS2 (ucrt64 ou mingw64)
set(PostgreSQL_ROOT
    "C:/msys64/ucrt64"
    "C:/msys64/mingw64"
    "C:/msys64/mingw32"
)
set(PostgreSQL_INCLUDE_DIR
    "C:/msys64/ucrt64/include"
    CACHE PATH "Diretório de includes do PostgreSQL" FORCE
)
set(PostgreSQL_LIBRARY
    "C:/msys64/ucrt64/lib/libpq.dll.a"
    CACHE FILEPATH "Biblioteca libpq do PostgreSQL" FORCE
)

find_package(PostgreSQL REQUIRED)

add_executable(rock_n_code_instrumentos
    main.cpp
    src/instrumento.cpp
    src/cliente.cpp
    src/venda.cpp
    src/itemVenda.cpp
    src/lojaGerencia.cpp
    src/gerenciaInstrumentos.cpp
    src/gerenciaClientes.cpp
    src/gerenciaVendas.cpp
)

target_include_directories(rock_n_code_instrumentos
    PRIVATE
        headers
        ${PostgreSQL_INCLUDE_DIRS}
)

target_link_libraries(rock_n_code_instrumentos
    PRIVATE
        PostgreSQL::PostgreSQL
)
