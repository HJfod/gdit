@echo off
for /f "tokens=1,* delims= " %%a in ("%*") do set ARGS=%%b
if "%1"=="r" (goto run)
echo Compiling...
windres gdit.rc -O coff -o resources/gdit.res --target=pe-i386
clang main.cpp ext/ZlibHelper.cpp -o gdit.exe resources/gdit.res -L"." -lshell32 -lole32 -lzlib -m32 -std=c++17 -O3
if %errorlevel% == 0 (echo Successfully Compiled!) else (goto error)
if "%1"=="p" (goto publish)
if "%1"=="c" (goto done) else (goto run)

:run
echo Running...
echo.
if "%1"=="a" (gdit.exe %ARGS%) else (gdit.exe)
goto done

:error
echo Compilation failed
goto done

:publish
md releases
xcopy /y gdit.exe releases\gdit.exe*
xcopy /y zlib.dll releases\zlib.dll*
echo Published in releases/gdit!
goto done

:done