@echo off
:: Use short paths if possible or ensure rigorous quoting
set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"

if not exist %VCVARS% (
    echo Error: vcvarsall.bat not found.
    exit /b 1
)

echo Setting up environment...
call %VCVARS% x64

echo.
echo Compiling main.cpp...
cl /EHsc /W4 main.cpp /Fe:hwcheck.exe User32.lib gdi32.lib dxgi.lib Advapi32.lib

if %ERRORLEVEL% equ 0 (
    echo.
    echo Build successful! Run hwcheck.exe to see hardware info.
) else (
    echo.
    echo Build failed.
)
