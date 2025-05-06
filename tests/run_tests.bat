@echo off

REM Create output directory
mkdir output 2>nul

REM Run each test case
for /L %%i in (1,1,5) do (
    echo Running test%%i.json...
    ..\json2relcsv < test%%i.json --out-dir output\test%%i
    if errorlevel 1 (
        echo Test %%i failed
    ) else (
        echo Test %%i completed successfully
        echo Output files:
        dir /b output\test%%i
    )
    echo.
) 