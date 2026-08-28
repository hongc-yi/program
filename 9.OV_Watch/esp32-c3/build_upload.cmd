@echo off
rem ============================================================
rem  OV-Watch ESP32-C3 build & upload helper
rem  Usage: double-click, or run in cmd:
rem      "D:\learn\program\9.OV_Watch\esp32-c3\build_upload.cmd"
rem
rem  Why: Arduino IDE cannot link on this PC because the user
rem  directory name contains non-ASCII chars (Chinese), which
rem  breaks the ESP32 ld.exe. So we compile with arduino-cli
rem  using an ASCII data dir (D:\arduino15) and build dir.
rem ============================================================
setlocal
set "CLI=R:\arduino-ide\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe"
set "CFG=D:\arduino15\cli.yaml"
set "FQBN=esp32:esp32:esp32c3:CDCOnBoot=cdc"
set "BUILD=D:\ovwatch_build"
set "SKETCH=%~dp0ovwatch_link"

echo [1/2] Compiling %FQBN% ...
"%CLI%" --config-file "%CFG%" compile --fqbn %FQBN% "%SKETCH%" --build-path "%BUILD%"
if errorlevel 1 (
  echo.
  echo COMPILE FAILED - fix errors above and retry.
  exit /b 1
)

echo.
echo [2/2] Uploading ...
rem The ESP32-C3 USB serial device is COM6 on this PC.
rem Change this value if Windows assigns another COM number.
set "PORT=COM6"
echo Using port: %PORT%
"%CLI%" --config-file "%CFG%" upload -p %PORT% --fqbn %FQBN% --input-dir "%BUILD%"
if errorlevel 1 (
  echo.
  echo UPLOAD FAILED - check USB cable / BOOT button.
  exit /b 1
)
echo.
echo DONE. Firmware is running.
endlocal
