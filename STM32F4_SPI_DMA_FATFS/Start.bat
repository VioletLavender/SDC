for /f "delims=" %%A in ('dir /b .\MDK\*.uvprojx') do set "Project=%%A"
start .\MDK\%Project%