@echo off

set CommonCompilerFlags=-diagnostics:column -WL -nologo -fp:fast -fp:except- -Gm- -GR- -EHa- -Zo -Oi -WX -W4 -FC -Z7 -GS- -Gs9999999
set InternalCompilerFlags=-DKENGINE_INTERNAL=1 -DKENGINE_SLOW=1
set IncludeDirectories=-I..\lib\kengine\code

REM unreferenced formal parameter
set CommonCompilerFlags=%CommonCompilerFlags% -wd4100

REM unreferenced local variable
set CommonCompilerFlags=%CommonCompilerFlags% -wd4101

REM named type definition in parentheses
set CommonCompilerFlags=%CommonCompilerFlags% -wd4115

REM unnamed type definition in parentheses
set CommonCompilerFlags=%CommonCompilerFlags% -wd4116

REM conditional expression is constant
set CommonCompilerFlags=%CommonCompilerFlags% -wd4127

REM local variable is initialized but not referenced
set CommonCompilerFlags=%CommonCompilerFlags% -wd4189

REM nonstandard extension used: nameless struct/union
set CommonCompilerFlags=%CommonCompilerFlags% -wd4201

REM possible loss of data
set CommonCompilerFlags=%CommonCompilerFlags% -wd4244

REM 'type cast': pointer truncation
set CommonCompilerFlags=%CommonCompilerFlags% -wd4311

REM 'type cast': conversion
set CommonCompilerFlags=%CommonCompilerFlags% -wd4312

set CommonLinkerFlags=-STACK:0x100000,0x100000 -incremental:no -opt:ref

IF NOT EXIST bin mkdir bin
pushd bin

if "%~1"=="build_release" (

	cl %CommonCompilerFlags% -O2 -DKENGINE_CONSOLE=1 -I..\src ..\lib\kengine\code\win32_kengine.c /Fe:http_spammer.exe /link /NODEFAULTLIB /SUBSYSTEM:console %CommonLinkerFlags% uuid.lib

) else (
	if "%~1"=="build_dependencies" (

		REM Preprocessor
		cl %CommonCompilerFlags% %InternalCompilerFlags% -MTd -Od ..\lib\kengine\code\win32_kengine_preprocessor.c /link /NODEFAULTLIB /SUBSYSTEM:console %CommonLinkerFlags%
		pushd ..\lib\kengine\code
		..\..\..\bin\win32_kengine_preprocessor.exe kengine_types.h > kengine_generated.h
		..\..\..\bin\win32_kengine_preprocessor.exe kengine_debug.h > kengine_debug_generated.h
		..\..\..\bin\win32_kengine_preprocessor.exe win32_kengine_types.h > win32_kengine_generated.c
		popd

		REM Unit tests
		cl %CommonCompilerFlags% %InternalCompilerFlags% -MTd -Od ..\lib\kengine\code\win32_kengine_tests.c /link /NODEFAULTLIB /SUBSYSTEM:console %CommonLinkerFlags%
		win32_kengine_tests.exe

		REM Win32 platform
		cl %CommonCompilerFlags% %InternalCompilerFlags% -MTd -Od -DKENGINE_CONSOLE=1 ..\lib\kengine\code\win32_kengine.c /Fe:http_spammer.exe /link /NODEFAULTLIB /SUBSYSTEM:console %CommonLinkerFlags% uuid.lib

	) else (

		del kengine_*.pdb > NUL 2> NUL

		REM Application
		echo WAITING FOR PDB > lock.tmp
		cl %CommonCompilerFlags% %InternalCompilerFlags% -MTd -Od -DKENGINE_CONSOLE=1 -I..\src ..\lib\kengine\code\kengine.c -LD /link %CommonLinkerFlags% -PDB:kengine_%random%.pdb -EXPORT:AppTick_
		del lock.tmp
	
	)
)

popd