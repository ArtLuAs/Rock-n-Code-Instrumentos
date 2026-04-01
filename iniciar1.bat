@echo off
echo ==================================================
echo      Iniciando o sistema Rock 'n' Code...
echo ==================================================

:: 1. Iniciar o Backend (Compila com g++ e executa)
echo [1/2] Iniciando o Backend em C++...
start "Backend - API" cmd /k "cd backend && g++ src/*.cpp main.cpp -o loja.exe -D_WIN32_WINNT=0x0A00 -lws2_32 -lpq && loja.exe"

:: 2. Iniciar o Frontend (Instala dependencias e roda o Vite)
echo [2/2] Iniciando o Frontend em React...
start "Frontend - Vite" cmd /k "cd frontend && npm install && npm run dev"

echo Tudo certo! Feche as janelas do terminal quando quiser parar o sistema.