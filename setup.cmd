@echo off
set v=vs2019
if not "%1"=="" (
	if "%1"=="-version" (
		set v=%2
	)
)
premake5 %v%