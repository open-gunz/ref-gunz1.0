@echo off
if "%1" == "" goto ERROR
set batfile=bakpath.bat
rem echo %temp%
call makebakfolder.exe %temp% %batfile% %1 time
call %batfile%
if exist %batfile% del %batfile%
if exist %temp%%1 rmdir %temp%%1

GOTO END

:ERROR
echo ȯ�溯�� �̸��� ���̼���.
pause
exit

:END

