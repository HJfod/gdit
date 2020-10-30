@echo off
for /f "tokens=1,* delims= " %%a in ("%*") do set ARGS=%%b
if "%1"=="r" (goto run)
echo Compiling...
windres gdit.rc -O coff -o resources/gdit.res --target=pe-i386
set g=0
if "%1"=="gui" set g=1
if "%1"=="guic" set g=1
if "%1"=="guip" set g=1
if %g%==1 (clang main.cpp ext/ZlibHelper.cpp -o gdit-gui.exe resources/gdit.res -L"." -DGUI -lSDL2 -lSDL2main -luser32 -ldwmapi -lSDL2_ttf -lshell32 -lole32 -lzlib -m32 -mwindows -std=c++17 -O3) else (clang main.cpp ext/ZlibHelper.cpp -o gdit.exe resources/gdit.res -L"." -lshell32 -lole32 -lzlib -m32 -std=c++17 -O3)
if %errorlevel% == 0 (echo Successfully Compiled!) else (goto error)
if "%1"=="p" (goto publish)
if "%1"=="guip" (goto publishwgui)
if "%1"=="c" (goto done)
if "%1"=="guic" (goto done) else (goto run)

:run
echo Running...
echo.
if "%1"=="a" (gdit.exe %ARGS%)
if "%1"=="gui" (gdit-gui.exe) else (gdit.exe)
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

:publishwgui
md releases
xcopy /y gdit-gui.exe releases\gdit.exe*
xcopy /y zlib.dll releases\zlib.dll*
xcopy /y SDL2.dll releases\SDL2.dll*
xcopy /y SDL2_ttf.dll releases\SDL2_ttf.dll*
xcopy /y zlib1.dll releases\zlib1.dll*
xcopy /y libfreetype-6.dll releases\libfreetype-6.dll*
xcopy /y README-SDL.txt releases\README-SDL.txt*
echo Published in releases/gdit!
goto done

:done