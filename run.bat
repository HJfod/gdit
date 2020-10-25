@echo off
for /f "tokens=1,* delims= " %%a in ("%*") do set ARGS=%%b
if "%1"=="r" (goto run)
echo Compiling...
clang main.cpp ext/ZlibHelper.cpp -o gdit.exe -L"." -lshell32 -lole32 -lzlib -m32 -std=c++17 -O3 -Wunused-value
if %errorlevel% == 0 (echo Successfully Compiled!) else (goto error)
if "%1"=="c" (goto done) else (goto run)

:run
echo Running...
echo.
if "%1"=="a" (gdit.exe %ARGS%) else (gdit.exe)
goto done

:error
echo Compilation failed

:done