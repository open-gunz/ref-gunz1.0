@echo off
rem publish.bat [���] [�������]

set TARGETLANG=KOR
if "%1" == "" goto :no_target_lang
set TARGETLANG=%1
:no_target_lang

del vssver.scc /s /q /f

mkdir \\Databank\TeamWorks\Gunz\BuildServer\lastest
set TARGET=\\Databank\TeamWorks\Gunz\BuildServer\lastest\MatchAgent\


del %TARGET%*.* /s /q /f
mkdir %TARGET%

copy MatchAgent.exe %TARGET%
copy MatchAgent.pdb %TARGET%
copy dbghelp.dll %TARGET%
copy AgentConfig.xml %TARGET%_AgentConfig.xml

rem �ͽ��÷η� â�� ����.
explorer \\databank\TeamWorks\Gunz\BuildServer\lastest\
