
@echo off

set common_compiler_flags=/nologo /TC /Zi /FC 
set common_linker_flags=user32.lib gdi32.lib winmm.lib opengl32.lib

REM -E - copies preprocessor output to standart output.
REM -EP - copies preprocessor output to standart output.
REM -P - writes preprocessor output to a file.

REM -F - stack size.(default is 1MB).
REM -FC - displays the full path of source code files.
REM -HELP - list compiler options.

REM -nologo - suppresses display of sign-on banner.
REM -O1 - creates small code.
REM -O2 - creates fast code.

REM -Od - disable optimization.

REM -TC - specifies all source files are C.
REM -TP - specifies all source files are C++.

REM -w - disable all warningns.
REM -W0 - set warning level 0.
REM -W1 - set warning level 1.
REM -W2 - set warning level 2.
REM -W3 - set warning level 3.
REM -W4 - set warning level 4.
REM -Wall - enable all warnings.
REM -wd<number> - disable the specific warning.

REM -Zi - generates complete debugging information.

IF NOT EXIST ..\..\build (mkdir ..\..\build)

pushd ..\..\build

cl ^
%common_compiler_flags% ^
..\Ced\code\w32_main.c /link %common_linker_flags% %*

popd

echo Compilation finished!