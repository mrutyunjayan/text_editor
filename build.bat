@ECHO OFF

SET project=%cd%
SET code=%project%\code

IF NOT EXIST build MKDIR build

IF NOT DEFINED DevEnvDir (
	CALL "D:\VS\2019\VC\Auxiliary\Build\vcvars64.bat" >NUL
)

SET debug_flags=^
	-Zi^
	-Od^
	-MTd^
	-DEBUG

SET common_compiler_flags=^
	%debug_flags%^
	-nologo^
	-FC^
	-fp:fast^
	-GR-^
	-EHa-^
	-Oi^
	-MTd^
	-TC^
    -ferror-limit=50^
	-Wall^
	-WX^
    -Wno-unused-parameter^
    -Wno-undef^
    -Wno-unused-macros^
    -Wno-extra-semi-stmt^
    -Wno-unused-function^
    -Wno-microsoft-include

SET defines=^
	-DJED_SLOW=1^
    -DJAI_INTERNAL=1

SET libs=^
	User32.lib^
	Gdi32.lib^
	Winmm.lib^
	Kernel32.lib

SET linker_flags=^
	-link^
	-incremental:no^
	-opt:ref^
	-SUBSYSTEM:CONSOLE^
	%libs%


SET include_directories=^
	-I %project%\include

PUSHD build

REM Precomipiled Header
clang-cl^
	%debug_flags%^
	%defines%^
	%include_directories%^
	%common_compiler_flags%^
	-c^
	-Yc"pch.h"^
	%code%\pch.c

REM Platform Layer
clang-cl^
	%defines%^
	%include_directories%^
	%common_compiler_flags%^
	-Yu"pch.h"^
	%code%\win32_main.c^
	-Fejed^
	%linker_flags%^
	pch.obj

POPD

IF %ERRORLEVEL% NEQ 0  (
	ECHO 	   ===============
	ECHO Error:	%ERRORLEVEL%
	ECHO 	   ================
)