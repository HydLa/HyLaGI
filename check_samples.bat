@echo OFF
setlocal enabledelayedexpansion
for /F %%f in ('dir /b samples\*.sample') do (call :judge samples\%%f)
if EXIST check_samples.tmp (echo check_samples.tmp already exists
exit /b)
endlocal
exit /b


:judge

if EXIST check_samples.tmp (exit /b)

for /F "tokens=*" %%s in (%1) DO (
echo %%s> check_samples.tmp
Hyrose.exe %%s >> check_samples.tmp
goto END_LOOP1
)
:END_LOOP1

fc %1 check_samples.tmp

del check_samples.tmp

exit /b