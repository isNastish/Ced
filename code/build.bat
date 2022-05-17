
@echo off

rem -E - copies preprocessor output to standart output.
rem -EP - copies preprocessor output to standart output.
rem -P - writes preprocessor output to a file.

rem -F - stack size.(default is 1MB).
rem -FC - displays the full path of source code files.
rem -HELP - list compiler options.

rem -nologo - suppresses display of sign-on banner.
rem -O1 - creates small code.
rem -O2 - creates fast code.

rem -Od - disable optimization.

rem -TC - specifies all source files are C.
rem -TP - specifies all source files are C++.

rem -w - disable all warningns.
rem -W0 - set warning level 0.
rem -W1 - set warning level 1.
rem -W2 - set warning level 2.
rem -W3 - set warning level 3.
rem -W4 - set warning level 4.
rem -Wall - enable all warnings.
rem -wd<number> - disable the specific warning.

rem -Zi - generates complete debugging information.
rem -Fe:pathname - specifies a name and a directory for the .exe file or DLL created by the compiler.

set common_compiler_flags=/nologo /TC /Zi /FC /Fe: ced.exe
set common_linker_flags=user32.lib kernel32.lib gdi32.lib opengl32.lib

if not exist ..\..\build (mkdir ..\..\build)
pushd ..\..\build

cl ^
%common_compiler_flags% ^
 ..\ced\code\w32_ced_main.c /link %common_linker_flags% %*

popd

echo Compilation finished!