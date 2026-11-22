@echo off
setlocal
cd /d "%~dp0"

where git >nul 2>nul || (echo Git ontbreekt. Installeer: https://git-scm.com/download/win & pause & exit /b 1)
where cmake >nul 2>nul || (echo CMake ontbreekt. Installeer: https://cmake.org/download/ ^(kies "Add to PATH"^) & pause & exit /b 1)

if not exist JUCE (
    echo JUCE downloaden...
    git clone --depth 1 https://github.com/juce-framework/JUCE.git || goto :fail
)

echo Configureren...
cmake -B build -G "Visual Studio 17 2022" -A x64 || goto :fail

echo Bouwen ^(dit duurt een paar minuten^)...
cmake --build build --config Release --target FrostSynth_VST3 || goto :fail

set "SRC=build\FrostSynth_artefacts\Release\VST3\FrostSynth.vst3"
set "DST=%CommonProgramFiles%\VST3"

echo Installeren in %DST% ...
xcopy /E /I /Y "%SRC%" "%DST%\FrostSynth.vst3" >nul
if errorlevel 1 (
    echo.
    echo Kopieren mislukt: rechtsklik op build_windows.bat en kies "Als administrator uitvoeren".
    echo Of kopieer zelf de map "%SRC%" naar "%DST%".
    pause
    exit /b 1
)

echo.
echo Klaar! Open FL Studio, ga naar Options ^> Manage plugins en klik op "Find installed plugins".
pause
exit /b 0

:fail
echo.
echo Er ging iets mis. Kopieer de foutmelding hierboven en stuur hem naar Claude.
pause
exit /b 1
