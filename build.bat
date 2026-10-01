@echo off
setlocal EnableExtensions

:: =============================================================================
::  SPACE INVADERS EXTENDED - Skrypt budowania / Build script
::  Rozbudowana wersja klasycznej gry Space Invaders
::  Extended version of the classic Space Invaders game
:: =============================================================================
::
::  Wersja / Version: 1.1
::  Autor / Author:   Maciej Sikorski
::  Data / Date:      01.10.2026
::
::  Licencja / License: Apache License, Version 2.0
::                      http://www.apache.org/licenses/LICENSE-2.0
::  SPDX-License-Identifier: Apache-2.0
::  Copyright 2026 Maciej Sikorski
::
::  Zmiany v1.1 / Changes v1.1:
::    - Automatyczne wykrywanie MSVC (vswhere) lub MinGW-w64 (g++).
::    - Wyswietlanie nazwy i wersji kompilatora.
::    - Opcje: debug, clean, msvc, mingw, help; poprawne kody wyjscia.
::    - Wersja dwujezyczna PL / EN.
::
::    - Automatic detection of MSVC (vswhere) or MinGW-w64 (g++).
::    - Compiler name and version are printed.
::    - Options: debug, clean, msvc, mingw, help; proper exit codes.
::    - Bilingual PL / EN version.
::
::  Zmiany v1.0 / Changes v1.0:
::    - Pierwsza wersja (tylko MSVC) / Initial version (MSVC only).
::
::  Licensed under the Apache License, Version 2.0 (the "License");
::  you may not use this file except in compliance with the License.
::  You may obtain a copy of the License at
::
::      http://www.apache.org/licenses/LICENSE-2.0
::
::  Unless required by applicable law or agreed to in writing, software
::  distributed under the License is distributed on an "AS IS" BASIS,
::  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
::  See the License for the specific language governing permissions and
::  limitations under the License.
::
:: -----------------------------------------------------------------------------
::  Platforma / Platform:   Windows (Win32 API + GDI), x64
::
::  Kompilatory / Compilers (wykrywane automatycznie / auto-detected):
::    1. MSVC  - Visual Studio 2019 / 2022 / 2026 (wymaga komponentu
::               "Desktop development with C++" / requires the "Desktop
::               development with C++" workload). Szukane przez vswhere.exe,
::               a gdy go brak - w standardowych katalogach instalacji.
::               Located via vswhere.exe, else in the standard install folders.
::    2. MinGW-w64 GCC 13+ (g++ w PATH / g++ on PATH) - np. MSYS2 UCRT64
::
::  Uzycie / Usage:
::    build.bat              Release, kompilator wykryty automatycznie
::                           Release, compiler auto-detected
::    build.bat debug        Wersja debug (bez optymalizacji, z symbolami)
::                           Debug build (no optimization, with symbols)
::    build.bat msvc         Wymus MSVC / Force MSVC
::    build.bat mingw        Wymus MinGW / Force MinGW
::    build.bat clean        Usun pliki wynikowe / Remove build output
::    build.bat help         Ta pomoc / This help
::    (opcje mozna laczyc / options can be combined: build.bat debug mingw)
::
::  Wynik / Output:          Space.exe w katalogu biezacym / in current directory
::  Kod wyjscia / Exit code: 0 = sukces / success, 1 = blad / failure
::
::  Uwaga / Note: ten plik jest celowo ASCII (bez polskich liter), aby dzialal
::  w kazdej stronie kodowej konsoli. / This file is intentionally ASCII-only
::  so it works in any console code page.
:: =============================================================================

:: --- Konfiguracja / Configuration -------------------------------------------
set "APP_NAME=SPACE INVADERS EXTENDED"
set "APP_VER=1.1.0"
set "SRC=space.cpp"
set "OUT=Space.exe"
set "OBJDIR=build"

set "MODE=release"
set "WANT=auto"
set "RC=0"

:: Uruchomienie dwuklikiem (okno zamknie sie po zakonczeniu) -> pause na koncu
:: Started by double-click (window would close at the end) -> pause at the end
set "DBLCLICK="
for %%x in (%cmdcmdline%) do if /i "%%~x"=="/c" set "DBLCLICK=1"
if defined CI set "DBLCLICK="

:: --- Argumenty / Arguments --------------------------------------------------
:parse
if "%~1"=="" goto parsed
if /i "%~1"=="release" set "MODE=release"
if /i "%~1"=="debug"   set "MODE=debug"
if /i "%~1"=="msvc"    set "WANT=msvc"
if /i "%~1"=="mingw"   set "WANT=mingw"
if /i "%~1"=="clean"   goto do_clean
if /i "%~1"=="help"    goto usage
if /i "%~1"=="-h"      goto usage
if /i "%~1"=="--help"  goto usage
if /i "%~1"=="/?"      goto usage
shift
goto parse
:parsed

:: --- Naglowek / Banner ------------------------------------------------------
echo.
echo =============================================================================
echo   %APP_NAME%  -  Build Script
echo =============================================================================
echo   Wersja / Version:   %APP_VER%
echo   Autor / Author:     Maciej Sikorski
echo   Licencja / License: Apache License, Version 2.0
echo                       http://www.apache.org/licenses/LICENSE-2.0
echo   Copyright (c) 2026 Maciej Sikorski
echo   Tryb / Mode:        %MODE%
echo =============================================================================
echo.

if not exist "%SRC%" (
    echo === BLAD: nie znaleziono %SRC% w katalogu biezacym ===
    echo === ERROR: %SRC% not found in the current directory ===
    set "RC=1"
    goto done
)

:: --- Wybor kompilatora / Compiler selection ---------------------------------
if /i "%WANT%"=="mingw" goto use_mingw

:: MSVC juz w PATH (np. Developer Command Prompt)?
:: MSVC already on PATH (e.g. Developer Command Prompt)?
where cl >nul 2>&1
if not errorlevel 1 goto use_msvc

call :find_vcvars
if defined VCVARS goto load_vcvars

if /i "%WANT%"=="msvc" goto no_msvc
goto use_mingw

:: --- MSVC: zaladowanie srodowiska / load environment ------------------------
:load_vcvars
echo [MSVC] Srodowisko / Environment: %VCVARS%
call "%VCVARS%" >nul
if errorlevel 1 (
    echo.
    echo === BLAD: nie udalo sie zaladowac vcvars64.bat ===
    echo === ERROR: failed to load vcvars64.bat ===
    set "RC=1"
    goto done
)

:: --- MSVC: informacje o kompilatorze / compiler info ------------------------
:use_msvc
set "CL_LINE="
for /f "delims=" %%v in ('cl 2^>^&1 ^| findstr /R /C:"[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*"') do if not defined CL_LINE set "CL_LINE=%%v"
echo -----------------------------------------------------------------------------
echo   Kompilator / Compiler:  MSVC (cl.exe)
echo   cl.exe:                 %CL_LINE%
echo   Toolset (VCToolsVersion): %VCToolsVersion%
echo   Visual Studio:          %VisualStudioVersion%
echo   Architektura / Target:  %VSCMD_ARG_TGT_ARCH%
echo   Windows SDK:            %WindowsSDKVersion%
echo   Standard C++:           C++17 (/std:c++17), UTF-8 (/utf-8)
echo -----------------------------------------------------------------------------
echo.

if not exist "%OBJDIR%" mkdir "%OBJDIR%"

set "CFLAGS=/nologo /EHsc /std:c++17 /utf-8 /W3 /D_CRT_SECURE_NO_WARNINGS"
set "LFLAGS=/SUBSYSTEM:WINDOWS"
if /i "%MODE%"=="debug" (
    set "CFLAGS=%CFLAGS% /MTd /Od /Zi /DDEBUG"
    set "LFLAGS=%LFLAGS% /DEBUG /PDB:%OBJDIR%\Space.pdb"
) else (
    set "CFLAGS=%CFLAGS% /MT /O2 /DNDEBUG"
)

echo [MSVC] Kompilacja i linkowanie / Compile and link...
cl %CFLAGS% "%SRC%" /Fo%OBJDIR%\ /Fd%OBJDIR%\ /link %LFLAGS% user32.lib gdi32.lib msimg32.lib winmm.lib /OUT:%OUT%
if errorlevel 1 set "RC=1"
goto result

:: --- MinGW-w64 --------------------------------------------------------------
:use_mingw
where g++ >nul 2>&1
if errorlevel 1 goto no_compiler

set "GCC_LINE="
for /f "delims=" %%v in ('g++ --version ^| findstr /R /C:"[0-9]"') do if not defined GCC_LINE set "GCC_LINE=%%v"
echo -----------------------------------------------------------------------------
echo   Kompilator / Compiler:  MinGW-w64 GCC (g++)
echo   g++:                    %GCC_LINE%
echo   Standard C++:           C++17 (-std=c++17), UTF-8
echo -----------------------------------------------------------------------------
echo.

set "GFLAGS=-std=c++17 -Wall -Wextra -static -mwindows"
if /i "%MODE%"=="debug" (
    set "GFLAGS=%GFLAGS% -O0 -g -DDEBUG"
) else (
    set "GFLAGS=%GFLAGS% -O2 -DNDEBUG"
)

echo [MinGW] Kompilacja i linkowanie / Compile and link...
g++ %GFLAGS% "%SRC%" -o "%OUT%" -lgdi32 -luser32 -lmsimg32 -lwinmm
if errorlevel 1 set "RC=1"
goto result

:: --- Wynik / Result ---------------------------------------------------------
:result
echo.
if "%RC%"=="0" (
    echo === OK: %OUT% gotowe / %OUT% ready ===
) else (
    echo === BLAD KOMPILACJI ===
    echo === COMPILATION ERROR ===
)
goto done

:: --- Bledy srodowiska / Environment errors ----------------------------------
:no_msvc
echo === BLAD: nie znaleziono MSVC ===
echo === ERROR: MSVC not found ===
echo.
echo   Zainstaluj Visual Studio (Community) lub Build Tools z komponentem
echo   "Desktop development with C++": https://visualstudio.microsoft.com/downloads/
echo   Install Visual Studio (Community) or Build Tools with the
echo   "Desktop development with C++" workload.
set "RC=1"
goto done

:no_compiler
echo === BLAD: nie znaleziono zadnego kompilatora (MSVC ani g++) ===
echo === ERROR: no compiler found (neither MSVC nor g++) ===
echo.
echo   Opcja 1 / Option 1: Visual Studio / Build Tools - "Desktop development with C++"
echo   Opcja 2 / Option 2: MSYS2 + MinGW-w64 (pacman -S mingw-w64-ucrt-x86_64-gcc)
set "RC=1"
goto done

:: --- clean ------------------------------------------------------------------
:do_clean
echo Czyszczenie / Cleaning...
if exist "%OBJDIR%" rmdir /s /q "%OBJDIR%"
if exist "%OUT%" del /q "%OUT%"
if exist "*.obj" del /q "*.obj"
echo === OK: wyczyszczono / cleaned ===
echo (Pliki zapisu w %%LOCALAPPDATA%%\SpaceInvadersExtended nie sa usuwane /
echo  save files in %%LOCALAPPDATA%%\SpaceInvadersExtended are not touched)
goto done

:: --- help -------------------------------------------------------------------
:usage
echo.
echo  %APP_NAME% - build.bat  v%APP_VER%
echo.
echo  Uzycie / Usage:  build.bat [release^|debug] [msvc^|mingw] [clean] [help]
echo.
echo    (bez argumentow / no arguments)  Release, kompilator wykryty automatycznie
echo                                     Release, compiler auto-detected
echo    debug     Bez optymalizacji, z symbolami / No optimization, with symbols
echo    msvc      Wymus MSVC / Force MSVC
echo    mingw     Wymus MinGW-w64 (g++) / Force MinGW-w64 (g++)
echo    clean     Usun pliki wynikowe / Remove build output
echo.
goto done

:: --- Koniec / End -----------------------------------------------------------
:done
if defined DBLCLICK pause
endlocal & exit /b %RC%

:: =============================================================================
::  Podprogram: szukanie vcvars64.bat / Subroutine: locate vcvars64.bat
::  Ustawia VCVARS (puste, gdy nie znaleziono) / Sets VCVARS (empty if not found)
:: =============================================================================
:find_vcvars
set "VCVARS="
set "VSROOT="
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" goto find_vcvars_fallback
for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSROOT=%%i"
if not defined VSROOT goto find_vcvars_fallback
if exist "%VSROOT%\VC\Auxiliary\Build\vcvars64.bat" set "VCVARS=%VSROOT%\VC\Auxiliary\Build\vcvars64.bat"
if defined VCVARS exit /b 0

:find_vcvars_fallback
:: Standardowe katalogi / Standard folders: wersje 18 (2026), 2022, 2019
for %%v in (18 2022 2019) do for %%e in (Community Professional Enterprise BuildTools) do if not defined VCVARS if exist "%ProgramFiles%\Microsoft Visual Studio\%%v\%%e\VC\Auxiliary\Build\vcvars64.bat" set "VCVARS=%ProgramFiles%\Microsoft Visual Studio\%%v\%%e\VC\Auxiliary\Build\vcvars64.bat"
exit /b 0
